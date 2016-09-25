/*
 * N+1-gram counting and synthesizing class.
 *
 * Kristoffer Öfjäll 2014
 */

#ifndef NGRAM_H
#define NGRAM_H

#include <iostream>
#include <bitset>
#include <unordered_map>
#include <array>
#include <cstdint>

/***
 * N: number of symbols in Ngram
 * SymCount: cardinality of symbol set
 * SymBits:  bits needed to enumerate symbol set
 * Ctype:    type of counter (unsigned integer type) or relative freq. (floating point type)
 */
template <size_t N, size_t SymCount, size_t SymBits, typename Ctype = uint64_t>
class Ngram
{
public:
	Ngram();

	inline void   addSample(const unsigned char sample[N+1]);
	unsigned char getChar(const unsigned char ngram[N], double rand01) const; ///< return 255 if no matching ngram

	//TODO: regenerateTotalCount, for floating point tables

	uint64_t write(std::ostream& os) const;  // write serialized values to stream (return entry count)
	uint64_t read(std::istream& is);     // load serialized values from stream (adds to already existing counts), return loaded entry count

	void dumpRep(std::ostream& os, const char* revCodeLUT) const; ///< print table in readable format (using provided lookup table for characters)




private:
	typedef std::bitset<N*SymBits>         KeyType;
	typedef std::array<Ctype, SymCount+1>  ArrayType; // zeroth index total count
	typedef std::unordered_map<KeyType, ArrayType> MapType;

	MapType map;

	inline KeyType toBitset(const unsigned char data[N]) const;
	inline void toCstr(const KeyType bitsIn, unsigned char dataOut[N]) const;

	const KeyType symbolMask;
};




template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
Ngram<N, SymCount, SymBits, Ctype>::
Ngram()
	: symbolMask(((1<<SymBits)-1))
{

}



template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
void Ngram<N, SymCount, SymBits, Ctype>::
addSample(const unsigned char sample[N+1])
{
	//++(map[toBitset(sample)].at(idx+1));
	const size_t idx = sample[N];
	ArrayType& arr = map[toBitset(sample)];
	++arr[0];
	++arr[idx+1];
}



template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
unsigned char  Ngram<N, SymCount, SymBits, Ctype>::
getChar(const unsigned char ngram[N], double rand01) const
{
	auto it = map.find(toBitset(ngram));
	if (it == map.end())
		return 255;

	const ArrayType& arr = it->second;

	Ctype selval = rand01*arr[0];
	for (size_t ii = 1; ii <= SymCount; ++ii)
	{
		if (selval < arr[ii])
			return ii-1;
		else
			selval -= arr[ii];
	}

	// else return last nonzero
	for (size_t ii = SymCount; ii > 0; --ii)
		if (arr[ii] > 0) return ii-1;

	return 255; // should not happen
}




/**
 * serialize data
 * 3 uint16_t: N, SymCount, SymBits
 * 1 uint64_t: table entry count
 *
 * lots of:
 * N uint8_t:            prefix.
 * 1 uint64_t:			 total count
 * SymCount uint64_t:    counts for each following symbol
 */
template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
uint64_t Ngram<N, SymCount, SymBits, Ctype>::
write(std::ostream& os) const
{
	uint16_t header[3] = {N, SymCount, SymBits};
	uint64_t entryCount = map.size();
	os.write((char*)header, 3*2);
	os.write((char*)&entryCount, 8);

	uint8_t  prefix[N];
	uint64_t counts[SymCount+1];
	for (auto it = map.begin(); it != map.end(); ++it)
	{
		toCstr(it->first, prefix);
		for (size_t ii = 0; ii < SymCount+1; ++ii)
			counts[ii] = it->second[ii];

		os.write((char*)prefix, 1*N);
		os.write((char*)counts, 8*(SymCount+1));
	}

	return entryCount;
}


/**
 * see write() for format.
 */
template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
uint64_t Ngram<N, SymCount, SymBits, Ctype>::
read(std::istream& is)
{
	uint16_t header[3];
	uint64_t entryCount;
	is.read((char*)header, 3*2);
	is.read((char*)&entryCount, 8);
	if(N != header[0] || SymCount != header[1] || SymBits != header[2])
		return 0;

	uint8_t  prefix[N];
	uint64_t counts[SymCount+1];

	for (uint64_t ii = 0; ii < entryCount; ++ii)
	{
		is.read((char*)prefix, 1*N);
		is.read((char*)counts, 8*(SymCount+1));
		ArrayType& arr = map[toBitset(prefix)];
		for (size_t ii = 0; ii < SymCount+1; ++ii)
			arr[ii] += counts[ii];
	}

	return entryCount;
}


template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
void Ngram<N, SymCount, SymBits, Ctype>::
dumpRep(std::ostream& os, const char* revCodeLUT) const
{

	for (auto it = map.begin(); it != map.end(); ++it)
	{
		//os << it->first << " " << it->second << std::endl;
		unsigned char gram[N];
		toCstr(it->first, gram);
		for(size_t ii = 0; ii < N; ++ii)
			os << revCodeLUT[gram[ii]];
		os << ": ";
		for(size_t ii = 1; ii < SymCount+1; ++ii)
			os << it->second[ii] << " ";
		os << ": " << it->second[0] << "\n";
	}
}



template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
typename Ngram<N, SymCount, SymBits, Ctype>::KeyType Ngram<N, SymCount, SymBits, Ctype>::
toBitset(const unsigned char data[N]) const
{
	KeyType bits;
	bits |= data[0];
	for (size_t ii = 1; ii < N; ++ii)
	{
		bits <<= SymBits;
		bits |=  data[ii];
	}

	return bits;
}



template <size_t N, size_t SymCount, size_t SymBits, typename Ctype>
void Ngram<N, SymCount, SymBits, Ctype>::
toCstr(const KeyType bitsIn, unsigned char dataOut[N]) const
{
	KeyType bits = bitsIn;
	for (size_t ii = N-1; ii > 0; --ii)
	{
		//dataOut[ii] = 0;
		dataOut[ii] = (bits & symbolMask).to_ulong();
		bits >>= SymBits;
	}
	//dataOut[0] = 0;
	dataOut[0] = (bits & symbolMask).to_ulong();
}




// code table for english text.
void fillLUT(unsigned char* codeLUT, char* revCodeLUT)
{
	revCodeLUT[0] = ' ';
	for (size_t ii = 0; ii < 26; ++ii)
		revCodeLUT[ii+1] = ii+'a';
	revCodeLUT[27] = ',';
	revCodeLUT[28] = '.';
//	revCodeLUT[27] = 'å';
//	revCodeLUT[28] = 'ä';
//	revCodeLUT[29] = 'ö';

	for (size_t ii = 0; ii < 256; ++ii)
		codeLUT[ii] = 0; // anything else as whitespace

	//codeLUT['-'] = 255; // remove
	codeLUT['a'] = 1;
	codeLUT['b'] = 2;
	codeLUT['c'] = 3;
	codeLUT['d'] = 4;
	codeLUT['e'] = 5;
	codeLUT['f'] = 6;
	codeLUT['g'] = 7;
	codeLUT['h'] = 8;
	codeLUT['i'] = 9;
	codeLUT['j'] = 10;
	codeLUT['k'] = 11;
	codeLUT['l'] = 12;
	codeLUT['m'] = 13;
	codeLUT['n'] = 14;
	codeLUT['o'] = 15;
	codeLUT['p'] = 16;
	codeLUT['q'] = 17;
	codeLUT['r'] = 18;
	codeLUT['s'] = 19;
	codeLUT['t'] = 20;
	codeLUT['u'] = 21;
	codeLUT['v'] = 22;
	codeLUT['w'] = 23;
	codeLUT['x'] = 24;
	codeLUT['y'] = 25;
	codeLUT['z'] = 26;
//	codeLUT['å'] = 27;
//	codeLUT['ä'] = 28;
//	codeLUT['ö'] = 29;
	codeLUT['A'] = 1;
	codeLUT['B'] = 2;
	codeLUT['C'] = 3;
	codeLUT['D'] = 4;
	codeLUT['E'] = 5;
	codeLUT['F'] = 6;
	codeLUT['G'] = 7;
	codeLUT['H'] = 8;
	codeLUT['I'] = 9;
	codeLUT['J'] = 10;
	codeLUT['K'] = 11;
	codeLUT['L'] = 12;
	codeLUT['M'] = 13;
	codeLUT['N'] = 14;
	codeLUT['O'] = 15;
	codeLUT['P'] = 16;
	codeLUT['Q'] = 17;
	codeLUT['R'] = 18;
	codeLUT['S'] = 19;
	codeLUT['T'] = 20;
	codeLUT['U'] = 21;
	codeLUT['V'] = 22;
	codeLUT['X'] = 23;
	codeLUT['X'] = 24;
	codeLUT['Y'] = 25;
	codeLUT['Z'] = 26;
	codeLUT[','] = 27;
	codeLUT['.'] = 28;
//	codeLUT['Å'] = 27;
//	codeLUT['Ä'] = 28;
//	codeLUT['Ö'] = 29;
	}





#endif
