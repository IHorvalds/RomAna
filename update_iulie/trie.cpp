// TO DO
// curățenie!!!
// flexiuni separate prin virgula + rescris consumeInflections


// DONE: ori folosim GENMODE, ori folosim mode, ori nu folosim niciuna
//        (adică ~(GENMODE & mode) LOL, adică fix ~(ce avem acum), și am gândit-o super natural, super repede
//            e fascinant cum mintea noastră gândește corect logic fără să își pună explicit logic problemele)

#include <map>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cassert>
#include <cstdlib>

#include "bits_ops.hpp"
#include "diacritica.hpp"
#include "special_characters.hpp"

using namespace std;

#define SIZE 3330000
#define EXPAND_TRIE_CONSTANT 16

const int32_t FULL_OF_BITS = ~0;

#define ROOT 0

#pragma pack(push, 1)
struct trieNode {
  uint32_t code;
  uint32_t parent;
  int32_t configuration;
  uint32_t* sons;
};
#pragma pack(pop)

class trie {
  trieNode* staticTrie;
  trieNode* auxTrie;
  
  // current sizes of both tries
  int32_t size, auxsize;
  
  // the size of the alphabet
  uint32_t sigma;
  
  uint32_t bufferPos;
  uint32_t lastOrigin;
  bool mode; // if trie is implemented with configuration (so only if it is loaded from a binary), mode == true.
  int32_t leafCheck(int32_t ptr);
  void updatePtrFreq(int32_t ptr);
  void updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo);
  trieNode* staticTrieAccess(int32_t ptr);

public:
  trie();
  trie(const char* filename);
  ~trie();
  void insert(int32_t ptr, string str, int32_t connect, uint32_t pos, int32_t& finalPtr);
  void addRoot(string str);
  void addDerivated(string root, string derivated);
  int search(string str, int& lastPos);
  string formWord(int32_t ptr);
  int32_t findParent(int32_t ptr, int32_t& encoding);
  void updateFreq(string word);
  void consumeInflexions(const char* filename, const char* latin_filename);
  void loadExternal(const char* filename);
  void saveExternal(const char* filename);
  
#if 0
  void compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b);
#endif

  // Compute the frequencies
  void getAllFreqs(int root, std::vector<pair<int, string>>& init_vector);
  void showFreqs(string filename);
  
  // Size of trie
  uint32_t getSize();
};

// set mode on false, if we build the trie (we don't use configuration).
trie::trie() {
  size = SIZE + 1;
  sigma = romanian::ROMANIAN_SIGMA;
  bufferPos = 0;
  mode = false;
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
  return bufferPos;
}

// if we read from a binary file, mode must be set on true.
trie::trie(const char* filename) {
  mode = true;
  auxTrie = NULL;
  auxsize = 1;
  loadExternal(filename);
}

trie::~trie() {
  for (int i = 0; i < bufferPos; i++) {
    delete[] staticTrieAccess(i)->sons;
  }
  delete[] staticTrie;
  if (sizeof(auxTrie) != 0)
    free(auxTrie);
}

trieNode* trie::staticTrieAccess(int32_t ptr) {
  if (ptr < size)
    return staticTrie + ptr;
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

void trie::updatePtrFreq(int32_t ptr) {
  // dacă e derivat, crește frecvența pentru rădăcină
  // dacă e rădăcină, crește frecvența pentru el
  if (!(staticTrieAccess(ptr)->code & 1))
    staticTrieAccess(staticTrieAccess(ptr)->code >> 1)->code += 2;
  else
    staticTrieAccess(ptr)->code += 2;
}

void trie::updateFreq(string word) {
  int lastPos;
  int ptr = search(word, lastPos);
  if (ptr < 0) {
    int finalPtr;
    insert(-ptr, word, -1, lastPos, finalPtr);
    updatePtrFreq(finalPtr);
  }
  if (ptr > 0)
    updatePtrFreq(ptr);
}

void trie::updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo) {
  int size = bitOp::countOfBits(staticTrieAccess(ptr)->configuration);
  uint32_t* temp = new uint32_t[size]();
  for (int i = 0; i < size; i++) {
    temp[i] = staticTrieAccess(ptr)->sons[i];
  }
  delete[] staticTrieAccess(ptr)->sons;
  staticTrieAccess(ptr)->sons = new uint32_t[sigma]();
  for (int i = 0; i < sigma; i++) {
    if (bitOp::getBit(staticTrieAccess(ptr)->configuration, i))
      staticTrieAccess(ptr)->sons[i] = temp[bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, i)];
    else
      staticTrieAccess(ptr)->sons[i] = 0;
  }
  staticTrieAccess(ptr)->configuration = FULL_OF_BITS;
  staticTrieAccess(ptr)->sons[pos] = goesTo;
}

// insert in trie str, the position the be regarded is now pos. connect is the node which str should be connected with.
// If str itself is a rootWord, connect must be set on -1.
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
  int32_t encoding = (pos == str.size() - 1) ? romanian::encode(str[pos]) : romanian::diacriticEncode(str[pos], str[pos + 1]);
  // if the next edge does not yet exist, create it.
  if ((mode && !bitOp::getBit(staticTrieAccess(ptr)->configuration, encoding)) || !staticTrieAccess(ptr)->sons[encoding]) {
    if (staticTrieAccess(ptr)->configuration != FULL_OF_BITS)
      updateMihailsJmenuri(ptr, encoding, ++bufferPos);
    else {
      staticTrieAccess(ptr)->sons[encoding] = ++bufferPos;
    }
    staticTrieAccess(bufferPos)->parent = encoding | (ptr << 7);
//    cout << bufferPos << ' ' << formWord(bufferPos) << endl;
  }
  // if encoding shows we reached a diacritic character, move to next byte
  insert(staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != FULL_OF_BITS ? bitOp::orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : encoding], str, connect, pos + 1 + romanian::isDiacritic(str[pos]), finalPtr);
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

#if 0
void print_inflex(string word, string inflex, bool val) {
  if (word == "zgramboia") 
    std::cerr << inflex << " " << val << std::endl;
}
#endif

void trie::consumeInflexions(const char* filename, const char* latin_filename) {
  assert(!mode);
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
    
    countOfInflexions = stoi(aux);
    for (int j = 0; j < countOfInflexions; j++) {
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

// load class members from external file
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

// save class members to external file
void trie::saveExternal(const char* filename) {
  ofstream out;
  out.open(filename, ios::out | ios::binary);
  int writeSize = bufferPos + 1;
  out.write((char*) &writeSize, sizeof(int32_t));
  out.write((char*) &sigma, sizeof(int32_t));

  for (unsigned i = 0; i < writeSize; i++) {
    unsigned howMany = bitOp::countOfBits(staticTrieAccess(i)->configuration);
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
    for (unsigned j = 0; j < howMany; j++)
      if (staticTrieAccess(i)->sons[j])
        out.write((char*) &staticTrieAccess(i)->sons[j], sizeof(uint32_t));
  }
//  size = writeSize;
  out.close();
}

// returns the parent in trie of ptr and also saves the type of edge that lies between them.
// this function depends also on mode.
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

void trie::showFreqs(string filename) {
    ofstream out(filename);
    
    // Get all frequencies from trie
    std::vector<pair<int, string>> init_vector;
    getAllFreqs(0, init_vector);

    // Sort the accumulated frequencies
    std::sort(init_vector.begin(), init_vector.end(), [](auto& left, auto& right) {
      return left.first > right.first;
    });
    
    // And print them
    for (auto iter = init_vector.begin(); iter != init_vector.end(); ++iter)
      out << iter->second << " " << iter->first << std::endl;
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << argv[0] << " file read(0 -> build_trie, 1 -> read_only) find_me" << std::endl;
    return -1;
  }
  
  const char* file_name = argv[1];
  string tmp = file_name;
  tmp = "latin_" + tmp;
  const char* latin_file_name = tmp.data();
  int read = stoi(argv[2]);

  if (!read) {
    trie A;
    
    cerr << "before consume" << std::endl;
    A.consumeInflexions(file_name, latin_file_name);
    std::cerr << "after consume" << std::endl;
    
    std::cerr << A.getSize() << std::endl;
    A.saveExternal("dictionary.bin");
    std::cerr << "after save" << std::endl;
  } else {
    trie A("dictionary.bin");
    int dummy;
    string find_me(argv[3]);
    cerr << A.formWord(A.search(find_me, dummy)) << endl;
  }
  return 0;
}
