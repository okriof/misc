#include "../ngram.h"
#include "speak.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <random>


const size_t Symbols = 29; // 26+1+2
const size_t SymbolBits = 6; // OK up to 32 symbols



void helptext(const char* progname, unsigned int Nmaxmax)
{
	std::cerr << "Usage: " << progname << " <input N-gram file> <output generated file>|speak <N-max> <output size>" << std::endl;
	std::cerr << "N-max 1-" << Nmaxmax << "\n" << std::endl;
}


template <unsigned int N>
void loadNgrams(Ngram<N, Symbols, SymbolBits>& ngram, std::istream& is, const unsigned int Nmax)
{
	if (N <= Nmax)
	{
		std::cout << "Loading " << N << "-grams..." << std::flush;
		uint64_t entries = ngram.read(is);
		std::cout << " " << entries << " entries loaded." << std::endl;
	}
}



int main(int argc, char**argv)
{
	eSpeak speaker;
	const unsigned int Nmaxmax = 10;
	unsigned int Nmax;
	uint64_t     outputSize;

	// input check
	if (argc < 5)
	{
		helptext(argv[0], Nmaxmax);
		return 1;
	}

	std::istringstream iss(argv[3]);
	iss >> Nmax;
	if (Nmax < 1 || Nmax > Nmaxmax)
	{
		helptext(argv[0], Nmaxmax);
		return 1;
	}

	std::istringstream issofs(argv[4]);
	issofs >> outputSize;

	std::ifstream is(argv[1], std::ios::binary);
	if (!is)
	{
		std::cerr << "Could not open input file: " << argv[1] << std::endl;
		return 1;
	}

	const bool doSpeak = strcmp(argv[2], "speak") == 0;
	std::ofstream os;
	if (!doSpeak)
	{
		os.open(argv[2], std::ios::binary);
		if (!os)
		{
			std::cerr << "Could not open output file: " << argv[2] << std::endl;
			return 1;
		}
	}
	else
		std::cout << "Speaking:" << std::endl;

	unsigned char codeLUT[256];
	char revCodeLUT[Symbols];
	fillLUT(codeLUT, revCodeLUT);

	Ngram<1, Symbols, SymbolBits> ngram1;
	Ngram<2, Symbols, SymbolBits> ngram2;
	Ngram<3, Symbols, SymbolBits> ngram3;
	Ngram<4, Symbols, SymbolBits> ngram4;
	Ngram<5, Symbols, SymbolBits> ngram5;
	Ngram<6, Symbols, SymbolBits> ngram6;
	Ngram<7, Symbols, SymbolBits> ngram7;
	Ngram<8, Symbols, SymbolBits> ngram8;
	Ngram<9, Symbols, SymbolBits> ngram9;
	Ngram<10, Symbols, SymbolBits> ngram10;

	loadNgrams<1>(ngram1, is, Nmax);
	loadNgrams<2>(ngram2, is, Nmax);
	loadNgrams<3>(ngram3, is, Nmax);
	loadNgrams<4>(ngram4, is, Nmax);
	loadNgrams<5>(ngram5, is, Nmax);
	loadNgrams<6>(ngram6, is, Nmax);
	loadNgrams<7>(ngram7, is, Nmax);
	loadNgrams<8>(ngram8, is, Nmax);
	loadNgrams<9>(ngram9, is, Nmax);
	loadNgrams<10>(ngram10, is, Nmax);

	std::cout << "Generating " << outputSize << " character text:" << std::endl;

	// random numbers:
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<double> rnd01(0.0,1.0);

	const unsigned int speakBufferSize = 10000;  // should be long enough?
	unsigned char* speakBuffer = 0;
	unsigned int speakIdx = 0;
	if (doSpeak)
	{
		speakBuffer = new unsigned char[speakBufferSize];
		speaker.speak("Hello!");
	}

	unsigned char data[Nmax];
	unsigned char* dataEnd = data+Nmax;
	data[Nmax-1] = 0;		// start from a space (not written)
	unsigned int usedN = 1; // generated from history length

	for (uint64_t chout = 0; doSpeak || chout < outputSize; ++chout)
	{
		unsigned char gen = 255;
		while (gen == 255)
		{
			double randnum = rnd01(generator);
			switch (usedN)
			{
			case 0:
				if (data[Nmax-1] != 0)
				{
					std::cout << "Error while generating, restarting from space" << std::endl;
					gen = 0;
				}
				else
				{
					std::cout << "No character to start with!" << std::endl;
					return 1;
				}
				break;
			case 1: gen = ngram1.getChar(dataEnd-usedN, randnum); break;
			case 2: gen = ngram2.getChar(dataEnd-usedN, randnum); break;
			case 3: gen = ngram3.getChar(dataEnd-usedN, randnum); break;
			case 4: gen = ngram4.getChar(dataEnd-usedN, randnum); break;
			case 5: gen = ngram5.getChar(dataEnd-usedN, randnum); break;
			case 6: gen = ngram6.getChar(dataEnd-usedN, randnum); break;
			case 7: gen = ngram7.getChar(dataEnd-usedN, randnum); break;
			case 8: gen = ngram8.getChar(dataEnd-usedN, randnum); break;
			case 9: gen = ngram9.getChar(dataEnd-usedN, randnum); break;
			case 10: gen = ngram10.getChar(dataEnd-usedN, randnum); break;
			default: std::cout << "Unexpected N" << std::endl; return 1; break;
			}
			if (gen == 255) --usedN;
		}
		// ok character, can expect longer sequence next round
		++usedN;
		if (usedN > Nmax) usedN = Nmax;
		// shift
		for (unsigned int ii = 1; ii < Nmax; ++ii)
			data[ii-1] = data[ii];
		data[Nmax-1] = gen;
		if (doSpeak)
		{
			speakBuffer[speakIdx] = revCodeLUT[gen];
			std::cout << speakBuffer[speakIdx];
			if (speakBuffer[speakIdx] == '.' || speakIdx == speakBufferSize-2)
			{
				std::cout << std::endl;
				speakBuffer[speakIdx +1] = 0;
				speaker.speak((char*)speakBuffer);
				speakIdx = 0;
			}
			else
				++speakIdx;
		}
		else
		{
			os.write(revCodeLUT+gen, 1);
		}
	}


	return 0;
}


/*
int mainOld()
{

	Ngram<maxN,Symbols,6> ngramRead;
	std::cout << "Reading from file..." << std::flush;
	{
		std::ifstream is("ngramfile", std::ios::binary);

		uint64_t entries = ngramRead.read(is);
		std::cout << entries << "entries loaded." << std::endl;
	}


	// generate
	std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
	std::uniform_real_distribution<double> rnd01(0.0,1.0);

	//for (unsigned int ii = 0; ii < 100; ++ii)
	//	std::cout << rnd01(generator) << std::endl;

	for(unsigned int ii = 0; ii < maxN; ++ii)
		std::cout << revCodeLUT[data[ii]];

	for(unsigned int ii = 0; ii < 40000; ++ii)
	{
		unsigned char tkn = ngramRead.getChar(data+ii, rnd01(generator));
		if (tkn >= Symbols) tkn = 0;
		data[ii+maxN] = tkn;
		std::cout << revCodeLUT[tkn];
	}

	std::cout << std::endl;

	return 0;
}
*/
