#ifndef INFLEX_ENGINE_H
#define INFLEX_ENGINE_H

#include <vector>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <fstream>

#include "bits_ops.hpp"
#include "language_support.hpp"
#include "special_characters.hpp"

#pragma pack(push, 1)
struct InflexEngineNode {
  uint32_t code;
  uint32_t parent;
  uint32_t configuration;
  uint32_t* sons;
  
  InflexEngineNode() {  
    this->code = 0;
    this->parent = 0;
    this->configuration = 0;
    this->sons = nullptr;
  }
};
#pragma pack(pop)

class InflexEngine;

class InflexEngine 
{
  public:
  enum Mode {
    Read,
    Load,
    Build
  };
    
  private:
  static constexpr unsigned SIZE = 2500000;
  static constexpr unsigned MAX_ACCEPTED_SIGMA = 32;   // all bits occupied
  static constexpr unsigned EXPAND_TRIE_CONSTANT = 16; // needed when expanding the trie
  static constexpr unsigned ROOT = 0;                  // the index in staticTrie of the root
  
  // The generation mode (Read, Load or Build)
  Mode generationMode;
  // The tries
  InflexEngineNode* staticTrie;
  InflexEngineNode* auxTrie;
  // The access on the auxiliar trie 
  InflexEngineNode* staticTrieAccess(uint32_t ptr);
  // The current sizes of both tris
  uint32_t size, auxTrieSize;
  // The properties of the namespace (alphabet)
  uint32_t sigma, fullBits;
  // The last used pointer in trie
  uint32_t bufferLastPtr;
  // The update of frequency for this pointer
  void updateFreqOfPointer(int32_t ptr);
  // Where the core idea for the performance lies 
  void updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo);
  // The function used by the original update of frequency
  void tryUpdateFreq(std::string word);
  // The getter of parent and encoding of the current pointer
  int32_t findParent(int32_t ptr, int32_t& encoding);
  // The internal function for computing the frequencies
  void getAllFreqs(uint32_t root, std::vector<std::pair<uint32_t, std::string>>& init_vector);
  // The dealloc function used by deconstructor
  void dealloc_();
  
  public:
  // The constructor
  InflexEngine(Mode mode, const char* filename = nullptr, const char* latinFileName = nullptr);
  // The deconstructor
  ~InflexEngine();
  
  // The getter of the size of InflexEngine
  uint32_t getSize();
  // The function used outside InflexEngine to reset the InflexEngine
  void reset(const char* filename);
  // The functions which insert the root-word and its inflections into InflexEngine
  void addRoot(std::string str), addDerivated(std::string root, std::string derivated);
  // The core-functions: search
  int32_t search(std::string str, int32_t& lastPos);
  // The core-functions: insert
  void insert(int32_t ptr, std::string str, int32_t connect, uint32_t pos, int32_t& finalPtr);
  // The functions to form the word 
  std::string formWord(int32_t ptr);
  // The function for updating the frequencies while parsing a text
  void updateFreq(std::string word, std::string latinWord);
  // The function for computing the frequencies
  std::vector<std::pair<uint32_t, std::string>> getFrequencies();
  // The function for dumping the computed frequencies
  void showFreqs(std::string filename);
  // Build the InflexEngine with the inflexions from dexonline.ro
  void consumeInflexions(const char* filename, const char* latinFilename);
  // Load the InflexEngine from an external binary file
  void loadExternal(const char* filename);
  // Save the InflexEngine into an external binary file
  void saveExternal(const char* filename);
#if 0
  // Temporary functions
  void compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b);
#endif
};

#endif // INFLEX_ENGINE_H
