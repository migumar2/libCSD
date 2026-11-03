/* Regression test: the decoding table of a Hu-Tucker coded dictionary lacked
 * the chunk that ends a header or the last string, which the decoder took for
 * a long codeword: a wild pointer. RPHTFC guessed the bits following a header,
 * and indexed the last one never; HHTFC never indexed the end of the last
 * string; and where the last bucket holds only a header, the decoder of HTFC,
 * HHTFC and RPHTFC read past the end of the stream.
 *
 *   make tests/last_chunk_decoding && tests/last_chunk_decoding
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

static const char *PATH = "tests/last_chunk_decoding.dict";

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
static void check(StringDictionary *built, const vector<string> &keys, uint bucket, const char *name)
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
			printf("%s, %zu keys, bucket %u: key %zu has the wrong answer\n", name, keys.size(), bucket, i+1);
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
			printf("%s, %zu keys, bucket %u: extractTable differs at key %zu\n", name, keys.size(), bucket, i+1);
			exit(1);
		}
	}
	delete it;
	delete dict;
}

/* The decimal numbers from 1 to n, in byte order. */
static vector<string> decimals(uint n)
{
	vector<string> keys;
	for (uint i=1; i<=n; i++)
	{
		char key[16];
		snprintf(key, sizeof(key), "%u", i);
		keys.push_back(key);
	}
	sort(keys.begin(), keys.end());
	return keys;
}

/* n strings of 25 pseudo-random bases: a small grammar, so few bits a symbol. */
static vector<string> bases(uint n)
{
	vector<string> keys;
	uint state = 1;
	for (uint i=0; i<n; i++)
	{
		string key;
		for (uint j=0; j<25; j++)
		{
			state = state*1103515245+12345;
			key += "ACGT"[(state >> 16) & 3];
		}
		keys.push_back(key);
	}
	sort(keys.begin(), keys.end());
	keys.erase(unique(keys.begin(), keys.end()), keys.end());
	return keys;
}

int main()
{
	// No multiples of a bucket: the last bucket is short, or holds only a header
	const uint sizes[] = {129, 1001, 1003, 1025};
	const uint buckets[] = {2, 8, 64};

	for (size_t s=0; s<sizeof(sizes)/sizeof(sizes[0]); s++)
	{
		vector<string> keys = decimals(sizes[s]);

		for (size_t b=0; b<sizeof(buckets)/sizeof(buckets[0]); b++)
		{
			if (keys.size()%buckets[b] == 0) continue;

			check(new StringDictionaryHTFC(plain(keys), buckets[b]), keys, buckets[b], "HTFC");
			check(new StringDictionaryHHTFC(plain(keys), buckets[b]), keys, buckets[b], "HHTFC");

			// A smaller set overruns the Re-Pair input, which is another defect
			if (keys.size() >= 8*buckets[b])
				check(new StringDictionaryRPHTFC(plain(keys), buckets[b]), keys, buckets[b], "RPHTFC");
		}
	}

	// The end of a short key set, in a short last bucket
	{
		vector<string> keys = decimals(3);
		check(new StringDictionaryHHTFC(plain(keys), 8), keys, 8, "HHTFC");
	}

	// The Re-Pair symbols are shorter than what is left of a header's last chunk
	{
		vector<string> keys = bases(2003);
		check(new StringDictionaryRPHTFC(plain(keys), 16), keys, 16, "RPHTFC");
	}

	remove(PATH);
	printf("ok\n");
	return 0;
}
