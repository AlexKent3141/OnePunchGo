#ifndef __BITSET_H__
#define __BITSET_H__

#include <cstdint>

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

    // Invert this BitSet in place.
    void Invert();

    BitSet& operator|=(const BitSet&);

private:
    const int WordSize = 64;
    const Word One = 1;

    // This is the internal representation of the bits.
    Word* _words = nullptr;

    int _numWords;
    int _numBits;

    int Count(Word) const;
};

#endif // __BITSET_H__
