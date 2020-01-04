// TODO: "aş" is not in lexems.in!!!
#include <vector>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
#include "InflexEngine.hpp"

InflexEngineNode* InflexEngine::staticTrieAccess(uint32_t ptr) {
// Performs a black-box access on the modified trie, when it gets expanded
  // Included in staticTrie?
  if (ptr < size)
    return staticTrie + ptr;
  
  // No? Then use auxTrie
  int32_t index = ptr - size;
  int32_t oldSize = auxTrieSize;
  
  // Is index contained in the old auxInflexEngine?
  if (index >= oldSize) {
    // Check for the first entrance in this case (auxTrieSize == 0)
    auxTrieSize = (!auxTrieSize) ? EXPAND_TRIE_CONSTANT : (auxTrieSize * EXPAND_TRIE_CONSTANT);
    
    // Realloc the entire auxTrie
    auxTrie = (InflexEngineNode*)realloc(auxTrie, auxTrieSize * sizeof(InflexEngineNode));
    
    // Init the new allocated enInflexEngines
    for (unsigned i = oldSize; i < auxTrieSize; i++) {
      auxTrie[i].code = 0;
      auxTrie[i].parent = 0;      
      auxTrie[i].sons = new uint32_t[sigma]();
      auxTrie[i].configuration = fullBits; // 31 bits
    }
  }
  return auxTrie + index;
}

void InflexEngine::updateFreqOfPointer(int32_t ptr) {
  // If derivated, increase frequency for root
  // If root, increase frequency for itself
  // Reminder: the first bit of code is occupied - that's why += 2!
  if (!(staticTrieAccess(ptr)->code & 1))
    staticTrieAccess(staticTrieAccess(ptr)->code >> 1)->code += 2;
  else
    staticTrieAccess(ptr)->code += 2;
}

void InflexEngine::tryUpdateFreq(std::string word) {
// Updade frequency of word. If not found, add it as a as-is word
  int32_t lastPos, ptr = search(word, lastPos);
  if (ptr < 0) {
    int32_t finalPtr;
    insert(-ptr, word, -1, lastPos, finalPtr);
    updateFreqOfPointer(finalPtr);
  } else if (ptr > 0) {
    updateFreqOfPointer(ptr);
  }
}

void InflexEngine::updateFreq(std::string word, std::string latinWord = "") {
// The strategy: add the latin variant, if the normal one hasn't been found or it's not a romanian word
  // Does it contain non-romanian characters
  if (!romanian::isRomanian(word)) {
    tryUpdateFreq(latinWord);
  } else {
    int32_t lastPos, ptr = search(word, lastPos);
    
    // Romanian word found?
    if (ptr > 0)
      updateFreqOfPointer(ptr);
    else
      tryUpdateFreq(latinWord);
  }
}

void InflexEngine::updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo) {
// Enlarge the vector of sons - the edge of "pos" points now to "goesTo" 
// The idea is to fully alloc the vector of sons
  // Get the number of sons
  size_t countOfSons = bitOp::countOfBits(staticTrieAccess(ptr)->configuration);
  
  // Create temporary copy of the sons
  uint32_t* temp = new uint32_t[countOfSons]();
  for (unsigned index = 0; index < countOfSons; index++)
    temp[index] = staticTrieAccess(ptr)->sons[index];
  delete[] staticTrieAccess(ptr)->sons;
  
  // Alloc a new vector of sons
  staticTrieAccess(ptr)->sons = new uint32_t[sigma]();
  
  // Re-establish the sons. 
  for (unsigned index = 0; index < sigma; index++) {
    if (bitOp::getBit(staticTrieAccess(ptr)->configuration, index))
      staticTrieAccess(ptr)->sons[index] = temp[bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, index)];
    else
      staticTrieAccess(ptr)->sons[index] = 0;
  }
  
  // Update the configuration
  staticTrieAccess(ptr)->configuration = fullBits;
  
  // Let the edge "pos" point to "goesTo"
  staticTrieAccess(ptr)->sons[pos] = goesTo;
}

void InflexEngine::insert(int32_t ptr, std::string str, int32_t connect, uint32_t pos, int32_t& finalPtr) {
// Insert in InflexEngine "str", the position to be regarded is now pos. "connect" is the node which str should be connected with.
// If "str" itself is a rootWord, connect must be set on -1.
  // It is not <= because even with a diacritica we arrive at the end of str.
  if (str.size() == pos) {
    // to keep track of which words are root words, and which words are derived from root words, the "code" variable of
    // each InflexEngine node will use its 0th bit set to 1 if the word is a root word, and to 0 otherwise.
    // if the 0th bit is set to 1, then all other bits keep the frequency of the word in the input text
    // if the 0th bit is set to 0, then all other bits keep the index of its root word in the InflexEngine array
    // Note:
    // + 2 in decimal <=> + 10 in binary, ie. add 1 to all bits but the 0th
    // * 2 in decimal <> << 1 in binary, ie. make sure 0th bit stays 0
    uint32_t hideme = (connect == -1);

    // why the third condition? - question for Alex.
    // Alex: good question
    if (hideme && (staticTrieAccess(ptr)->code & 1)) {
      // daca intai a fost root, iar acum vrea din nou sa fie root.
      // Exemplu: abate are 2 liste in inflections.in
      return;
    }
    if (hideme && !(staticTrieAccess(ptr)->code & 1) && (staticTrieAccess(ptr)->code != 0)) {
      // daca intai a fost trimis ca derivat, iar acum imi cere sa-l fac root
      // ce fac? il las derivat, n-a zis sudo. adica nu mai modific nimic
      return;
    }
    if (!hideme && (staticTrieAccess(ptr)->code & 1)) {
      // daca intai a fost trimis ca root, si acum vrea derivat
      // ce fac? e bou, il las root, cum sa cedezi puterea? adica nu mai modific nimic
      return;
    }
    if (!hideme && (staticTrieAccess(ptr)->code >> 1) && static_cast<uint32_t>(connect) != (staticTrieAccess(ptr)->code >> 1)) {
      // daca deja e derivatul cuiva, si acum vrea sa devina derivatul altcuiva
      // stai cuminte la casa ta, ca nu e dupa tine aici. adica nu mai modific nimic
      return;
    }
    if (!hideme && (!(staticTrieAccess(connect)->code & 1))) {
      // daca vrea să devină derivat al unui derivat al unei rădăcini
      // nouă nu ne place birocrația și nici ierarhia. devino direct derivat al rădăcinii
      // ie. path-compression
      connect = staticTrieAccess(connect)->code >> 1;
    }
    staticTrieAccess(ptr)->code = (hideme) ? 1 : connect * 2;
    finalPtr = ptr;
    return;
  }
  int32_t encoding = (pos == str.size() - 1) ? 
                      romanian::encode(str[pos]) : 
                      romanian::diacriticEncode(str[pos], str[pos + 1]);
                      
  // If the coming edge does not yet exist, create it.
  // There are 2 cases: either we build up the InflexEngine or not.
  // In the latter, we can without any problems use the "configuration" of the current node.
  // But if we build up the InflexEngine, we do not have any configuration, so we only work with the pointers to the sons.
  if (((generationMode != Build) && (!bitOp::getBit(staticTrieAccess(ptr)->configuration, encoding))) || (!staticTrieAccess(ptr)->sons[encoding])) {
    // If the configuration is not at its fullest, we decide to expand the node.
    // Please note here that "configuration" in "Build"-mode has been already set to "fullBits", so we cannot get in the "updateMihailsJmenuri" 
    if (staticTrieAccess(ptr)->configuration != fullBits)
      updateMihailsJmenuri(ptr, encoding, ++bufferLastPtr);
    else
      staticTrieAccess(ptr)->sons[encoding] = ++bufferLastPtr;
    
    // Save the parent and also the encoding of the edge, which represents the index of the character in alphabet
    staticTrieAccess(bufferLastPtr)->parent = ptr;
  }
  
  // If encoding shows we reached a diacritica, move to the next byte
  insert(staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != fullBits ? 
        bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : 
        encoding],
        str,
        connect,
        pos + 1 + romanian::isDiacritic(str[pos]), finalPtr);
}

int32_t InflexEngine::search(std::string str, int32_t& lastPos) {
// Good for tests: if str is a rootWord returns the pointer where it was saved. Otherwise, the pointer of its rootword.
  int32_t ptr, pos, encoding;
  bool toCheck;
  ptr = 0;
  for (pos = 0; pos < str.size();) {
    // Get the encoding
    encoding = (pos == str.size() - 1) ? romanian::encode(str[pos]) : romanian::diacriticEncode(str[pos], str[pos + 1]);
    
    toCheck = (staticTrieAccess(ptr)->configuration != fullBits) ? bitOp::getBit(staticTrieAccess(ptr)->configuration, encoding) : staticTrieAccess(ptr)->sons[encoding];
    if (!toCheck) {
      lastPos = pos;
      return -ptr;
    }
    ptr = staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != fullBits ? bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : encoding];
    pos += 1 + romanian::isDiacritic(str[pos]);
  }
  return (staticTrieAccess(ptr)->code & 1) ? ptr : (staticTrieAccess(ptr)->code / 2);
}

int32_t InflexEngine::findParent(int32_t ptr, int32_t& encoding) {
// Returns the parent in InflexEngine of "ptr" and also saves the type of edge that lies between them.
  if (ptr == 0)
    return -1;
  uint32_t parent = staticTrieAccess(ptr)->parent, parentConfiguration = staticTrieAccess(parent)->configuration;
  for (unsigned index = 0; index < sigma; ++index) {
    if (bitOp::getBit(parentConfiguration, index) && (staticTrieAccess(parent)->sons[bitOp::orderOfBit(parentConfiguration, index)] == ptr))
      encoding = index;
  }
  return parent;
}

std::string InflexEngine::formWord(int32_t ptr) {
// Create the word that ends in ptr.
  std::string ret;
  if (ptr <= 0)
    return ret;
  int32_t encoding, nextPtr = findParent(ptr, encoding);
  return formWord(nextPtr) + romanian::decode(encoding);
}
