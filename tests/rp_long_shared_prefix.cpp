/* Regression test: from a shared prefix of 16,384 chars the VByte takes 3 bytes.
 * The RPFC and RPHTFC constructors took a 0 past its first byte for the end of a
 * string, and their decoders read 2 bytes of it.
 *
 * The key count is no multiple of the bucket, and the bucket is large enough, to
 * stay clear of other defects. RPHTFC is only asked by ID outside its last bucket
 * for the same reason.
 *
 *   make tests/rp_long_shared_prefix && tests/rp_long_shared_prefix
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include "StringDictionary.h"
#include "iterators/IteratorDictStringPlain.h"

static const char *PATH = "tests/rp_long_shared_prefix.dict";

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

static void save(StringDictionary *built)
{
	ofstream out(PATH, ios::binary);
	built->save(out);
	delete built;
}

static void checkRPHTFC(const vector<string> &keys, uint bucketsize)
{
	save(new StringDictionaryRPHTFC(plain(keys), bucketsize));

	ifstream in(PATH, ios::binary);
	StringDictionary *dict = StringDictionary::load(in, 0);
	in.close();

	size_t asked = ((keys.size()-1)/bucketsize)*bucketsize;
	IteratorDictString *it = dict->extractTable();

	for (size_t i=0; i<asked; i++)
	{
		uint len = 0;
		uchar *back = dict->extract(i+1, &len);
		bool same = keys[i] == (char*)back;
		delete [] back;

		back = it->next(&len);
		same = same && (keys[i] == (char*)back);
		delete [] back;

		if (!same)
		{
			printf("RPHTFC, %zu keys: key %zu has the wrong answer\n", keys.size(), i+1);
			exit(1);
		}
	}

	delete it;
	delete dict;
}

int main()
{
	// With the VBytes: 00 00 81, 01 00 81, 7F 00 81, 00 01 81, 20 1C 81, 00 00 82
	const size_t shared[] = {16384, 16385, 16511, 16512, 20000, 32768};
	const uint bucketsize = 64;

	for (size_t s=0; s<sizeof(shared)/sizeof(shared[0]); s++)
	{
		// Groups of keys sharing exactly that many chars with the previous
		// one in the group. One key short of a multiple of the bucket.
		vector<string> keys;
		for (uint group=0; group<64; group++)
			for (uint i=0; i<4; i++)
			{
				if ((group == 63) && (i == 3)) break;

				string key(shared[s]-1, 'a');
				key += (char)('A'+group);
				key += (char)('a'+i);
				key += string(i%2, 'z');
				keys.push_back(key);
			}

		check(new StringDictionaryRPFC(plain(keys), bucketsize), keys, "RPFC");
		checkRPHTFC(keys, bucketsize);
	}

	// Each shared length just once, with no 0 in its VByte, so Re-Pair does not
	// join its bytes in a rule the decoder would read at once
	vector<string> keys;
	for (uint i=0; i<255; i++)
		keys.push_back(string(31790-(58*i), 'a')+'b');

	check(new StringDictionaryRPFC(plain(keys), bucketsize), keys, "RPFC");
	checkRPHTFC(keys, bucketsize);

	remove(PATH);
	printf("ok\n");
	return 0;
}
