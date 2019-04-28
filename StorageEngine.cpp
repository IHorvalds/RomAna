// TODO: dictionary vs non-dictionary words (insert any non-dictionary word you are fed, keep track of all of them

// MAI IMPORTANT:
// "vector" de sons ca int32_t
// cratime?
// flexiuni separate prin virgula + rescris consumeInflections
// derivat de derivat de  deritvat

#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <cctype>
#include <cassert>
#include <cstdlib>

using namespace std;

#define GENMODE 1
#ifdef GENMODE
  #define SIZE 2323437 // we got some problems with this number. Apparently, it should be lower.
  #define ROMANIAN_SIGMA 31
  #define ENGLISH_SIGMA 26
#endif

#define ROOT 0
#define NOTFOUND -1

int isDiacritic(char c) {
  return (c > -62 && c < -55);
}

// If c is a letter, returns the alphabetic order of c. Otherwise, -1.
int32_t encode(char c) {
  int val = tolower(c) - 'a';
  return (0 <= val && val < ENGLISH_SIGMA) ? val : -1;
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

#pragma pack(push, 1)
struct trieNode {
  uint32_t code;
  int32_t configuration;
  uint32_t* sons;
};
#pragma pack(pop)

class trie {
  trieNode* staticTrie;
  int32_t size;
  uint32_t sigma;
  uint32_t bufferPos;
  uint32_t lastOrigin;
  bool mode; // if trie is implemented with configuration (so only if it is loaded from a binary), mode == true. It should be changed with GENMODE.
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
    staticTrie[i].sons = new uint32_t[sigma]();
    staticTrie[i].configuration = 0;
  }
}
#endif

uint32_t trie::getSize() {
  return bufferPos;
}

// if we read from a binary file, mode must be set on true.
trie::trie(const char* filename) {
  mode = true;
  loadExternal(filename);
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

// insert in trie str, the position the be regarded is now pos. connect is the node which str should be connected with.
// If str itself is a rootWord, connect must be set on -1.
void trie::insert(int32_t ptr, string str, int32_t connect, uint32_t pos) {
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

#ifdef GENMODE
    // why the third condition? - question for Alex.
    if (hideme && !(staticTrie[ptr].code & 1) && (staticTrie[ptr].code != 0)) {
      // daca intai a fost trimis ca derivat, iar acum imi cere sa-l fac root
      // ce fac? il las derivat, n-a zis sudo. adica nu mai modific nimic
      return;
    }
    if (!hideme && (staticTrie[ptr].code & 1)) {
      // daca intai a fost trimis ca root, si acum vrea derivat
      // ce fac? e bou, il las root, cum sa cedezi puterea? adica nu mai modific nimic
      return;
    }
    if (!hideme && (staticTrie[ptr].code >> 1) && ((uint32_t)connect) != (staticTrie[ptr].code >> 1)) {
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
  // if the next edge does not yet exist, create it.
  if (mode) {
    // if trie is read from a binary file, we should verify here whether the edge is saved.
    if (!getBit(staticTrie[ptr].configuration, encoding))
      error("Word doesn't exist in our dictionary!");
  } else if (!staticTrie[ptr].sons[encoding]) {
    staticTrie[ptr].sons[encoding] = ++bufferPos;
  }
  // if encoding shows we reached a diacritic character, move to next byte
  insert(staticTrie[ptr].sons[mode ? orderOfBit(staticTrie[ptr].configuration, encoding) : encoding], str, connect, pos + 1 + isDiacritic(str[pos]));
}

// add a root-word. Save the position in buffer where its last character has been saved.
void trie::addRoot(string str) {
  insert(ROOT, str, -1, 0);
  // save the position of the position where the origin was put (provided derivates are added after the origin).
  lastOrigin = bufferPos;
}

// add a derivate. Use lastOrigin to connect the inflexion to its root-word.
void trie::addDerivated(string derivated) {
  insert(ROOT, derivated, lastOrigin, 0);
}

// Good for tests: if str is a rootWord returns the pointer where it was saved. Otherwise, the pointer of its rootword.
int32_t trie::search(string str, int32_t ptr, uint32_t pos) {
  if (str.size() == pos)
    return (staticTrie[ptr].code & 1) ? ptr : staticTrie[ptr].code / 2;

  int32_t encoding = (pos == str.size() - 1) ? encode(str[pos]) : diacriticEncode(str[pos], str[pos + 1]);

  // verify whether the edge exists. It depends on mode (or GENMODE).
  bool toCheck = mode ? getBit(staticTrie[ptr].configuration, encoding) : staticTrie[ptr].sons[encoding];
  if (!toCheck)
    return NOTFOUND;
  return search(str, staticTrie[ptr].sons[mode ? orderOfBit(staticTrie[ptr].configuration, encoding) : encoding], pos + 1 + isDiacritic(str[pos]));
}

// erase c from word.
void cleanWordOfChar(string& word, char c) {
  word.erase(remove(word.begin(), word.end(), c), word.end());
}

// clean word of unusable characters.
void cleanWord(string& word) {
  for (auto c = word.begin(); c != word.end(); c++) {
    if (isDiacritic(*c)) {
      c++;
    } else if (encode(*c) == -1) {
      cleanWordOfChar(word, *c);
      cleanWord(word);
      return;
    }
  }
}

// Sorry, my GGC doesn't have neither stoi nor iterators ;)
int32_t myStoi(string& str) {
  int ret = 0;
  unsigned size = str.size();
  for (unsigned i = 0; i < size; i++)
    ret = (ret << 3) + (ret << 1) + str[i] - '0';
  return ret;
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
    in >> word >> aux;
    if (!isdigit(aux[0])) {
      getline(in, word);
      continue;
    }
    cleanWord(word);
    addRoot(word);
    infl = myStoi(aux);
    for (int j = 0; j < infl; j++) {
      in >> inflexion;
      cleanWord(inflexion);
      // Don't add duplicates.
      if (inflexion != word)
        addDerivated(inflexion);
    }
  }
}
#endif

// load class members from external file
void trie::loadExternal(const char* filename) {
  ifstream in;
  in.open(filename, ios::in | ios::binary);

  in.read((char*) &size, sizeof(int32_t));
  in.read((char*) &sigma, sizeof(int32_t));

  staticTrie = new trieNode[size];
  for (unsigned i = 0; i < size; i++) {
    in.read((char*) &staticTrie[i].code, sizeof(uint32_t));
    in.read((char*) &staticTrie[i].configuration, sizeof(int32_t));

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
  size = bufferPos + 1;
  out.write((char*) &size, sizeof(int32_t));
  out.write((char*) &sigma, sizeof(int32_t));

  for (unsigned i = 0; i < size; i++) {
    // Computes configuration (the sons with non-zero values).
    staticTrie[i].configuration = 0;
    for (unsigned j = 0; j < sigma; j++)
      if (staticTrie[i].sons[j])
        staticTrie[i].configuration |= (1 << j);
    out.write((char*) &staticTrie[i].code, sizeof(int32_t));
    out.write((char*) &staticTrie[i].configuration, sizeof(int32_t));

    // Writes only the sons with non-zero values.
    for (unsigned j = 0; j < sigma; j++)
      if (staticTrie[i].sons[j])
        out.write((char*) &staticTrie[i].sons[j], sizeof(uint32_t));
  }
  out.close();
}

// returns the parent in trie of ptr and also saves the type of edge that lies between them.
// this function depends also on mode.
// TODO: update configuration during insert. Not necessary! because we only build a trie with sons at full size (SIGMA).
int32_t trie::findParent(int32_t ptr, int32_t& encoding) {
  if (ptr == 0)
    return -1;
  for (unsigned index = 0; index <= size; ++index) {
    for (unsigned alpha = 0; alpha < sigma; ++alpha) {
      if (mode) {
        // check if alpha is found in configuration and than return its order.
        if (getBit(staticTrie[index].configuration, alpha) && staticTrie[index].sons[orderOfBit(staticTrie[index].configuration, alpha)] == ptr) {
          encoding = alpha;
          return index;
        }
      } else if(staticTrie[index].sons[alpha] == ptr) {
        encoding = alpha;
        return index;
      }
    }
  }
  return -1;
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
  if (ptr <= 0)
    return ret;
  int32_t encoding;
  int32_t nextPtr = findParent(ptr, encoding);
  return formWord(nextPtr) + decode(encoding);
}

int main(void) {
  // read represents, whether we want to build the trie or not (GENMODE should be instead).
  #define READ 0
  if (READ) {
    trie A;
    A.consumeInflexions("inflections.in");
    A.saveExternal("dictionary.bin");
    cout << A.formWord(A.search("soluționatelor", 0, 0)) << endl;
  } else {
    trie A("dictionary.bin");
    cerr << A.formWord(A.search("soluționatelor", 0, 0)) << endl;
  }
  return 0;
}
