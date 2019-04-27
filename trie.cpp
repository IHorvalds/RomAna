// TODO: dictionary vs non-dictionary words (insert any non-dictionary word you are fed, keep track of all of them


// MAI IMPORTANT:
// "vector" de sons ca int32_t
// cratime?
// flexiuni separate prin virgula + rescris consumeInflections
// derivat de derivat de  deritvat

#include <fstream>
#include <string>
#include <cctype>
#include <iostream>
#include <algorithm>
#include <cassert>
#include <cstdlib>

using namespace std;

#define GENMODE 1
#ifdef GENMODE
  #define SIZE 2323437
  #define SIGMA 32
#endif

#define ROOT 0
#define NOTFOUND -1

int isDiacritic(char c) {
  return (c > -62 && c < -55);
}

// returns the alphabetic order of c.
int32_t encode(char c) {
  int val = tolower(c) - 'a';
  return ((0 <= val) && (val < 26)) ? val : -1;
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
  return encode(byte1);
}

typedef struct trieNode {
  uint32_t code;
  int32_t configuration;
  uint32_t* sons;
} trieNode;

class trie {
  trieNode* staticTrie;
  int32_t size;
  uint32_t sigma;
  uint32_t bufferPos;
  uint32_t lastOrigin;
  bool mode; // if trie is implemented with configuration, mode == true.
  int32_t leafCheck(int32_t ptr);
  void updateFreq(int32_t ptr);

public:
  trie();
  trie(const char* filename);
  ~trie();
  void insert(int32_t ptr, string str, int32_t connect, uint32_t pos);
  void addRoot(string str);
  void addDerivated(string derivated);
  int search(string str, int32_t ptr, uint32_t pos);
  string formWord(int32_t ptr);
  int32_t findParent(int32_t ptr, int32_t& encoding);
  void consumeInflexions(const char* filename);
  void loadExternal(const char* filename);
  void saveExternal(const char* filename);
  uint32_t getSize();
};

#ifdef GENMODE
// set mode on 1, if we build the trie (we don't use configuration).
trie::trie() {
  size = SIZE + 1;
  sigma = SIGMA;
  bufferPos = 0;
  lastOrigin = 0;
  mode = false;
  staticTrie = new trieNode[size];
  for (int i = 0; i < size; i++) {
    staticTrie[i].code = 0;
    staticTrie[i].sons = new uint32_t[sigma]();
    staticTrie[i].configuration = 0;
  }
}
#endif

// if we read from a binary file, mode must be set.
trie::trie(const char* filename) {
  loadExternal(filename);
  mode = true;
}

trie::~trie() {
  for (int i = 0; i < size; i++) {
    delete[] staticTrie[i].sons;
  }
  delete[] staticTrie;
}

void trie::updateFreq(int32_t ptr) {
  // derivat al unui derivat al unui derivat ... al unui root
  // down the rabbit hole
  if (!(staticTrie[ptr].code & 1))
    updateFreq(staticTrie[ptr].code >> 1);
  else
    staticTrie[ptr].code += 2;
}

// set in change the bit 'pos' on 1.
// TODO: save also the powers of 2 into a static vector.
void setBit(int32_t& change, unsigned pos) {
  change |= (1 << pos);
}

bool getBit(int32_t val, unsigned pos) {
  return (val >> pos) & 1; 
}

uint32_t countOfBits(int32_t val) {
  unsigned count;
  for (count = 0; val; val &= (val - 1))
    count++;
  return count;
} 

// returns how many set bits are before the bit pos.
// creates a mask: 00000111111, where the count of 1s equals pos.
uint32_t orderOfBit(int32_t val, uint32_t pos) {
  return countOfBits(val & ((1 << pos) - 1));
}

void error(const char *msg) {
  cerr << msg;
  exit(0);
}

// insert in trie str, the position the be regarded is now pos. connect is the node which str should be connected with.
// If str itself is a rootWord, connect must be set on -1.
void trie::insert(int32_t ptr, string str, int32_t connect, uint32_t pos) {
  if (str.size() <= pos) {
    // to keep track of which words are root words, and which words are derived from root words, the "code" variable of
    // each trie node will use its 0th bit set to 1 if the word is a root word, and to 0 otherwise.
    // if the 0th bit is set to 1, then all other bits keep the frequency of the word in the input text
    // if the 0th bit is set to 0, then all other bits keep the index of its root word in the trie array
    // Note:
    // + 2 in decimal <=> + 10 in binary, ie. add 1 to all bits but the 0th
    // * 2 in decimal <> << 1 in binary, ie. make sure 0th bit stays 0
    uint32_t hideme = (connect == -1);

#ifdef GENMODE
    if (hideme && !(staticTrie[ptr].code & 1) && staticTrie[ptr].code != 0) {
      // daca intai a fost trimis ca derivat, iar acum imi cere sa-l fac root
      // ce fac? il las derivat, n-a zis sudo. adica nu mai modific nimic
      return;
    }
    if (!hideme && (staticTrie[ptr].code & 1)) {
      // daca intai a fost trimis ca root, si acum vrea derivat
      // ce fac? e bou, il las root, cum sa cedezi puterea? adica nu mai modific nimic
      return;
    }
    if (!hideme && staticTrie[ptr].code && (uint32_t) connect != (staticTrie[ptr].code >> 1)) {
      // daca deja e derivatul cuiva, si acum vrea sa devina derivatul altcuiva
      // stai cuminte la casa ta, ca nu e dupa tine aici. adica nu mai modific nimic
      return;
    }
    staticTrie[ptr].code = (hideme) ? 1 : connect * 2;
#else
    updateFreq(ptr);
#endif
    // wtf was that, right?
    // I use GENMODE to tell the program whether I want to generate a new dictionary, or I want to update an existing
    // one. If I want to generate a new dictionary, ie. use the inflexions file from Domi, the frequency of every word I
    // insert in the dictionary *has to be 0*. Regardless of how many times I read it from the inflexions file. So if
    // GENMODE is defined, I simply no longer increment each word's frequency.
    // If, however, I want to update an existing dictionary, I'll need updated frequencies.
    return;
  }
  int32_t encoding = (pos == str.size() - 1) ? encode(str[pos]) : diacriticEncode(str[pos], str[pos + 1]);
  pos += (encoding >= 26); // if encoding shows we reached a diacritic character, move to next byte
  // if the next edge does not yet exist, create it.
  // Also, if trie is read from a binary file, we should verify here if the edge is saved.
  if (mode) {
    if (!getBit(staticTrie[ptr].configuration, encoding))
      error("Word doesn't exist in our dictionary!");
  } else if (!staticTrie[ptr].sons[encoding]) {
    staticTrie[ptr].sons[encoding] = ++bufferPos;
  }
  insert(staticTrie[ptr].sons[mode ? orderOfBit(staticTrie[ptr].configuration, encoding) : encoding], str, connect, pos + 1);
}

void trie::addRoot(string str) {
  insert(ROOT, str, -1, 0);
  // save the position of the position where the origin was put (provided derivates are added after the origin).
  lastOrigin = bufferPos;
}

void trie::addDerivated(string derivated) {
  insert(ROOT, derivated, lastOrigin, 0);
}

// Good for tests: if str is a rootWord returns the pointer where it was saved. Otherwise, the pointer of its rootword.
int32_t trie::search(string str, int32_t ptr, uint32_t pos) {
  if (str.size() <= pos) {
    return (staticTrie[ptr].code & 1) ? ptr : staticTrie[ptr].code / 2;
  }
  int32_t encoding = (pos == str.size() - 1) ? encode(str[pos]) : diacriticEncode(str[pos], str[pos + 1]);
  if (!staticTrie[ptr].sons[encoding])
    return NOTFOUND;
  return search(str, staticTrie[ptr].sons[mode ? orderOfBit(staticTrie[ptr].configuration, encoding) : encoding], pos + 1 + isDiacritic(str[pos]));
}

void cleanWordOfChar(string& word, char c) {
  word.erase(remove(word.begin(), word.end(), c), word.end());
}

void cleanWord(string& word) {
  for (auto c = word.begin(); c != word.end(); c++) {
    if (isDiacritic(*c)) {
      c++;
    }
    else if (encode(*c) == -1) {
      cleanWordOfChar(word, *c);
      cleanWord(word);
      return;
    }
  }
}

#ifdef GENMODE
void trie::consumeInflexions(const char* filename) {
  assert(!mode);
  ifstream in(filename);
  int32_t total, infl;
  string word, inflexion;
  string aux;
  in >> total;
  for (int i = 0; i < total; i++) {
   // cerr << i << endl;
    in >> word >> aux;
    if (!isdigit(aux[0])) {
      getline(in, word);
      continue;
    }
    cleanWord(word);
    //cerr << word << endl;
    addRoot(word);
    infl = stoi(aux);
    for (int j = 0; j < infl; j++) {
      in >> inflexion;
      cleanWord(inflexion);
      addDerivated(inflexion);
    }
  }
}
#endif

// load class members from external file
void trie::loadExternal(const char* filename) {
  ifstream in(filename);
  in.read((char*) &bufferPos, sizeof(int32_t));
  in.read((char*) &sigma, sizeof(int32_t));
  staticTrie = new trieNode[bufferPos + 1];
  for (unsigned i = 0; i <= bufferPos; i++) {
    in.read((char*) &staticTrie[i].code, sizeof(int32_t));
    in.read((char*) &staticTrie[i].configuration, sizeof(int32_t));
    staticTrie[i].sons = new uint32_t[countOfBits(staticTrie[i].configuration)](); 
    if (!staticTrie[i].configuration)
      continue;
    in.seekg((uint32_t) in.tellg() - sizeof(int32_t));
    // TODO: use hier x & -x to get the smallest power of two in configuration :)
    in.read((char*) staticTrie[i].sons, countOfBits(staticTrie[i].configuration) * sizeof(uint32_t));
  }
  in.close();
}

// save class members to external file
void trie::saveExternal(const char* filename) {
  ofstream out(filename);
  out.write((char*) &bufferPos, sizeof(int32_t));
  out.write((char*) &sigma, sizeof(int32_t));
  for (unsigned i = 0; i <= bufferPos; i++) {
    out.write((char*) &staticTrie[i].code, sizeof(int32_t));
    for (unsigned j = 0; j < sigma; j++)
      if (staticTrie[i].sons[j])
        staticTrie[i].configuration |= (1 << j);
    out.write((char*) &staticTrie[i].configuration, sizeof(int32_t));
    if (staticTrie[i].configuration) {
      for (unsigned j = 0; j < sigma; j++)
        if (staticTrie[i].configuration & (1 << j))
          out.write((char*) &staticTrie[i].sons[j], sizeof(uint32_t));
    }
  }
  out.close();
}

uint32_t trie::getSize() {
  return bufferPos;
}

int32_t trie::findParent(int32_t ptr, int32_t& encoding) {
  if (ptr == 0)
    return -1;
  for (unsigned index = 0; index <= bufferPos; ++index) {
    for (unsigned alpha = 0; alpha < sigma; ++alpha) {
      if (staticTrie[index].sons[alpha] == ptr) {
        encoding = alpha;
        return index;
      }
    }
  }
  return -1;
}

string decode(int32_t encoding) {
  string ret;
  string diacritic;
  if (encoding < 26) {
    ret.push_back('a' + encoding);
  } else {
    switch(encoding) {
    case 26:
      diacritic = "ă";
      break;
    case 27:
      diacritic = "â";
      break;
    case 28:
      diacritic = "î";
      break;
    case 29:
      diacritic = "ș";
      break;
    case 30:
      diacritic = "ț";
      break;
    }
    ret = diacritic;
  }
  return ret;
}

string trie::formWord(int32_t ptr) {
  string ret;
  if (ptr <= 0) {
    ret.clear();
    return ret;
  }
  int32_t encoding;
  int32_t nextPtr = findParent(ptr, encoding);
  return formWord(nextPtr) + decode(encoding);
}

int main(int argv, char** argc) {
  if (argv != 3) {
    cout << "Usage: ./a.out [path_to_dictionary] [word]" << endl;
    return -1;
  }
  string find_my_parent = argc[2];
//  trie A(argc[1]);
  
  trie A;
  A.consumeInflexions("inflections.in");
//  A.saveExternal("dictionary.bin");
  
//  trie A("dictionary.bin");
  cout << A.formWord(A.search(find_my_parent, 0, 0)) << endl;
  return 0;
}

