// TODO: dictionary vs non-dictionary words (insert any non-dictionary word you are fed, keep track of all of them
//       homonymes array? (is it needed?)

#include <fstream>
#include <string>
#include <cctype>
#include <iostream>

using namespace std;

#define SIZE 100
#define SIGMA 32
#define ROOT 0
#define NOTFOUND -1

int isDiacritic(char c) {
  return (c > -62 && c < -55);
}

// returns the alphabetic order of c.
int32_t encode(char c) {
  return tolower(c) - 'a';
}

// check two bytes from string to see if you got a diacritică
int32_t diacriticEncode(int8_t byte1, int8_t byte2) {
  int32_t c = byte1 + byte2;
  if (c == -186 || c == -185) // ă
    return 27;
  if (c == -187 || c == -155) // â
    return 28;
  if (c == -175 || c == -143) // î
    return 29;
  if (c == -160 || c == -159) // ș
    return 30;
  if (c == -158 || c == -157) // ț
    return 31;
  // no diacritică? ok, call function for regular letter
  return encode(byte1);
}

typedef struct trieNode {
  uint32_t code;
  uint32_t* sons;
} trieNode;

class trie {
  trieNode* staticTrie;
  int32_t size;
  int32_t sigma;
  uint32_t bufferPos;
  uint32_t lastOrigin;
  
public:
  trie();
  trie(const char* filename);
  ~trie();
  void insert(int32_t ptr, string str, int32_t connect, uint32_t pos);
  void addRoot(string str);
  void addDerivated(string derivated);
  int search(int32_t ptr, string str, uint32_t pos);
  void saveExternal(const char* filename);
  void loadExternal(const char* filename);
};

trie::trie() {
  size = SIZE + 1;
  sigma = SIGMA;
  bufferPos = 0;
  lastOrigin = 0;
  staticTrie = new trieNode[size];
  for (int i = 0; i < size; i++) {
    staticTrie[i].code = 0;
    staticTrie[i].sons = new uint32_t[sigma];
  }
}

trie::trie(const char* filename) {
  loadExternal(filename);
}

trie::~trie() {
  for (int i = 0; i < size; i++) {
    delete[] staticTrie[i].sons;
  }
  delete[] staticTrie;
}

// insert in trie str, the position the be regarded is now pos. connect is the node which str should be connected with.
// If str itself is a rootWord, connect must be set on -1.
void trie::insert(int32_t ptr, string str, int32_t connect, uint32_t pos) {
  if (str.size() == pos) {
    // to keep track of which words are root words, and which words are derived from root words, the "code" variable of
    // each trie node will use its 0th bit set to 1 if the word is a root word, and to 0 otherwise.
    // if the 0th bit is set to 1, then all other bits keep the frequency of the word in the input text
    // if the 0th bit is set to 0, then all other bits keep the index of its root word in the trie array
    // Note:
    // + 2 in decimal <=> + 10 in binary, ie. add 1 to all bits but the 0th
    // * 2 in decimal <> << 1 in binary, ie. make sure 0th bit stays 0
    uint32_t hideme = (connect == -1);
    staticTrie[ptr].code = (hideme) ? ((staticTrie[ptr].code | hideme) + 2) | hideme : connect * 2;
    return;
  }
  int32_t encoding = diacriticEncode(str[pos], str[pos + 1]);
  pos = (encoding > 26) ? pos + 1 : pos; // if encoding shows we reached a diacritic character, move to next byte
  // if the next edge does not yet exist, create it.
  if (!staticTrie[ptr].sons[encoding]) {
    staticTrie[ptr].sons[encoding] = ++bufferPos;
  }
  insert(staticTrie[ptr].sons[encoding], str, connect, pos + 1);
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
int trie::search(int32_t ptr, string str, uint32_t pos) {
  if (str.size() == pos) {
    return (staticTrie[ptr].code & 1) ? staticTrie[ptr].code / 2 : staticTrie[staticTrie[ptr].code / 2].code / 2;
  }
  if (!staticTrie[ptr].sons[diacriticEncode(str[pos], str[pos + 1])])
    return NOTFOUND;
  return search(staticTrie[ptr].sons[diacriticEncode(str[pos], str[pos + 1])], str, pos + 1 + isDiacritic(str[pos]));
}

// save class members to external file
void trie::saveExternal(const char* filename) {
  ofstream out(filename);
  out.write((char*) &size, sizeof(int32_t));
  out.write((char*) &sigma, sizeof(int32_t));
  for (int i = 0; i < size; i++) {
    out.write((char*) &staticTrie[i].code, sizeof(int32_t));
    out.write((char*) staticTrie[i].sons, sigma * sizeof(int32_t));
  }
  out.write((char*) &bufferPos, sizeof(int32_t));
  out.write((char*) &lastOrigin, sizeof(int32_t));
  out.close();
}

// load class members from external file
void trie::loadExternal(const char* filename) {
  ifstream in(filename);
  in.read((char*) &size, sizeof(int32_t));
  in.read((char*) &sigma, sizeof(int32_t));
  staticTrie = new trieNode[size];
  for (int i = 0; i < size; i++) {
    staticTrie[i].sons = new uint32_t[sigma];
    in.read((char*) &staticTrie[i].code, sizeof(int32_t));
    in.read((char*) staticTrie[i].sons, sigma * sizeof(int32_t));
  }
  in.read((char*) &bufferPos, sizeof(int32_t));
  in.read((char*) &lastOrigin, sizeof(int32_t));
  in.close();
}


int main(void) {
  trie T("dictionary.bin");  
  cout << T.search(ROOT, "mănânc", 0) << endl;
  cout << T.search(ROOT, "luat", 0) << endl;
  return 0;
}
