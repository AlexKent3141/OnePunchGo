#ifndef __BITSET_H__
#define __BITSET_H__

#include <cassert>
#include <cstdint>
#include <string>

typedef uint64_t Word;

const Word One = 1;

// Bit set class which is used for quick board-wise operations.
class BitSet
{
public:
    BitSet() = delete;
    BitSet(int);
    BitSet(const BitSet&);
    ~BitSet();

    inline int NumWords() const { return _numWords; }

    inline int NumBits() const { return _numBits; }

    inline Word GetWord(int i) const
    {
        assert(i < _numWords);
        return _words[i];
    }

    inline int GetWordSize() const { return WordSize; }

    void Copy(const BitSet&);

    // Set the specified bit.
    void Set(int);

    void Set(const BitSet&);

    // Unset the specified bit.
    void UnSet(int);

    // Unset the specified bits.
    void UnSet(const BitSet&);

    // Check the state of the specified bit.
    bool Test(int) const;

    // Count the number of set bits.
    int Count() const;

    // Count the number of set bits in the specified word.
    int Count(int) const;

    // Count the number of set bits in the bitwise AND of this and another BitSet.
    int CountAnd(const BitSet&) const;

    int CountAndSparse(const BitSet&) const;

    // Invert this BitSet in place.
    void Invert();

    // Get the index of the n-th set bit in the word.
    int BitInWord(int, int) const;

    BitSet& operator|=(const BitSet&);

    BitSet& operator&=(const BitSet&);

    // Get a string representation of this BitSet.
    std::string ToString() const;

private:
    const int WordSize = 64;

    // This is the internal representation of the bits.
    Word* _words = nullptr;

    int _numWords;
    int _numBits;

    // Get a string representation for the word.
    std::string WordString(Word) const;
    
    int Count(Word) const;

    int CountSparse(Word) const;
};

// This object allows iteration over the positions of the set bits in a BitSet.
class BitIterator
{
public:
    static const int NoBit = -2;

    BitIterator(const BitSet& bs) : _bs(bs), _wi(0), _i(-1)
    {
        _cw = _bs.GetWord(0);
    }

    int Next();

private:
    const BitSet& _bs;
    int _wi, _i;
    Word _cw;
};

// This object allows bits to be selected by index efficiently.
class BitSelector
{
public:
    BitSelector(const BitSet& bs) : _bs(bs)
    {
        InitialiseCounts();
    }

    ~BitSelector();

    int operator[](int);

private:
    const BitSet& _bs;
    int* _counts = nullptr;

    void InitialiseCounts();
};

#endif // __BITSET_H__
