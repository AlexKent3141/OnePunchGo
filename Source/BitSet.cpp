#include "BitSet.h"
#include <cassert>
#include <cstring>

BitSet::BitSet(int size)
{
    _numBits = size;
    _numWords = size / WordSize;
    if (size % WordSize) ++_numWords; // Take care of rounding down.
    _words = new Word[_numWords];
    memset(_words, 0, _numWords*sizeof(Word));
}

BitSet::BitSet(const BitSet& other)
{
    _numBits = other._numBits;
    _numWords = other._numWords;
    _words = new Word[_numWords];
    memcpy(_words, other._words, _numWords*sizeof(Word));
}

BitSet::~BitSet()
{
    if (_words != nullptr)
    {
        delete[] _words;
        _words = nullptr;
    }
}

void BitSet::Copy(const BitSet& other)
{
    assert(_numBits == other._numBits);
    memcpy(_words, other._words, _numWords*sizeof(Word));
}

void BitSet::Set(int b)
{
    assert(b < _numBits);
    _words[b / WordSize] |= One << (b % WordSize);
}

void BitSet::Set(const BitSet& other)
{
    assert(_numBits == other._numBits);
    for (int i = 0; i < _numWords; i++)
        _words[i] |= other._words[i];
}

void BitSet::UnSet(int b)
{
    assert(b < _numBits);
    _words[b / WordSize] &= ~(One << (b % WordSize));
}

void BitSet::UnSet(const BitSet& other)
{
    assert(_numBits == other._numBits);
    for (int i = 0; i < _numWords; i++)
        _words[i] &= ~other._words[i];
}

bool BitSet::Test(int b) const
{
    assert(b < _numBits);
    return _words[b / WordSize] & (One << b % WordSize);
}

int BitSet::Count() const
{
    int s = 0;
    for (int i = 0; i < _numWords; i++)
        s += Count(_words[i]);
    return s;
}

int BitSet::CountAnd(const BitSet& other) const
{
    assert(_numBits == other._numBits);
    int s = 0;
    for (int i = 0; i < _numWords; i++)
        s += Count(_words[i] & other._words[i]);
    return s;
}

void BitSet::Invert()
{
    for (int i = 0; i < _numWords-1; i++)
        _words[i] = ~_words[i];

    // Deal with the last word more carefully.
    // If bits above _numBits get set it will break the "Count" methods.
    int n = _numBits % WordSize;
    if (n == 0) n = WordSize; // Edge case where _numBits is divisible by WordSize.
    Word mask = 0;
    for (int i = 0; i < n; i++)
        mask |= One << i;

    _words[_numBits-1] = ~_words[_numBits-1] & mask;
}

BitSet& BitSet::operator|=(const BitSet& other)
{
    assert(_numBits == other._numBits);
    for (int i = 0; i < _numWords; i++)
        _words[i] |= other._words[i];
    return *this;
}

int BitSet::Count(Word w) const
{
    w = w - ((w >> 1) & 0x5555555555555555);
    w = (w & 0x3333333333333333) + ((w >> 2) & 0x3333333333333333);
    return (((w + (w >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
}
