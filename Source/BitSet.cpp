#include "BitSet.h"
#include <cassert>
#include <cmath>
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
    assert(_numWords == other._numWords);
    memcpy(_words, other._words, _numWords*sizeof(Word));
}

void BitSet::Set(int b)
{
    assert(b >= 0 && b < _numBits);
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
    assert(b >= 0 && b < _numBits);
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

int BitSet::Count(int w) const
{
    return Count(_words[w]);
}

int BitSet::CountAnd(const BitSet& other) const
{
    assert(_numBits == other._numBits);
    int s = 0;
    for (int i = 0; i < _numWords; i++)
        s += Count(_words[i] & other._words[i]);
    return s;
}

int BitSet::CountAndSparse(const BitSet& other) const
{
    assert(_numBits == other._numBits);
    int s = 0;
    for (int i = 0; i < _numWords; i++)
        s += CountSparse(_words[i] & other._words[i]);
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

    _words[_numWords-1] = ~_words[_numWords-1] & mask;
}

int BitSet::BitInWord(int wi, int n) const
{
    int b = -1;
    Word w = _words[wi];
    int i = 0;
    while (i++ <= n)
    {
        b = __builtin_ffsll(w)-1;
        w &= ~(One << b);
    }

    return b;
}

BitSet& BitSet::operator|=(const BitSet& other)
{
    assert(_numBits == other._numBits);
    for (int i = 0; i < _numWords; i++)
        _words[i] |= other._words[i];
    return *this;
}

BitSet& BitSet::operator&=(const BitSet& other)
{
    assert(_numBits == other._numBits);
    for (int i = 0; i < _numWords; i++)
        _words[i] &= other._words[i];
    return *this;
}

std::string BitSet::ToString() const
{
    std::string s = "";
    for (int i = 0; i < _numWords; i++)
        s += WordString(_words[i]);

    // Truncate s to remove the unused bits.
    s = s.substr(0, _numBits);

    // Insert newlines and reverse.
    std::string s2 = "";
    int n = sqrt(_numBits);
    for (int i = n; i >= 0; i--)
    {
        s2 += s.substr(n*i, n);
        s2 += "\n";
    }

    return s2;
}

int BitSet::Count(Word w) const
{
    w = w - ((w >> 1) & 0x5555555555555555);
    w = (w & 0x3333333333333333) + ((w >> 2) & 0x3333333333333333);
    return (((w + (w >> 4)) & 0xF0F0F0F0F0F0F0F) * 0x101010101010101) >> 56;
}

// Kernighan
int BitSet::CountSparse(Word w) const
{
    int c;
    for (c = 0; w; c++)
        w &= w-1;
    return c;
}

std::string BitSet::WordString(Word w) const
{
    std::string s = "";
    for (int i = 0; i < WordSize; i++)
        s += (w & One << i) ? "1" : "0";
    return s;
}

int BitIterator::Next()
{
    if (_i == BitIterator::NoBit)
        return _i;

    // Find the next set bit.
    bool found = false;
    while (!found && _wi < _bs.NumWords())
    {
        _i = __builtin_ffsll(_cw)-1;
        if (_i < 0)
        {
            // Move to next word.
            _cw = ++_wi < _bs.NumWords() ? _bs.GetWord(_wi) : 0;
        }
        else
        {
            // Found a bit, turn it off in the current word.
            _cw &= ~(One << _i);
            found = true;
        }
    }

    if (!found)
        _i = BitIterator::NoBit;

    return found ? _i + _wi*_bs.GetWordSize() : _i;
}

BitSelector::~BitSelector()
{
    if (_counts != nullptr)
    {
        delete[] _counts;
        _counts = nullptr;
    }
}

void BitSelector::InitialiseCounts()
{
    _counts = new int[_bs.NumWords()];
    for (int i = 0; i < _bs.NumWords(); i++)
        _counts[i] = _bs.Count(i);
}

int BitSelector::operator[](int n)
{
    int bit = 0;
    bool found = false;
    for (int w = 0; !found && w < _bs.NumWords(); w++)
    {
        if (n >= _counts[w])
        {
            n -= _counts[w];
            bit += _bs.GetWordSize();
        }
        else
        {
            found = true;
            bit += _bs.BitInWord(w, n);
        }
    }

    return bit;
}
