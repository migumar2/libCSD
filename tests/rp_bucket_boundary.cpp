/* Regression test: the RPFC and RPHTFC constructors wrote one size_t past
 * their vector of bucket beginnings when the number of keys is a multiple of
 * the bucket size. AddressSanitizer reports the write; without it, glibc's
 * allocator aborts later or not at all, depending on what the write hits.
 *
 *   make tests/rp_bucket_boundary && tests/rp_bucket_boundary
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

static const char *PATH = "tests/rp_bucket_boundary.dict";

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

int main()
{
	vector<string> keys;
	for (uint i=0; i<1024; i++)
	{
		char key[16];
		snprintf(key, sizeof(key), "key%04u", i);
		keys.push_back(key);
	}

	const uint buckets[] = {2, 4, 8, 16, 32};

	for (size_t b=0; b<sizeof(buckets)/sizeof(buckets[0]); b++)
	{
		delete new StringDictionaryRPHTFC(plain(keys), buckets[b]);

		// The library only queries a loaded dictionary
		{
			StringDictionary *built = new StringDictionaryRPFC(plain(keys), buckets[b]);
			ofstream out(PATH, ios::binary);
			built->save(out);
			delete built;
		}

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
				printf("RPFC, bucket %u: key %zu has the wrong answer\n", buckets[b], i+1);
				return 1;
			}
		}

		delete dict;
	}

	remove(PATH);
	printf("ok\n");
	return 0;
}
