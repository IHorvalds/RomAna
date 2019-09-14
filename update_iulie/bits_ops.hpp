#ifndef BITS_OPS_H
#define BITS_OPS_H

namespace bitOp {
  static void setBit(int32_t& x, unsigned index) {
    x |= (1 << index);
  }

  static bool getBit(int32_t x, unsigned index) {
    return (x >> index) & 1;
  }

  // Count the number of bits of val.
  static uint32_t countOfBits(int32_t x) {
    return __builtin_popcount(x);
  }

#if 0
  // A help-function to see the configuration (should be erased).
  void printBits(int val, int pos) {
    cerr << "An welcher Stelle steht " << pos << "?\n";
    for (int i = 31; i >= 0; i--)
      cerr << ((val >> i) & 1);
    cerr << "ret = " << countOfBits(val & ((1 << pos) - 1)) << endl;
  }
#endif
  
  // returns how many set bits are before the bit index.
  // creates a mask: 00000111111, where the count of 1s equals "index".
  static uint32_t orderOfBit(int32_t x, uint32_t index) {
    // assures that the bit "index" is set in "x", otherwise this wouldn't work.
    assert(getBit(x, index));
    return countOfBits(x & ((1 << index) - 1));
  }
};

#endif // BITS_OPS_H
