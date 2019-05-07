// A gift for Alex (by the way, gift in german is poison, but I know what I mean by that).
// Run this code with the text of 'Luceafarul'. It should work.
// TODO: for example, 'n-o' should be mapped to 'nu' and 'o'.
// We could achieve that by storing a simple vector or hash table or trie with the word that it corresponds to.
// What we also need: think if for a shorter form there exists more than one possibility.
// TODO: implementation issues -> the iterations over a vector are done with old iterators. Change them in the newest ones.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

#define MAX_LEN 1000     // change this for bigger texts
#define ENGLISH_SIGMA 26

using namespace std;

//---------------------------------------------
// Code taken from StorageEngine.cpp
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
//---------------------------------------------
void printWordsInFile(vector<string>& words, ofstream& file) {
  vector<string>::iterator it;
  for (it = words.begin(); it != words.end(); ++it)
    file << *it << "\n";
}

// prints messages.
void error(const char* msg) {
  cerr << msg << endl;
  exit(0);
}

// Add more specific characters that separate words in romanian.
const unsigned MAX_SPECIFICS = 8;
const unsigned MAX_VERY_SPECIFICS = 2;
const char specifics[MAX_SPECIFICS] = {'-', '\'', '?', '!', ':', ',', '.', ';'};
const int verySpecifics[MAX_VERY_SPECIFICS][2] = {{0xc2, 0xab}, {0xc2, 0xbb}};

// test for specific characters.
bool isSpecific(char c) {
  for (unsigned index = 0; index < MAX_SPECIFICS; ++index)
    if (specifics[index] == c)
      return true;
  return false;
}

// test for very specific characters (the ones which have the represantation on more than 1 byte).
bool isVerySpecific(int byte1, int byte2) {
  byte1 &= 0xff;
  byte2 &= 0xff;
  for (unsigned index = 0; index < MAX_VERY_SPECIFICS; ++index)
    if (verySpecifics[index][0] == byte1 && verySpecifics[index][1] == byte2)
      return true;
  return false;
}

// Separate the line in words (even though they couldn't exist in the romanian dictionary).
vector<string> getWords(char* line, uint32_t length) {
  vector<string> newWords;

  int currPos = -1;
  analyzeLine : {
    ++currPos;
    if (currPos == length)
      goto endOfSearch;
    // find the first position which doesn't have space.
    if (line[currPos] == ' ')
      goto analyzeLine;

    string buildWord;
    constructWord : {
      // We're at the end of the string.
      if (currPos == length) {
        newWords.push_back(buildWord);
        goto endOfSearch;
      }
      // Empty spaces separate words.
      if (line[currPos] == ' ') {
        newWords.push_back(buildWord);
        goto analyzeLine;
      }

      // Analyze the next character.
      char c = line[currPos];
      if (c & 0x80) {
        if (isDiacritic(c)) {
          buildWord.push_back(line[currPos++]);
          buildWord.push_back(line[currPos++]);
          goto constructWord;
        } else {
          // Takes the entire UTF-8 character.
          // TODO: it should be easier!!!
          while (line[currPos] & 0x80) {
            buildWord.push_back(line[currPos++]);
          }
          buildWord.push_back(line[currPos++]);
          goto constructWord;
        }
      } else {
        // Check if it's a letter.
        if (encode(c) == -1) {
          if (!isSpecific(c))
            goto analyzeLine;
        }
        buildWord.push_back(line[currPos++]);
        goto constructWord;
      }
      goto constructWord;
    }
  }
  endOfSearch : {
    // end here the function.
  }
  return newWords;
}

// analyze the word, returning the smaller words which probably it consists from.
vector<string> analyzeWord(string& word) {
  vector<string> wordsList;
  string wordBuilder;

  unsigned len = word.size();
  unsigned index = 0;
  checkForSpecific : {
    // We're at the end. Add the last wordBuilder.
    if (index == len) {
      if (wordBuilder.size())
        wordsList.push_back(wordBuilder);
      goto endOfAnalysis;
    }

    // Separates the smaller words. If the specific characters are the end, nothing extra should be done.
    char c = word[index];
    if (isSpecific(c)) {
      if (wordBuilder.size())
        wordsList.push_back(wordBuilder);
      wordBuilder.clear();
      ++index;
      goto checkForSpecific;
    }

    // Analyze the next character.
    if (c & 0x80) {
      if (isDiacritic(c)) {
        wordBuilder.push_back(word[index++]);
        wordBuilder.push_back(word[index++]);
        goto checkForSpecific;
      } else {
        // Check for a very specific character.
        if (index < len - 1 && isVerySpecific(word[index], word[index + 1])) {
          if (!wordBuilder.empty()) {
            wordsList.push_back(wordBuilder);
          }
          wordBuilder.clear();

          // Go further.
          index += 2;
          goto checkForSpecific;
        } else {
          cerr << "We've got some problems " << c << endl;
        }
      }
    } else {
      // A normal english letter.
      wordBuilder.push_back(word[index++]);
      goto checkForSpecific;
    }
    cerr << "what case is out?\n";
  }
  endOfAnalysis : {
    // end here the analysis.
  }
  return wordsList;
}

// Takes the initial words and filter them with the romanian style of writing.
vector<string> filterWords(vector<string>& words) {
  vector<string> filteredWords, tmp;
  vector<string>::iterator it;
  for (it = words.begin(); it != words.end(); ++it) {
    string currString = *it;
    tmp = analyzeWord(currString);
    if (tmp.size())
      filteredWords.insert(filteredWords.end(), tmp.begin(), tmp.end());
  }
  return filteredWords;
}

int main(void) {
  ifstream in("text.in");
  ofstream ok("output.ok"); // in this file you should have the words you need.

  char* line;
  line = new char[MAX_LEN + 2];

  // Read by line.
  vector<string> words;
  while (in.getline(line, MAX_LEN)) {
    vector<string> newWords = getWords(line, strlen(line));
    words.insert(words.end(), newWords.begin(), newWords.end());
  }
  words = filterWords(words);
  printWordsInFile(words, ok);
  return 0;
}
