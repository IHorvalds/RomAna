#ifndef TRIE_H
#define TRIE_H

using namespace std;

#include <vector>
#include <iomanip>
#include <cmath>

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
  trie(const char* filename);
  ~trie();
  
  // Size of trie
  uint32_t getSize();
  
  // Functions for adding the romanian words
  void addRoot(string str);
  void addDerivated(string root, string derivated);

  // Typical functions for such a data structure
  void insert(int32_t ptr, string str, int32_t connect, uint32_t pos, int32_t& finalPtr);
  int search(string str, int& lastPos);
  
  // Functions to get the parent of each word
  string formWord(int32_t ptr);
  uint32_t findParent(int32_t ptr, int32_t& encoding);
  
  // Functions for updating the frequencies while parsing a text
  void tryUpdateFreq(string word);
  void updateFreq(string word, string latin_word);
  
  // Build the trie with the inflexions from dexonline.ro
  void consumeInflexions(const char* filename, const char* latin_filename);
  
  // Functions with files
  void loadExternal(const char* filename);
  void saveExternal(const char* filename);
  
#if 0
  void compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b);
#endif

  // Compute the frequencies
  void getAllFreqs(int root, std::vector<pair<int, string>>& init_vector);
  void showFreqs(string filename) {
    ofstream out(filename);
    
    // Get all frequencies from trie
    std::vector<pair<int, string>> init_vector;
    getAllFreqs(0, init_vector);

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
      out << setprecision(3) << iter->second << " (" << iter->first << ", " << (iter->first / sumOfFreqs) * 100 << "%)" << std::endl;
  }
};

#endif // TRIE_H