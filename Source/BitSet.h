#ifndef __BITSET_H__
#define __BITSET_H__

#include <cassert>
#include <cstdint>
#include <string>

// Bit set class which is used for quick board-wise operations.
class BitSet
{
private:
    typedef uint64_t Word;

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

    // Count the number of set bits in the bitwise AND of this and another BitSet.
    int CountAnd(const BitSet&) const;

    int CountAndSparse(const BitSet&) const;

    // Invert this BitSet in place.
    void Invert();

    BitSet& operator|=(const BitSet&);

    // Get a string representation of this BitSet.
    std::string ToString() const;

private:
    const int WordSize = 64;
    const Word One = 1;

    // This is the internal representation of the bits.
    Word* _words = nullptr;

    int _numWords;
    int _numBits;

    int Count(Word) const;
    int CountSparse(Word) const;

    // Get a string representation for the word.
    std::string WordString(Word) const;
};

// This object allows iteration over the positions of the set bits in a BitSet.
class BitSetIterator
{
public:
    static const int NoBit = -2;

    BitSetIterator(const BitSet& bs) : _bs(bs), _i(-1) { }
    int Next();

private:
    const BitSet& _bs;
    int _i;
};

#endif // __BITSET_H__
