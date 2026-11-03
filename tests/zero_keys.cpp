/* Regression test: with no keys the positional index gets fields of 0 bits and no
 * word to hold them, and the search for a string or a prefix leaves its bucket
 * range empty and reads a bucket anyway.
 *
 *   make tests/zero_keys && tests/zero_keys
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

static const char *PATH = "tests/zero_keys.dict";


static void check(StringDictionary *built, const char *name)
{
	{
		ofstream out(PATH, ios::binary);
		built->save(out);
	}
	delete built;

	ifstream in(PATH, ios::binary);
	StringDictionary *dict = StringDictionary::load(in, 0);
	in.close();

	uchar key[] = "alpha";
	uint len = 1;

	bool same = (dict->numElements() == 0)
		&& (dict->locate(key, 5) == NORESULT)
		&& (dict->extract(1, &len) == NULL) && (len == 0);

	IteratorDictString *it = dict->extractTable();
	same = same && !it->hasNext();
	delete it;

	IteratorDictID *ids = dict->locatePrefix(key, 5);
	same = same && !ids->hasNext();
	delete ids;

	same = same && (dict->extractPrefix(key, 5) == NULL);

	if (!same)
	{
		printf("%s has the wrong answer with no keys\n", name);
		exit(1);
	}

	delete dict;
}

int main()
{
	const uint buckets[] = {2, 8};

	for (size_t b=0; b<sizeof(buckets)/sizeof(buckets[0]); b++)
	{
		check(new StringDictionaryPFC(new IteratorDictStringPlain(new uchar[0], 0), buckets[b]), "PFC");
		check(new StringDictionaryHTFC(new IteratorDictStringPlain(new uchar[0], 0), buckets[b]), "HTFC");
		check(new StringDictionaryHHTFC(new IteratorDictStringPlain(new uchar[0], 0), buckets[b]), "HHTFC");
		check(new StringDictionaryRPFC(new IteratorDictStringPlain(new uchar[0], 0), buckets[b]), "RPFC");
		check(new StringDictionaryRPHTFC(new IteratorDictStringPlain(new uchar[0], 0), buckets[b]), "RPHTFC");
	}

	remove(PATH);
	printf("ok\n");
	return 0;
}
