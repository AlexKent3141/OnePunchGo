#ifndef __BITSET_H__
#define __BITSET_H__

// Bit set class which is used for quick board-wise operations.
class BitSet
{
public:
    BitSet() = delete;
    BitSet(size_t);
    BitSet(const BitSet&);
    ~BitSet();

private:
    typedef uint64_t Word;

    // This is the internal representation of the bits.
    Word* _words = nullptr;
};

#endif // __BITSET_H__
