// TODO: there is somewhere a problem with memory-leaks. Use valgrind for that!
// TODO: Modificarea lui Alex de o linie.
// DONE: curățenie!!!
// flexiuni separate prin virgula + rescris consumeInflections


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
#include "diacritica.hpp"
#include "special_characters.hpp"

using namespace std;

// Set mode on false (BUILD_MODE), if we build the trie (we don't use configuration).
trie::trie() {
  size = SIZE + 1;
  sigma = romanian::ROMANIAN_SIGMA;
  bufferPos = 0;
  mode = BUILD_MODE;
  staticTrie = new trieNode[size];
  for (int i = 0; i < size; i++) {
    staticTrie[i].code = 0;
    staticTrie[i].parent = 0;
    staticTrie[i].sons = new uint32_t[sigma]();
    staticTrie[i].configuration = FULL_OF_BITS;
  }
}

// Return the current size of the trie
uint32_t trie::getSize() {
  return bufferPos + 1;
}

// if we read from a binary file, mode must be set on true (READ_MODE)
trie::trie(const char* filename) {
  mode = READ_MODE;
  auxTrie = nullptr;
  auxsize = 1;
  loadExternal(filename);
}

trie::~trie() {
  for (unsigned i = 0; i < bufferPos; i++) {
    if (staticTrieAccess(i)->sons != nullptr)
      delete[] staticTrieAccess(i)->sons;
  }
  delete[] staticTrie;
  if (auxTrie != nullptr)
    free(auxTrie);
}

trieNode* trie::staticTrieAccess(uint32_t ptr) {
  if (ptr < size)
    return staticTrie + ptr;
  // TODO:? should be - size?
  int index = ptr % size;
  int oldsize = auxsize - 1;
  if (index > oldsize) {
    auxsize *= EXPAND_TRIE_CONSTANT;
    auxTrie = (trieNode*) realloc(auxTrie, auxsize * sizeof(trieNode) - 1);
    for (int i = oldsize; i < auxsize - 1; i++) {
      auxTrie[i].code = 0;
      auxTrie[i].parent = 0;
      auxTrie[i].sons = new uint32_t[sigma]();
      auxTrie[i].configuration = FULL_OF_BITS;
    }
//    cout << "realloc success!" << endl;
  }
//  cout << ptr << ' ' << index << ' ' << auxTrie[index].code << ' ' << (auxTrie[index].parent >> 7) << ' ' << auxTrie[index].configuration << endl;
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
void trie::updateFreq(string word, string latin_word = "") {
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
  
  // Restablish the sons. 
  for (int i = 0; i < sigma; i++) {
    if (bitOp::getBit(staticTrieAccess(ptr)->configuration, i))
      staticTrieAccess(ptr)->sons[i] = temp[bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, i)];
    else
      staticTrieAccess(ptr)->sons[i] = 0;
  }
  
  // Update the configuration
  staticTrieAccess(ptr)->configuration = FULL_OF_BITS;
  
  // Let the edge "pos" point to "goesTo"
  staticTrieAccess(ptr)->sons[pos] = goesTo;
}

// Insert in trie "str", the position to be regarded is now pos. "connect" is the node which str should be connected with.
// If "str" itself is a rootWord, connect must be set on -1.
void trie::insert(int32_t ptr, string str, int32_t connect, uint32_t pos, int32_t& finalPtr) {
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
    if (staticTrieAccess(ptr)->configuration != FULL_OF_BITS)
      updateMihailsJmenuri(ptr, encoding, ++bufferPos);
    else
      staticTrieAccess(ptr)->sons[encoding] = ++bufferPos;
    staticTrieAccess(bufferPos)->parent = encoding | (ptr << 7);
  }
  
  // If encoding shows we reached a diacritica, move to the next byte
  insert(staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != FULL_OF_BITS ? 
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

void trie::addDerivated(string root, string derivated) {
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
    encoding = (pos == str.size() - 1) ? romanian::encode(str[pos]) : romanian::diacriticEncode(str[pos], str[pos + 1]);
    toCheck = staticTrieAccess(ptr)->configuration != FULL_OF_BITS ? bitOp::getBit(staticTrieAccess(ptr)->configuration, encoding) : staticTrieAccess(ptr)->sons[encoding];
    if (!toCheck) {
      lastPos = pos;
      return -ptr;
    }
    ptr = staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != FULL_OF_BITS ? bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : encoding];
    pos += 1 + romanian::isDiacritic(str[pos]);
  }
  return (staticTrieAccess(ptr)->code & 1) ? ptr : staticTrieAccess(ptr)->code / 2;
}

void trie::consumeInflexions(const char* filename, const char* latin_filename) {
  assert(mode == BUILD_MODE);
  ifstream latin_in(latin_filename);
  ifstream in(filename);
  
  int32_t latin_total, latin_infl, total, infl;
  string latin_word, latin_inflexion, word, inflexion;
  string latin_aux, aux;
  
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

// Load class members from external file
void trie::loadExternal(const char* filename) {
  ifstream in;
  in.open(filename, ios::in | ios::binary);

  in.read((char*) &size, sizeof(int32_t));
  in.read((char*) &sigma, sizeof(int32_t));
  
  bufferPos = size;
  staticTrie = new trieNode[size];
  for (unsigned i = 0; i < size; i++) {
    in.read((char*) &staticTrie[i].code, sizeof(uint32_t));
    in.read((char*) &staticTrie[i].configuration, sizeof(int32_t));
    in.read((char*) &staticTrie[i].parent, sizeof(int32_t));
    unsigned howMany = bitOp::countOfBits(staticTrie[i].configuration);
    if (howMany) {
      staticTrie[i].sons = new uint32_t[howMany];
      in.read((char*) staticTrie[i].sons, howMany * sizeof(uint32_t));
    }
  }
}

// Save class members to external file
// TODO: there is somewhere here a leak of reading over 4 bytes
void trie::saveExternal(const char* filename) {
  ofstream out;
  out.open(filename, ios::out | ios::binary);
  
  // "+ 1" because bufferPos points on the last used pointer
  int writeSize = bufferPos + 1;
  out.write((char*) &writeSize, sizeof(int32_t));
  out.write((char*) &sigma, sizeof(int32_t));

  for (unsigned i = 0; i < writeSize; i++) {
    unsigned countOfSons = bitOp::countOfBits(staticTrieAccess(i)->configuration);
    int dummy;
//    if (staticTrieAccess(findParent(i, dummy))->configuration == FULL_OF_BITS)
    if (staticTrieAccess(i)->configuration == FULL_OF_BITS) {
      // Computes configuration (the sons with non-zero values).
      staticTrieAccess(i)->configuration = 0;
      for (unsigned j = 0; j < sigma; j++)
        if (staticTrieAccess(i)->sons[j])
          staticTrieAccess(i)->configuration |= (1 << j);
    }
    out.write((char*) &staticTrieAccess(i)->code, sizeof(int32_t));
    out.write((char*) &staticTrieAccess(i)->configuration, sizeof(int32_t));
    out.write((char*) &staticTrieAccess(i)->parent, sizeof(int32_t));

    // Writes only the sons with non-zero values.
    for (unsigned j = 0; j < countOfSons; j++)
      if (staticTrieAccess(i)->sons[j])
        out.write((char*) &staticTrieAccess(i)->sons[j], sizeof(uint32_t));
  }
  out.close();
}

// TODO: Alex? te rog sa scrii despre ">> 7", nu am inteles. Merci!
// Returns the parent in trie of "ptr" and also saves the type of edge that lies between them.
// TODO: update configuration during insert. Not necessary! because we only build a trie with sons at full size (SIGMA).
int32_t trie::findParent(int32_t ptr, int32_t& encoding) {
  if (ptr == 0)
    return -1;
  encoding = staticTrieAccess(ptr)->parent & 63;
  return staticTrieAccess(ptr)->parent >> 7;
}

// create the word that ends in ptr.
string trie::formWord(int32_t ptr) {
  string ret;
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

void trie::getAllFreqs(int root, std::vector<pair<int, string>>& init_vector) {
  // Push the frequence of from this node
  if ((staticTrieAccess(root)->code & 1) && (staticTrieAccess(root)->code >> 1))
    init_vector.push_back(make_pair(staticTrieAccess(root)->code >> 1, formWord(root)));
  
  // And go further with the children
  for (unsigned i = 0; i < bitOp::countOfBits(staticTrieAccess(root)->configuration); i++)
    if (staticTrieAccess(root)->sons[i] != 0)
      getAllFreqs(staticTrieAccess(root)->sons[i], init_vector);
}
