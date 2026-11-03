/* Regression test: the constructors warn that a bucket size below 2 builds with
 * buckets of size 2, set the member, and go on with the parameter of the same
 * name: a division by zero at bucket size 1 in RPFC and RPHTFC, a wrong
 * dictionary or a hang elsewhere.
 *
 * The key counts stay clear of other defects: odd in RPFC and RPHTFC, even in
 * the rest. RPHTFC is only compared with its build at bucket size 2 for the
 * same reason.
 *
 *   make tests/bucket_size_below_two && tests/bucket_size_below_two
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
using namespace std;

#include "StringDictionary.h"
#include "iterators/IteratorDictStringPlain.h"

static const char *PATH = "tests/bucket_size_below_two.dict";

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

static vector<string> keysOf(uint n)
{
	vector<string> keys;
	char key[16];

	for (uint i=0; i<n; i++)
	{
		sprintf(key, "key-%04u", i);
		keys.push_back(key);
	}

	return keys;
}

template <class Dict>
static string fileOf(const vector<string> &keys, uint bucketsize)
{
	Dict *dict = new Dict(plain(keys), bucketsize);
	{
		ofstream out(PATH, ios::binary);
		dict->save(out);
	}
	delete dict;

	ifstream in(PATH, ios::binary);
	return string(istreambuf_iterator<char>(in), istreambuf_iterator<char>());
}

template <class Dict>
static void cases(const vector<string> &keys, const char *name, bool query)
{
	string two = fileOf<Dict>(keys, 2);

	const uint below[] = {1, 0};

	for (size_t b=0; b<2; b++)
	{
		uint bucketsize = below[b];

		if (fileOf<Dict>(keys, bucketsize) != two)
		{
			printf("%s at bucket size %u differs from bucket size 2\n", name, bucketsize);
			exit(1);
		}

		if (query) check(new Dict(plain(keys), bucketsize), keys, name);
	}
}

int main()
{
	cases<StringDictionaryPFC>(keysOf(100), "PFC", true);
	cases<StringDictionaryHTFC>(keysOf(100), "HTFC", true);
	cases<StringDictionaryHHTFC>(keysOf(100), "HHTFC", true);
	cases<StringDictionaryRPFC>(keysOf(101), "RPFC", true);
	cases<StringDictionaryRPHTFC>(keysOf(101), "RPHTFC", false);

	remove(PATH);
	printf("ok\n");
	return 0;
}
