/* Regression test: with a single key no bucket holds an internal string, the end
 * of the first bucket minus 1 wraps around, and the RPFC constructor walks off
 * the empty vector of symbols.
 *
 *   make tests/rp_single_key && tests/rp_single_key
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

static const char *PATH = "tests/rp_single_key.dict";

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

int main()
{
	const uint buckets[] = {2, 8, 64};
	vector<string> keys;
	keys.push_back("alpha");

	for (size_t b=0; b<sizeof(buckets)/sizeof(buckets[0]); b++)
		check(new StringDictionaryRPFC(plain(keys), buckets[b]), keys, "RPFC");

	remove(PATH);
	printf("ok\n");
	return 0;
}
