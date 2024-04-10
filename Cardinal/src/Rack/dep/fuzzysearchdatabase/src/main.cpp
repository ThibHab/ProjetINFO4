
#ifdef _MSC_VER // compiles with /Wall in VS
#pragma warning(push)
#pragma warning(disable : 4365)
#pragma warning(disable : 4514)
#pragma warning(disable : 4571)
#pragma warning(disable : 4625)
#pragma warning(disable : 4626)
#pragma warning(disable : 4710)
#pragma warning(disable : 4774)
#pragma warning(disable : 4820)
#pragma warning(disable : 5026)
#pragma warning(disable : 5027)
#endif

#include <chrono>
#include <iostream>

#ifdef _MSC_VER
#pragma warning(pop) 
#endif

#include "FuzzySearchDatabase.hpp"

#include "testData.h"


void fillDb(fuzzysearch::Database<>& f)
{
	auto t1 = std::chrono::steady_clock::now();

	size_t cnt = sizeof(entries) / sizeof(Entry);

	std::vector<std::string> fields(5);

	for (size_t i = 0; i < cnt; ++i)
	{
		const Entry& e = entries[i];

		fields[0] = e.brand;
		fields[1] = e.modslug;
		fields[2] = e.modname;
		fields[3] = e.moddesc;
		fields[4] = e.tags;

		std::string key(fields[0]);
		key += ":";
		key += fields[1];
		key += ":";
		key += fields[2];
		key += ":";
		key += fields[3];
		key += ":";
		key += fields[4];

		f.addEntry( key, fields );
	}

	auto t2 = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

	std::cout << "\nFillDB Time: " << fp_ms.count() << " ms\n";

}

void q(const fuzzysearch::Database<>& f, std::string qs)
{
	std::cout << " --- \n";

	auto t1 = std::chrono::steady_clock::now();
	auto res = f.search( qs );
	auto t2 = std::chrono::steady_clock::now();
	std::chrono::duration<double, std::milli> fp_ms = t2 - t1;

	std::cout << "\nQuery: \"" << qs << "\" Time: " << fp_ms.count() << "ms #Results: " << res.size() << "\n";

	for (size_t i = 0; i < res.size(); ++i)
	{
		auto& a = res[i];
		std::cout << "#" << i << ": score: " << a.score << " -> Key: \"" << a.key << "\"\n";

		// skip after N
		if (i > 22)
			break;
	}
}

void word2word(std::string queryWord, std::string word)
{
	fuzzysearch::HelperFunctions::WordHist2 qHist2(queryWord);

	float score = fuzzysearch::HelperFunctions::scoreQueryWordToWord(qHist2, queryWord.c_str(), queryWord.size(), word.c_str(), word.size());
	printf("%s <-> %s = %f\n", queryWord.c_str(), word.c_str(), score);
}

int main(int /*argc*/, char** /*argv*/)
{
	fuzzysearch::Database<> f;
	fillDb(f);

	f.setWeights({
		1.0f, // brand
		0.9f, // modslug
		1.0f, // modname
		0.9f, // moddesc
		1.0f // tags
		});
	
	f.setThreshold(0.5f);

	// test individual words
//	word2word("cvtomidi","cv-midi");

#if 0
	// test for searches that give too many result
	q(f, "vcv");
#endif

#if 1
	// test mis-spelling
	q(f, "rvrb"); // ~2
	q(f, "revrb"); // ~1

	// should return results?
	q(f, "elctrc"); // ~2
#endif

#if 1
	// match-order or query-word
	q(f, "cv to midi");
	q(f, "cvtomidi");
#endif

	// classics
#if 1
	q(f, "vca");
	q(f, "vcf");
	q(f, "vco");
	q(f, "vcv vca");
	q(f, "vcv vco");
	q(f, "vcv vcf");
#endif

#if 1
	// connected-words
	q(f, "multi band");
	q(f, "multiband");
	q(f, "hostxl");
	q(f, "host xl");
	q(f, "midimap");
	q(f, "midi map");
#endif

#if 1
	// kick by itself score higher than rest
	q(f, "kick");

	// suggestions from:
	// https://community.vcvrack.com/t/please-help-me-improve-the-module-browser-search-function-feedback-request/18687
	q(f, "FMOP");
	q(f, "S&H");
	q(f, "Switch");
#endif


	puts( "done" );
}