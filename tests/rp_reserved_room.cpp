/* Regression test: the RPFC and RPHTFC constructors reserved room for a bucket
 * from the key count and from bucketsize*maxlength or bucketsize*1000, and wrote
 * past the Re-Pair input or the compressed text where the bucket took more.
 *
 * The key counts are no multiple of the bucket and leave more than a header in
 * the last bucket, to stay clear of other defects. RPHTFC is only built here,
 * for the same reason.
 *
 *   make tests/rp_reserved_room && tests/rp_reserved_room
 */

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "StringDictionary.h"
#include "iterators/IteratorDictStringPlain.h"

static const char *PATH = "tests/rp_reserved_room.dict";

static IteratorDictString *plain(const vector<string> &keys)
{
	size_t bytes = 0;
	for (size_t i=0; i<keys.size(); i++) bytes += keys[i].size()+1;

	uchar *text = new uchar[bytes];
	size_t at = 0;
	for (size_t i=0; i<keys.size(); i++)
	{
		memcpy(text+at, keys[i].c_str(), keys[i].size()+1);
		at += keys[i].size()+1;
	}

	return new IteratorDictStringPlain(text, bytes);
}

/* Saves and loads the dictionary, because the library only queries a loaded
   one, and asks it for every key by string, by ID and through extractTable. */
static void check(StringDictionary *built, const vector<string> &keys, const char *name)
{
	{
		ofstream out(PATH, ios::binary);
		built->save(out);
	}
	delete built;

	ifstream in(PATH, ios::binary);
	StringDictionary *dict = StringDictionary::load(in, 0);
	in.close();

	for (size_t i=0; i<keys.size(); i++)
	{
		uint len = 0;
		uchar *back = dict->extract(i+1, &len);
		bool same = keys[i] == (char*)back;
		delete [] back;

		uint id = dict->locate((uchar*)keys[i].c_str(), keys[i].size());

		if (!same || (id != i+1))
		{
			printf("%s, %zu keys: key %zu has the wrong answer\n", name, keys.size(), i+1);
			exit(1);
		}
	}

	IteratorDictString *it = dict->extractTable();
	for (size_t i=0; i<keys.size(); i++)
	{
		uint len = 0;
		uchar *back = it->next(&len);
		bool same = keys[i] == (char*)back;
		delete [] back;

		if (!same)
		{
			printf("%s, %zu keys: extractTable differs at key %zu\n", name, keys.size(), i+1);
			exit(1);
		}
	}
	delete it;
	delete dict;
}

static vector<string> randomKeys(uint n, uint len)
{
	uint64_t state = 88172645463325252ULL;
	vector<string> keys;

	for (uint i=0; i<n; i++)
	{
		string key(len, 'a');
		for (uint c=0; c<len; c++)
		{
			state ^= state << 13; state ^= state >> 7; state ^= state << 17;
			key[c] = (char)('a'+(state%26));
		}
		keys.push_back(key);
	}

	sort(keys.begin(), keys.end());
	return keys;
}

static void cases(const vector<string> &keys, uint bucketsize)
{
	check(new StringDictionaryRPFC(plain(keys), bucketsize), keys, "RPFC");
	delete new StringDictionaryRPHTFC(plain(keys), bucketsize);
}

int main()
{
	// Few keys: a single doubling of the Re-Pair input is not enough
	vector<string> two;
	two.push_back("alpha");
	two.push_back("bravo-charlie-delta-echo");
	cases(two, 8);

	// Short keys in long buckets: bucketsize*maxlength is below what they take
	cases(randomKeys(63, 6), 32);

	// Long keys: a bucket takes more than a doubling of the Re-Pair input
	cases(randomKeys(63, 3000), 8);

	// Long keys: a bucket takes more than bucketsize*1000 bytes of compressed text
	cases(randomKeys(255, 3000), 8);

	remove(PATH);
	printf("ok\n");
	return 0;
}
