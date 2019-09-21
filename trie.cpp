// flexiuni separate prin virgula + rescris consumeInflections
// TODO: "aş" is not in inflections.in!!!


// DONE: ori folosim GENMODE, ori folosim mode, ori nu folosim niciuna
//        (adică ~(GENMODE & mode) LOL, adică fix ~(ce avem acum), și am gândit-o super natural, super repede
//            e fascinant cum mintea noastră gândește corect logic fără să își pună explicit logic problemele)

#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cstdlib>

#include "trie.hpp"
#include "bits_ops.hpp"
#include "language_support.hpp"
#include "special_characters.hpp"

using namespace std;

// Set mode on false (BUILD_MODE), if we build the trie (we don't use configuration).
trie::trie(bool alloc) {
  if (alloc) {
    // Initialize the staticTrie
    
    // Properties of the namespace
    size = SIZE + 1;
    sigma = romanian::ROMANIAN_SIGMA;
    
    assert(sigma <= MAX_ACCEPTED_SIGMA);
    full_bits = (sigma == MAX_ACCEPTED_SIGMA) ? 0xffffffff : ((1u << sigma) - 1);
    bits_sigma = trie::computeCountOfBits(sigma);
    mask_sigma = (1u << bits_sigma) - 1;
    
    // Alloc staticTrie
    bufferPos = 0;
    mode = BUILD_MODE;
    staticTrie = new trieNode[size];
    for (int i = 0; i < size; i++) {
      staticTrie[i].code = 0;
      staticTrie[i].parent = 0;
      staticTrie[i].sons = new uint32_t[sigma]();
      staticTrie[i].configuration = full_bits;
    }
    
    // Also auxTrie
    auxTrie_size = 0;
    auxTrie = nullptr;
  } else {
    mode = READ_MODE;
  }
}

// Return the current size of the trie
uint32_t trie::getSize() {
  return bufferPos + 1;
}

// if we read from a binary file, mode must be set on true (READ_MODE)
trie::trie(const char* filename) {
  mode = READ_MODE;
  
  // Initialize auxTrie
  auxTrie_size = 0;
  auxTrie = nullptr;
  loadExternal(filename);
}

trie::~trie() {
  for (unsigned i = 0; i < bufferPos; i++) {
    if (staticTrieAccess(i)->sons != nullptr)
      delete[] staticTrieAccess(i)->sons;
  }
  delete[] staticTrie;
  
  // Free auxTrie, if it has been reallocated
  if ((mode != BUILD_MODE) && (auxTrie_size))
    free(auxTrie);
}

trieNode* trie::staticTrieAccess(uint32_t ptr) {
  // Included in staticTrie?
  if (ptr < size)
    return staticTrie + ptr;
  
  // No? Then use auxTrie
  int index = ptr - size;
  int oldsize = auxTrie_size;
  
  // Is index contained in the old auxtrie?
  if (index >= oldsize) {
    // Check for the first entrance in this case (auxTrie_size == 0)
    auxTrie_size = (!auxTrie_size) ? EXPAND_TRIE_CONSTANT : (auxTrie_size * EXPAND_TRIE_CONSTANT);
    
    // Realloc the entire auxTrie
    auxTrie = (trieNode*)realloc(auxTrie, auxTrie_size * sizeof(trieNode));
    
    // Init the new allocated entries
    for (int i = oldsize; i < auxTrie_size; i++) {
      auxTrie[i].code = 0;
      auxTrie[i].parent = 0;
      
      // TODO: Alex suggested (sigma + 1). I think, there was the problem with ~0.
      // In ~0 there are 32 set bits. But we need only 31.
      // Now it should work as it's now. But still, check for that.
      auxTrie[i].sons = new uint32_t[sigma]();
      auxTrie[i].configuration = full_bits; // 31 bits
    }
  }
  return auxTrie + index;
}

void trie::updateFreqOfPointer(int32_t ptr) {
  // If derivated, increase frequency for root
  // If root, increase frequency for itself
  
  // Reminder: the first bit of code is occupied - that's why += 2!
  if (!(staticTrieAccess(ptr)->code & 1))
    staticTrieAccess(staticTrieAccess(ptr)->code >> 1)->code += 2;
  else
    staticTrieAccess(ptr)->code += 2;
}

// Updade frequency of word. If not found, add it as a as-is word
void trie::tryUpdateFreq(string word) {
  int32_t lastPos;
  int32_t ptr = search(word, lastPos);
  // assert(ptr != ROOT);
  
  if (ptr < 0) {
    int32_t finalPtr;
    insert(-ptr, word, -1, lastPos, finalPtr);
    updateFreqOfPointer(finalPtr);
  } else if (ptr > 0) {
    updateFreqOfPointer(ptr);
  }
}

// The strategy is to add the latin variant, if the normal one hasn't been found or it's not a romanian word
void trie::updateFreq(string word, std::string latin_word = "") {
  if (!romanian::isRomanian(word)) {
    tryUpdateFreq(latin_word);
  } else {
    int32_t lastPos;
    int32_t ptr = search(word, lastPos);
    // assert(ptr != ROOT);
    
    if (ptr > 0)
      updateFreqOfPointer(ptr);
    else
      tryUpdateFreq(latin_word);
  }
}

// Enlarge the vector of sons - the edge of "pos" points now to "goesTo" 
// The idea is to fully alloc the vector of sons
void trie::updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo) {
  // Get the number of sons
  int countOfSons = bitOp::countOfBits(staticTrieAccess(ptr)->configuration);
  
  // Create temporary copy of the sons
  uint32_t* temp = new uint32_t[countOfSons]();
  for (int i = 0; i < countOfSons; i++) {
    temp[i] = staticTrieAccess(ptr)->sons[i];
  }
  delete[] staticTrieAccess(ptr)->sons;
  
  // Alloc a new vector of sons
  staticTrieAccess(ptr)->sons = new uint32_t[sigma]();
  
  // Re-establish the sons. 
  for (int i = 0; i < sigma; i++) {
    if (bitOp::getBit(staticTrieAccess(ptr)->configuration, i))
      staticTrieAccess(ptr)->sons[i] = temp[bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, i)];
    else
      staticTrieAccess(ptr)->sons[i] = 0;
  }
  
  // Update the configuration
  staticTrieAccess(ptr)->configuration = full_bits;
  
  // Let the edge "pos" point to "goesTo"
  staticTrieAccess(ptr)->sons[pos] = goesTo;
}

// Insert in trie "str", the position to be regarded is now pos. "connect" is the node which str should be connected with.
// If "str" itself is a rootWord, connect must be set on -1.
void trie::insert(int32_t ptr, std::string str, int32_t connect, uint32_t pos, int32_t& finalPtr) {
  // It isn't <= because even with a diacritica we arrive at the end of str.
  if (str.size() == pos) {
    // to keep track of which words are root words, and which words are derived from root words, the "code" variable of
    // each trie node will use its 0th bit set to 1 if the word is a root word, and to 0 otherwise.
    // if the 0th bit is set to 1, then all other bits keep the frequency of the word in the input text
    // if the 0th bit is set to 0, then all other bits keep the index of its root word in the trie array
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
    if (!hideme && (staticTrieAccess(ptr)->code >> 1) && ((uint32_t)connect) != (staticTrieAccess(ptr)->code >> 1)) {
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
    // wtf was that, right?
    // I use  to tell the program whether I want to generate a new dictionary, or I want to update an existing
    // one. If I want to generate a new dictionary, ie. use the inflexions file from Domi, the frequency of every word I
    // insert in the dictionary *has to be 0*. Regardless of how many times I read it from the inflexions file. So if
    // GENMODE is defined, I simply no longer increment each word's frequency.
    // If, however, I want to update an existing dictionary, I'll need updated frequencies.
    return;
  }
  int32_t encoding = (pos == str.size() - 1) ? 
                      romanian::encode(str[pos]) : 
                      romanian::diacriticEncode(str[pos], str[pos + 1]);
  
  // TODO: Alex, nu inteleg de ce mode era pe true, adica READ_MODE?
  // If the next edge does not yet exist, create it.
  if (((mode == READ_MODE) && (!bitOp::getBit(staticTrieAccess(ptr)->configuration, encoding))) || (!staticTrieAccess(ptr)->sons[encoding])) {
    if (staticTrieAccess(ptr)->configuration != full_bits)
      updateMihailsJmenuri(ptr, encoding, ++bufferPos);
    else
      staticTrieAccess(ptr)->sons[encoding] = ++bufferPos;
    
    // Save the parent and also the encoding of the edge, which represents the index of the character in alphabet
    staticTrieAccess(bufferPos)->parent = (ptr << bits_sigma) | encoding;
  }
  
  // If encoding shows we reached a diacritica, move to the next byte
  insert(staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != full_bits ? 
        bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : 
        encoding], 
        str, 
        connect, 
        pos + 1 + romanian::isDiacritic(str[pos]), finalPtr);
}

void trie::addRoot(string str) {
  int dummy;
  insert(ROOT, str, -1, 0, dummy);
}

void trie::addDerivated(string root, std::string derivated) {
  if (derivated == root)
    return;
  int dummy, root_pointer = search(root, dummy);
  insert(ROOT, derivated, root_pointer, 0, dummy);
}

// Good for tests: if str is a rootWord returns the pointer where it was saved. Otherwise, the pointer of its rootword.
int32_t trie::search(string str, int& lastPos) {
  int32_t ptr, pos, encoding;
  bool toCheck;
  ptr = 0;
  for (pos = 0; pos < str.size();) {
    // Get the encoding
    encoding = (pos == str.size() - 1) ? romanian::encode(str[pos]) : romanian::diacriticEncode(str[pos], str[pos + 1]);
    
    toCheck = (staticTrieAccess(ptr)->configuration != full_bits) ? bitOp::getBit(staticTrieAccess(ptr)->configuration, encoding) : staticTrieAccess(ptr)->sons[encoding];
    if (!toCheck) {
      lastPos = pos;
      return -ptr;
    }
    ptr = staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != full_bits ? bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : encoding];
    pos += 1 + romanian::isDiacritic(str[pos]);
  }
  return (staticTrieAccess(ptr)->code & 1) ? ptr : (staticTrieAccess(ptr)->code / 2);
}

void trie::consumeInflexions(const char* filename, const char* latin_filename) {
  assert(mode == BUILD_MODE);
  ifstream latin_in(latin_filename);
  ifstream in(filename);
  
  int32_t latin_total, latin_infl, total, infl;
  std::string latin_word, latin_inflexion, word, inflexion;
  std::string latin_aux, aux;
  
  // Read both variants
  in >> total;
  latin_in >> latin_total;
  
  // Compare for equality of count of words
  assert(latin_total == total);
  
  for (int i = 0; i < total; i++) {
    // Read both variants
    in >> word >> aux;
    latin_in >> latin_word >> latin_aux;
    
    // Clean up both variants
    specialChars::cleanUpWord(word);
    specialChars::cleanUpWord(latin_word);
    
    // Check for invalid lines - TODO: still work on it! PS (pount sign) :)
    if ((!isdigit(aux[0])) || (latin_word == "PS")) {
      // std::cerr << "serios?____________________________________" << std::endl;
      
      // Consume both lines
      getline(in, word);
      getline(latin_in, latin_word);
      // TODO: pe asfintite. Add separator in inflextions
      continue;
    }
    
    // Check if the word is indeed a romanian one.
    bool latin_is_root = false;
    if (!romanian::isRomanian(word)) {
      latin_is_root = true;
      addRoot(latin_word);
    } else {
      addRoot(word);
      
      // And add the latin word as a derivated of the root
      addDerivated(word, latin_word);
    }

    unsigned countOfInflexions = stoi(aux);
    for (unsigned j = 0; j < countOfInflexions; j++) {
      // Read both words
      in >> inflexion;
      latin_in >> latin_inflexion;
      
      // Clean up both words
      specialChars::cleanUpWord(inflexion);
      specialChars::cleanUpWord(latin_inflexion);
      
      if (latin_is_root) {
        // This means, the root is not a romanian word. But its derivates could be romanian
        if (romanian::isRomanian(inflexion)) {
          addDerivated(latin_word, inflexion);
          
          // And add the latin variant of the inflexion if it does not exist
          if (latin_inflexion != inflexion)
            addDerivated(latin_word, latin_inflexion);
        } else {
          addDerivated(latin_word, latin_inflexion);
        }
      } else {
        // This mens, the root is romanian. We are sure that the inflexion also is romanian.
        addDerivated(word, inflexion);
        
        // And add the latin variant of the inflexion if it does not exist
        if (latin_inflexion != inflexion)
          addDerivated(word, latin_inflexion);
      }
    }
  }
  // Close both files
  latin_in.close();
  in.close();
}

// Returns the parent in trie of "ptr" and also saves the type of edge that lies between them.
// TODO: update configuration during insert. Not necessary! because we only build a trie with sons at full size (SIGMA).
uint32_t trie::findParent(int32_t ptr, int32_t& encoding) {
  if (ptr == 0)
    return -1;
  encoding = (staticTrieAccess(ptr)->parent & mask_sigma);
  return (staticTrieAccess(ptr)->parent >> bits_sigma);
}

// create the word that ends in ptr.
string trie::formWord(int32_t ptr) {
  std::string ret;
  if (ptr <= 0)
    return ret;
  int32_t encoding;
  int32_t nextPtr = findParent(ptr, encoding);
  return formWord(nextPtr) + romanian::decode(encoding);
}

#if 0
void trie::compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b) {
  if (bitOp::countOfBits(staticTrieAccess(root)->configuration) == 1) {
    if (!current)
      last = root;
    if (++current > max) {
      max = current;
      b = root;
      a = last;
    }
    if (last != root)
      avg++;
    compressionTest(staticTrieAccess(root)->sons[0], current, max, avg, last, a, b);
    return;
  }
  for (auto i = 0; i < countOfBits(staticTrieAccess(root)->configuration); i++) {
    compressionTest(staticTrieAccess(root)->sons[i], 0, max, avg, root, a, b);
  }
}
#endif

void trie::getAllFreqs(int root, std::vector<pair<int, std::string>>& init_vector) {
  // Push the frequence of from this node
  if ((staticTrieAccess(root)->code & 1) && (staticTrieAccess(root)->code >> 1))
    init_vector.push_back(make_pair(staticTrieAccess(root)->code >> 1, formWord(root)));
  
  // And go further with the children
  for (unsigned i = 0; i < bitOp::countOfBits(staticTrieAccess(root)->configuration); i++)
    if (staticTrieAccess(root)->sons[i] != 0)
      getAllFreqs(staticTrieAccess(root)->sons[i], init_vector);
}
