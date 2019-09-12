// TO DO
// curățenie!!!
//      ori folosim GENMODE, ori folosim mode, ori nu folosim niciuna
//        (adică ~(GENMODE & mode) LOL, adică fix ~(ce avem acum), și am gândit-o super natural, super repede
//            e fascinant cum mintea noastră gândește corect logic fără să își pună explicit logic problemele)
// flexiuni separate prin virgula + rescris consumeInflections

#include <map>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cassert>
#include <cstdlib>
#include <set>

#include "parserOltean.cpp"
text dummy_text;

using namespace std;

#define GENMODE 0
//#ifdef GENMODE
  #define SIZE 2323437 // we got some problems with this number. Apparently, it should be lower.
  #define ROMANIAN_SIGMA 31
  #define ENGLISH_SIGMA 26
//#endif

#define ROOT 0
#define NOTFOUND -1

std::map<std::pair<int8_t, int8_t>, uint32_t> special_letters;

void add_special_letter(int8_t byte1, int8_t byte2, uint32_t code) {
  special_letters[std::make_pair(byte1, byte2)] = code;
}

void init_special_letters() {
  add_special_letter(0xc3, 0xa4, 'a' - 'a'); // ä (german)
  add_special_letter(0xc3, 0xa5, 'a' - 'a'); // å (scandinavian)
  add_special_letter(0xc3, 0xa0, 'a' - 'a'); // à (french)
  add_special_letter(0xc3, 0xa1, 'a' - 'a'); // á (french)
  
  add_special_letter(0xc3, 0xa7, 's' - 'a'); // ç (french)
  
  add_special_letter(0xc3, 0xaa, 'e' - 'a'); // ê (french)
  add_special_letter(0xc3, 0xab, 'e' - 'a'); // ë (albanian)
  add_special_letter(0xc3, 0xa9, 'e' - 'a'); // é (french)
  add_special_letter(0xc3, 0xa8, 'e' - 'a'); // è (french)

  add_special_letter(0xc3, 0xb6, 'o' - 'a'); // ö (german)
  
  add_special_letter(0xc3, 0xb1, 'n' - 'a'); // ñ (spanish)
  
  add_special_letter(0xc3, 0xbc, 'u' - 'a'); // ü (german)
}

int32_t get_special_letter(int8_t byte1, int8_t byte2) {
  if (special_letters.find(std::make_pair(byte1, byte2)) == special_letters.end())
    return -1;
  else
    return special_letters[std::make_pair(byte1, byte2)];
}

int isDiacritic(char c) {
  return (c > -62 && c < -55);
}

// If c is a letter, returns the alphabetic order of c. Otherwise, -1.
int32_t encode(char c) {
  int val = tolower(c) - 'a';
  if (0 <= val && val < ENGLISH_SIGMA)
    return val;
  else
    return -1;
}

// check two bytes from string to see if you got a diacritică
int32_t diacriticEncode(int8_t byte1, int8_t byte2) {
  int32_t c = byte1 + byte2;
  if (c == -186 || c == -185) // ă
    return 26;
  if (c == -187 || c == -155) // â
    return 27;
  if (c == -175 || c == -143) // î
    return 28;
  if (c == -160 || c == -159) // ș
    return 29;
  if (c == -158 || c == -157) // ț
    return 30;
  // no diacritică? ok, call function for regular letter
  // std::cerr << "error" << byte1 << " " << byte2 << std::endl;
#if 0
  int32_t ret = encode(byte1);
  if (ret != -1)
    return ret;
  else {
    ret = get_special_letter(byte1, byte2);
    if (ret == -1) {
      std::cerr << "error for " << (int32_t)byte1 << " " << (int32_t)byte2 << std::endl; 
      assert(0);
    } else {
      return ret;
    }
  }
#else
  return encode(byte1);
#endif
}

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
  int32_t size;
  int32_t auxsize;
  uint32_t sigma;
  uint32_t bufferPos;
  uint32_t lastOrigin;
  bool mode; // if trie is implemented with configuration (so only if it is loaded from a binary), mode == true. It should be changed with GENMODE.
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
  void consumeInflexions(const char* uncleaned_filename, const char* cleaned_filename);
  void loadExternal(const char* filename);
  void saveExternal(const char* filename);
  void compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b);
  void computeFreqs(string filename);
  void sortFreqs(int root, set<pair<int, string>>& heap);
  void showFreqs(string filename);
  uint32_t getSize();
};

#ifdef GENMODE
// set mode on false, if we build the trie (we don't use configuration).
trie::trie() {
  size = SIZE + 1;
  sigma = ROMANIAN_SIGMA;
  bufferPos = 0;
  lastOrigin = 0;
  mode = false;
  staticTrie = new trieNode[size];
  for (int i = 0; i < size; i++) {
    staticTrie[i].code = 0;
    staticTrie[i].parent = 0;
    staticTrie[i].sons = new uint32_t[sigma]();
    staticTrie[i].configuration = ~0;
  }
}
#endif

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
  free(auxTrie);
}

trieNode* trie::staticTrieAccess(int32_t ptr) {
  if (ptr < size)
    return staticTrie + ptr;
  int index = ptr % size;
  int oldsize = auxsize - 1;
  if (index > oldsize) {
    auxsize *= 512;
    auxTrie = (trieNode*) realloc(auxTrie, auxsize * sizeof(trieNode) - 1);
    for (int i = oldsize; i < auxsize - 1; i++) {
      auxTrie[i].code = 0;
      auxTrie[i].parent = 0;
      auxTrie[i].sons = new uint32_t[sigma]();
      auxTrie[i].configuration = ~0;
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

//--------------------------------------------------------
// Operations on bits

// set in change the bit 'pos' on 1.
// TODO: save also the powers of 2 into a static vector.
void setBit(int32_t& change, unsigned pos) {
  change |= (1 << pos);
}

// returns the value of bit pos.
bool getBit(int32_t val, unsigned pos) {
  return (val >> pos) & 1;
}

// Count the number of bits of val.
uint32_t countOfBits(int32_t val) {
  unsigned count;
  for (count = 0; val; val &= (val - 1))
    count++;
  return count;
}

// A help-function to see the configuration (should be erased).
void printBits(int val, int pos) {
  cerr << "An welcher Stelle steht " << pos << "?\n";
  for (int i = 31; i >= 0; i--)
    cerr << ((val >> i) & 1);
  cerr << "ret = " << countOfBits(val & ((1 << pos) - 1)) << endl;
}

// returns how many set bits are before the bit pos.
// creates a mask: 00000111111, where the count of 1s equals pos.
uint32_t orderOfBit(int32_t val, uint32_t pos) {
  // assures that the bit pos is set in val, otherwise this wouldn't work.
  assert(getBit(val, pos));
  return countOfBits(val & ((1 << pos) - 1));
}
//----------------------------------------------------

// print any error message you wish.
void error(const char *msg) {
  cerr << msg;
  exit(0);
}

void trie::updateMihailsJmenuri(int32_t ptr, uint32_t pos, int32_t goesTo) {
  int size = countOfBits(staticTrieAccess(ptr)->configuration);
  uint32_t* temp = new uint32_t[size]();
  for (int i = 0; i < size; i++) {
    temp[i] = staticTrieAccess(ptr)->sons[i];
  }
  delete[] staticTrieAccess(ptr)->sons;
  staticTrieAccess(ptr)->sons = new uint32_t[sigma]();
  for (int i = 0; i < sigma; i++) {
    if (getBit(staticTrieAccess(ptr)->configuration, i))
      staticTrieAccess(ptr)->sons[i] = temp[orderOfBit(staticTrieAccess(ptr)->configuration, i)];
    else
      staticTrieAccess(ptr)->sons[i] = 0;
  }
  staticTrieAccess(ptr)->configuration = ~0;
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
    // I use GENMODE to tell the program whether I want to generate a new dictionary, or I want to update an existing
    // one. If I want to generate a new dictionary, ie. use the inflexions file from Domi, the frequency of every word I
    // insert in the dictionary *has to be 0*. Regardless of how many times I read it from the inflexions file. So if
    // GENMODE is defined, I simply no longer increment each word's frequency.
    // If, however, I want to update an existing dictionary, I'll need updated frequencies.
    return;
  }
  int32_t encoding = (pos == str.size() - 1) ? encode(str[pos]) : diacriticEncode(str[pos], str[pos + 1]);
  // if the next edge does not yet exist, create it.
  if ((mode && !getBit(staticTrieAccess(ptr)->configuration, encoding)) || !staticTrieAccess(ptr)->sons[encoding]) {
    if (staticTrieAccess(ptr)->configuration != ~0)
      updateMihailsJmenuri(ptr, encoding, ++bufferPos);
    else {
      staticTrieAccess(ptr)->sons[encoding] = ++bufferPos;
    }
    staticTrieAccess(bufferPos)->parent = encoding | (ptr << 7);
//    cout << bufferPos << ' ' << formWord(bufferPos) << endl;
  }
  // if encoding shows we reached a diacritic character, move to next byte
  insert(staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != ~0 ? orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : encoding], str, connect, pos + 1 + isDiacritic(str[pos]), finalPtr);
}

// add a root-word. Save the position in buffer where its last character has been saved.
void trie::addRoot(string str) {
  int dummy;
  insert(ROOT, str, -1, 0, dummy);
  // save the position of the position where the origin was put (provided derivates are added after the origin).
  lastOrigin = bufferPos;
}

// add a derivate. Use lastOrigin to connect the inflexion to its root-word.
void trie::addDerivated(string root, string derivated) {
  int dummy;
  insert(ROOT, derivated, search(root, dummy), 0, dummy);
}

// Good for tests: if str is a rootWord returns the pointer where it was saved. Otherwise, the pointer of its rootword.
int32_t trie::search(string str, int& lastPos) {
  int32_t ptr, pos, encoding;
  bool toCheck;
  ptr = 0;
  for (pos = 0; pos < str.size();) {
    encoding = (pos == str.size() - 1) ? encode(str[pos]) : diacriticEncode(str[pos], str[pos + 1]);
    toCheck = staticTrieAccess(ptr)->configuration != ~0 ? getBit(staticTrieAccess(ptr)->configuration, encoding) : staticTrieAccess(ptr)->sons[encoding];
    if (!toCheck) {
      lastPos = pos;
      return -ptr;
    }
    ptr = staticTrieAccess(ptr)->sons[staticTrieAccess(ptr)->configuration != ~0 ? orderOfBit(staticTrieAccess(ptr)->configuration, encoding) : encoding];
    pos += 1 + isDiacritic(str[pos]);
  }
  return (staticTrieAccess(ptr)->code & 1) ? ptr : staticTrieAccess(ptr)->code / 2;
}

// erase c from word.
void getRidOfChar(string& word, char c) {
  word.erase(remove(word.begin(), word.end(), c), word.end());
}

// clean word of unusable characters.
void getRidOfUnletters(string& word) {
#if 0
  for (auto c = word.begin(); c != word.end(); c++) {
    if (isDiacritic(*c)) {
      c++;
    } else if (encode(*c) == -1) {
      getRidOfChar(word, *c);
      getRidOfUnletters(word);
      return;
    }
  }
#else
  dummy_text.cleanUpWord(word);
#endif
}

#if 0
// Sorry, my GGC doesn't have neither stoi nor iterators ;)
int32_t myStoi(string& str) {
  int ret = 0;
  unsigned size = str.size();
  for (unsigned i = 0; i < size; i++)
    ret = (ret << 3) + (ret << 1) + str[i] - '0';
  return ret;
}
#endif

bool isRomanian(string str, int pos = 0) {
  if (pos == str.size())
    return true;
  if (pos == str.size() - 1)
    return encode(str.back()) != -1;
  else {
    int ret = diacriticEncode(str[pos], str[pos + 1]);
    if (ret == -1) {
      std::cerr << str << std::endl;
      return false;
    }
    return isRomanian(str, pos + 1 + !((0 <= ret) && (ret < ENGLISH_SIGMA)));  
  }
}

#ifdef GENMODE
void trie::consumeInflexions(const char* uncleaned_filename, const char* cleaned_filename) {
  assert(!mode);
  ifstream cleaned_in(cleaned_filename);
  ifstream uncleaned_in(uncleaned_filename);
  
  int32_t cleaned_total, cleaned_infl, uncleaned_total, uncleaned_infl;
  string cleaned_word, cleaned_inflexion, uncleaned_word, uncleaned_inflexion;
  string cleaned_aux, uncleaned_aux;
  
  uncleaned_in >> uncleaned_total;
  cleaned_in >> cleaned_total;
  
  assert(cleaned_total == uncleaned_total);
  for (int i = 0; i < uncleaned_total; i++) {
    uncleaned_in >> uncleaned_word >> uncleaned_aux;
    cleaned_in >> cleaned_word >> cleaned_aux;
    
    std::cerr << i << std::endl;
    std::cerr << uncleaned_word << std::endl;
    
    if (!isdigit(uncleaned_aux[0])) {
      std::cerr << "serios?" << std::endl;
      getline(uncleaned_in, uncleaned_word);
      continue;
    }
    
    getRidOfUnletters(uncleaned_word);
    getRidOfUnletters(cleaned_word);
    
    
    std::cerr << i << std::endl;
    std::cerr << uncleaned_word << std::endl;
    
    // Check if the word is indeed a romanian one.
    bool root_is_romanian = true;
    if (!isRomanian(uncleaned_word)) {
      std::cerr << uncleaned_word << "-----------------------------------" << std::endl;
      addRoot(cleaned_word);
      root_is_romanian = false;
      std::cerr << cleaned_word;
    } else {
      addRoot(uncleaned_word);
    }
    
    // std::cerr << uncleaned_word << std::endl;
    
    //std::cerr << "before infle" << std::endl;
    
    // addRoot(uncleaned_word);
    uncleaned_infl = stoi(uncleaned_aux);
    for (int j = 0; j < uncleaned_infl; j++) {
      //std::cerr << j << std::endl;
      uncleaned_in >> uncleaned_inflexion;
      cleaned_in >> cleaned_inflexion;
      
      getRidOfUnletters(uncleaned_inflexion);
      getRidOfUnletters(cleaned_inflexion);
      
      // Don't add duplicates.
      if (uncleaned_inflexion != uncleaned_word) {
        if (!isRomanian(uncleaned_inflexion)) {
          addDerivated(cleaned_word, cleaned_inflexion);
        } else if (root_is_romanian) {
          addDerivated(uncleaned_word, uncleaned_inflexion);
        } else {
          addDerivated(cleaned_word, uncleaned_inflexion);
        }
      }
    }
  }
  cleaned_in.close();
  uncleaned_in.close();
}
#endif

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
    unsigned howMany = countOfBits(staticTrie[i].configuration);
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
    unsigned howMany = countOfBits(staticTrieAccess(i)->configuration);
    int dummy;
//    if (staticTrieAccess(findParent(i, dummy))->configuration == ~0)
    if (staticTrieAccess(i)->configuration == ~0) {
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

// decode into string the hash encoding.
string decode(int32_t encoding) {
  string ret;
  if (encoding < ENGLISH_SIGMA) {
    ret = 'a' + encoding;
  } else {
    // particular cases.
    switch(encoding) {
    case 26:
      ret = "ă";
      break;
    case 27:
      ret = "â";
      break;
    case 28:
      ret = "î";
      break;
    case 29:
      ret = "ș";
      break;
    case 30:
      ret = "ț";
      break;
    }
  }
  return ret;
}

// create the word that ends in ptr.
string trie::formWord(int32_t ptr) {
  string ret;
  //cout << ptr << endl;
  if (ptr <= 0)
    return ret;
  int32_t encoding;
  int32_t nextPtr = findParent(ptr, encoding);
  return formWord(nextPtr) + decode(encoding);
}

void trie::compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b) {
  if (countOfBits(staticTrieAccess(root)->configuration) == 1) {
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

void trie::sortFreqs(int root, set<pair<int, string>>& heap) {
  if ((staticTrieAccess(root)->code & 1) && (staticTrieAccess(root)->code >> 1)) {
    heap.insert(make_pair(staticTrieAccess(root)->code >> 1, formWord(root)));
  }
  for (auto i = 0; i < countOfBits(staticTrieAccess(root)->configuration); i++) {
    if (staticTrieAccess(root)->sons[i] != 0)
      sortFreqs(staticTrieAccess(root)->sons[i], heap);
  }
}

void trie::showFreqs(string filename) {
    ofstream out(filename);
    set<pair<int, string>> sorted;
    sortFreqs(0, sorted);
    auto i = sorted.end();
    i--;
    for (; i != sorted.begin(); i--) {
      out << (i->second) << ' ' << i->first << endl;
    }
}

int main(void) {
  // read represents, whether we want to build the trie or not (GENMODE should be instead).
    
  init_special_letters();  
  
  #define READ 1
  if (READ) {
    trie A;
    cerr << "before" << std::endl;
    A.consumeInflexions("uncleaned_inflections.in", "cleaned_inflections.in");
    std::cerr << "after" << std::endl;
    A.saveExternal("dictionary.bin");
    int dummy;
    cout << A.formWord(A.search("soluționatelor", dummy)) << endl;
  } else {
    trie A("dictionary.bin");
    int dummy;
    cerr << A.formWord(A.search("soluționatelor", dummy)) << endl;
  }
  return 0;
}
