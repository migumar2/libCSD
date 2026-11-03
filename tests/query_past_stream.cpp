/* Regression test: the search for a bucket compares a compressed header with the
 * code of the query over the whole length of that code, which reads past the end
 * of the stream where the query is longer than what follows the header. Run this
 * under AddressSanitizer: a plain build answers correctly with or without the fix.
 *
 * The key counts stay clear of other defects: multiples of the bucket in HTFC and
 * HHTFC, none in RPHTFC, which is also asked only for the keys of its first buckets.
 *
 *   make tests/query_past_stream && tests/query_past_stream
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

static const char *PATH = "tests/query_past_stream.dict";

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


static vector<string> keysOf(uint longKeys, uint shortKeys)
{
	vector<string> keys;

	for (uint i=0; i<longKeys; i++)
	{
		string key(3000, 'a');
		key += (char)('A'+i);
		key += string(i%2, 'z');
		keys.push_back(key);
	}

	for (uint i=0; i<shortKeys; i++)
	{
		string key("b");
		key += (char)('A'+i);
		keys.push_back(key);
	}

	return keys;
}

static void check(StringDictionary *built, const vector<string> &keys, size_t asked, uint longKeys, const char *name)
{
	{
		ofstream out(PATH, ios::binary);
		built->save(out);
	}
	delete built;

	ifstream in(PATH, ios::binary);
	StringDictionary *dict = StringDictionary::load(in, 0);
	in.close();

	for (size_t i=0; i<asked; i++)
	{
		if (dict->locate((uchar*)keys[i].c_str(), keys[i].size()) != i+1)
		{
			printf("%s: key %zu has the wrong answer\n", name, i+1);
			exit(1);
		}
	}

	string absent(6000, 'a');
	if (dict->locate((uchar*)absent.c_str(), absent.size()) != NORESULT)
	{
		printf("%s: an absent key was found\n", name);
		exit(1);
	}

	string prefix(3000, 'a');
	IteratorDictIDContiguous *ids = (IteratorDictIDContiguous*)dict->locatePrefix((uchar*)prefix.c_str(), prefix.size());
	if ((ids->getLeftLimit() != 1) || (ids->getRightLimit() != longKeys))
	{
		printf("%s: the prefix has the wrong answer\n", name);
		exit(1);
	}
	delete ids;

	delete dict;
}

int main()
{
	const uint bucketsize = 8;

	vector<string> keys = keysOf(16, 16);
	check(new StringDictionaryHTFC(plain(keys), bucketsize), keys, keys.size(), 16, "HTFC");
	check(new StringDictionaryHHTFC(plain(keys), bucketsize), keys, keys.size(), 16, "HHTFC");

	keys = keysOf(16, 14);
	check(new StringDictionaryRPHTFC(plain(keys), bucketsize), keys, 16, 16, "RPHTFC");

	remove(PATH);
	printf("ok\n");
	return 0;
}
