#include "../ngram.h"
#include <iostream>
#include <fstream>
#include <sstream>


const size_t Symbols = 29; // 26+1+2
const size_t SymbolBits = 6; // OK up to 32 symbols



class Charencoder
{
public:
	Charencoder(const char* infile)
	: is(infile), WSlast(true)
	{ fillLUT(codeLUT, revCodeLUT); };

	class EndOfInput {};

	unsigned char get()
	{
		unsigned char tkn = is.get();
		while(is.good())
		{
			const unsigned char enc = codeLUT[tkn];
			if((enc != 0 || !WSlast) && enc < Symbols)
			{
				WSlast = enc == 0;
				return enc;
			}
			tkn = is.get();
		}
		throw EndOfInput();
	};

private:
	std::ifstream is;
	bool WSlast; // last character whitespace (remove consecutive whitespaces)
	unsigned char codeLUT[256];
	char revCodeLUT[Symbols];

};






template <unsigned int N>
void generateNgram(const char* infile, std::ostream& os)
{
	uint64_t samplesParsed = 0;

	std::cout << "Generating " << N << "-grams..." << std::flush;
	Ngram<N, Symbols, SymbolBits> ngram;
	Charencoder inp(infile);
	unsigned char data[N+1];

	try
	{
		// initial fill
		for (unsigned int ii = 0; ii < N+1; ++ii)
			data[ii] = inp.get();

		// read
		for (;;)
		{
			ngram.addSample(data);
			++samplesParsed;
			for (unsigned int ii = 1; ii < N+1; ++ii)
				data[ii-1] = data[ii];
			data[N] = inp.get();
		}
	}
	catch (Charencoder::EndOfInput& e)
	{
		// end of input, continue..
	}
	std::cout << " " << samplesParsed << " samples parsed." << std::endl;


	std::cout << "Writing to file..." << std::flush;
	uint64_t entries = ngram.write(os);
	os << std::flush;

	uint64_t maxEnt = 1;
	for (unsigned int ii = 0; ii < N; ++ii)
		maxEnt *= Symbols;

	std::cout << " " << entries << " of " << maxEnt << " possible N-grams entries written." << std::endl;
}




void helptext(const char* progname, unsigned int Nmaxmax)
{
	std::cerr << "Usage: " << progname << " <input text file> <output N-gram file> <N-max>" << std::endl;
	std::cerr << "N-max 1-" << Nmaxmax << "\n" << std::endl;
}




int main(int argc, char**argv)
{
	const unsigned int Nmaxmax = 10;
	unsigned int Nmax;

	// input check
	if (argc < 4)
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

	{
		std::ifstream is(argv[1], std::ios::binary);
		if (!is)
		{
			std::cerr << "Could not open input file: " << argv[1] << std::endl;
			return 1;
		}
	}

	std::ofstream os(argv[2], std::ios::binary);
	if (!os)
	{
		std::cerr << "Could not open output file: " << argv[2] << std::endl;
		return 1;
	}

	generateNgram<1>(argv[1], os);
	if (Nmax == 1) return 0;

	generateNgram<2>(argv[1], os);
	if (Nmax == 2) return 0;

	generateNgram<3>(argv[1], os);
	if (Nmax == 3) return 0;

	generateNgram<4>(argv[1], os);
	if (Nmax == 4) return 0;

	generateNgram<5>(argv[1], os);
	if (Nmax == 5) return 0;

	generateNgram<6>(argv[1], os);
	if (Nmax == 6) return 0;

	generateNgram<7>(argv[1], os);
	if (Nmax == 7) return 0;

	generateNgram<8>(argv[1], os);
	if (Nmax == 8) return 0;

	generateNgram<9>(argv[1], os);
	if (Nmax == 9) return 0;

	generateNgram<10>(argv[1], os);
	if (Nmax == 10) return 0;


	return 0;
}








/*
int mainOld()
{
	const size_t maxN = 10;
	unsigned char codeLUT[256];
	char revCodeLUT[Symbols];
	fillLUT(codeLUT, revCodeLUT);

	const size_t datalen = 1000000;

	//const unsigned char* str = reinterpret_cast<const unsigned char*>("Hejsan hoppsan fallerallera nar julen kommer finns grot.");
	unsigned char data[datalen];
	size_t wrIdx = 0;
	//size_t rdIdx = 0;
	bool WSlast = true;
	//std::ifstream inp("../darwin.txt");
	std::istream& inp = std::cin;
	while(inp.good() && wrIdx < datalen)
	{
		const unsigned char tkn = inp.get();
		const unsigned char enc = codeLUT[tkn];
		if((enc != 0 || !WSlast) && enc < Symbols)
		{
			data[wrIdx++] = enc;
			WSlast = enc == 0;
		}
	}

	std::cout << wrIdx << std::endl;


	//for (size_t ii = 0; ii < wrIdx; ++ii)
	//	std::cout << " " << revCodeLUT[data[ii]] << (int) data[ii];
	//std::cout << std::endl;

	Ngram<maxN,Symbols,6> ngram;
	for (size_t ii = 0; ii+maxN < wrIdx; ++ii)
		ngram.addSample(data+ii);


	std::cout << "Writing to file..." << std::flush;
	{
		std::ofstream os("ngramfile", std::ios::binary);
		ngram.write(os);
	}
	std::cout << "done." << std::endl;

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

