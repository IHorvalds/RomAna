#ifndef TRIE_H
#define TRIE_H

#include <vector>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <fstream>

#include "bits_ops.hpp"
#include "language_support.hpp"
#include "special_characters.hpp"

#pragma pack(push, 1)
struct trieNode {
  uint32_t code;
  uint32_t parent;
  uint32_t configuration; // keep it with signed
  uint32_t* sons;
  
  trieNode() {  
    this->code = 0;
    this->parent = 0;
    this->configuration = 0;
    this->sons = nullptr;
  }
};
#pragma pack(pop)

class trie;

class trie 
{
  private:
  static constexpr unsigned SIZE = 3330000;
  static constexpr unsigned EXPAND_TRIE_CONSTANT = 16;
  static constexpr unsigned MAX_ACCEPTED_SIGMA = 32;
  static constexpr unsigned ROOT = 0;
  
  // Modes of trie
  // TODO: make them public enums
  static constexpr bool BUILD_MODE = false;
  static constexpr bool READ_MODE = true;
  
  trieNode* staticTrie;
  trieNode* auxTrie;
  
  // current sizes of both tries
  uint32_t size, auxTrie_size;
  
  // Properties of the alphabet
  uint32_t sigma, full_bits, bits_sigma, mask_sigma; // formula of mask_sigma = (1 << bits_sigma) - 1
  
  // Last used pointer in trie
  uint32_t bufferPos;
  
  // If trie loaded from binary -> mode == true (READ_MODE)
  bool mode;
  
  // Update the frequency for this pointer
  void updateFreqOfPointer(int32_t ptr);
  
  // Used for adding new words
  void updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo);
  trieNode* staticTrieAccess(uint32_t ptr);

  // If sigma is a power of 2, we need log + 1. Example 2^5 -> 6 bits
  // Otherwise, simply rounded log. Example 26 -> 5 bits 
  uint32_t computeCountOfBits(unsigned curr_sigma) {
    uint32_t log = ceil(log2(curr_sigma));
    // Check if pow of 2
    return log + ((curr_sigma & (curr_sigma - 1)) == 0);
  }
  
  public:
  trie();
  trie(bool alloc);
  trie(const char* filename);
  ~trie();
  
  // Functions used within de/constructor 
  void dealloc_();
  void prepare(const char* filename);
  void reset(const char* filename);
  
  // Size of trie
  uint32_t getSize();
  
  // Functions for adding the romanian words
  void addRoot(std::string str);
  void addDerivated(std::string root, std::string derivated);

  // Typical functions for such a data structure
  void insert(int32_t ptr, std::string str, int32_t connect, uint32_t pos, int32_t& finalPtr);
  int search(std::string str, int& lastPos);
  
  // Functions to get the parent of each word
  std::string formWord(int32_t ptr);
  uint32_t findParent(int32_t ptr, int32_t& encoding);
  
  // Functions for updating the frequencies while parsing a text
  void tryUpdateFreq(std::string word);
  void updateFreq(std::string word, std::string latin_word);
  
  // Build the trie with the inflexions from dexonline.ro
  void consumeInflexions(const char* filename, const char* latin_filename);
  
  // Load class members from external file
  void loadExternal(const char* filename) {
    std::ifstream in;
    in.open(filename, std::ios::in | std::ios::binary);

    in.read((char*) &size, sizeof(uint32_t));
    in.read((char*) &sigma, sizeof(uint32_t));
    in.read((char*) &full_bits, sizeof(uint32_t));
    in.read((char*) &bits_sigma, sizeof(uint32_t));
    in.read((char*) &mask_sigma, sizeof(uint32_t));
    
    bufferPos = size;
    staticTrie = new trieNode[size];
    for (unsigned i = 0; i < size; i++) {
      in.read((char*) &staticTrie[i].code, sizeof(uint32_t));
      in.read((char*) &staticTrie[i].configuration, sizeof(uint32_t));
      in.read((char*) &staticTrie[i].parent, sizeof(uint32_t));
      unsigned howMany = bitOp::countOfBits(staticTrie[i].configuration);
      if (howMany) {
        staticTrie[i].sons = new uint32_t[howMany];
        in.read((char*) staticTrie[i].sons, howMany * sizeof(uint32_t));
      }
    }
  }


  // Save class members to external file
  // TODO: there is somewhere here a leak of reading over 4 bytes
  void saveExternal(const char* filename) {
    std::ofstream out;
    out.open(filename, std::ios::out | std::ios::binary);
    
    // "+ 1" because bufferPos points on the last used pointer
    int writeSize = bufferPos + 1;
    out.write((char*) &writeSize, sizeof(int32_t));
    out.write((char*) &sigma, sizeof(uint32_t));
    out.write((char*) &full_bits, sizeof(uint32_t));
    out.write((char*) &bits_sigma, sizeof(uint32_t));
    out.write((char*) &mask_sigma, sizeof(uint32_t));
    
    for (unsigned i = 0; i < writeSize; i++) {
      unsigned countOfSons = bitOp::countOfBits(staticTrieAccess(i)->configuration);
      if (staticTrieAccess(i)->configuration == full_bits) {
        // Computes configuration (the sons with non-zero values).
        staticTrieAccess(i)->configuration = 0;
        for (unsigned j = 0; j < sigma; j++)
          if (staticTrieAccess(i)->sons[j])
            staticTrieAccess(i)->configuration |= (1u << j);
      }
      out.write((char*) &staticTrieAccess(i)->code, sizeof(uint32_t));
      out.write((char*) &staticTrieAccess(i)->configuration, sizeof(uint32_t));
      out.write((char*) &staticTrieAccess(i)->parent, sizeof(uint32_t));

      // Writes only the sons with non-zero values.
      for (unsigned j = 0; j < countOfSons; j++)
        if (staticTrieAccess(i)->sons[j])
          out.write((char*) &staticTrieAccess(i)->sons[j], sizeof(uint32_t));
    }
    out.close();
  }

#if 0
  void compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b);
#endif

  // Compute the frequencies (wrapper)
  std::vector<std::pair<uint32_t, std::string>> getFrequencies() {
    std::vector<std::pair<uint32_t, std::string>> dummy;
    getAllFreqs(ROOT, dummy);
    return dummy;
  }
  
  // Compute the frequencies
  void getAllFreqs(int root, std::vector<std::pair<uint32_t, std::string>>& init_vector);
  void showFreqs(std::string filename) {
    std::ofstream out(filename);
    
    // Get all frequencies from trie
    std::vector<std::pair<uint32_t, std::string>> init_vector;
    getAllFreqs(ROOT, init_vector);

    // Sort the accumulated frequencies. Where equality between frequencies, prefer lexicografically order on words
    std::sort(init_vector.begin(), init_vector.end(), [](auto& left, auto& right) {
      return (left.first > right.first) || 
             ((left.first == right.first) && (left.second < right.second));
    });
    
    // Compute the sum of all frequencies
    double sumOfFreqs = 0;
    for (auto iter = init_vector.begin(); iter != init_vector.end(); ++iter)
      sumOfFreqs += iter->first;

    // And print them as word - (frequency, ratio)
    for (auto iter = init_vector.begin(); iter != init_vector.end(); ++iter)
      out << std::setprecision(3) << iter->second << " (" << iter->first << ", " << (iter->first / sumOfFreqs) * 100 << "%)" << std::endl;
  }
};

#endif // TRIE_H
