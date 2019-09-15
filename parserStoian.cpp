// A gift for Alex (by the way, gift in german is poison, but I know what I mean by that).
// Run this code with the text of 'Luceafarul'. It should work.
// TODO: for example, 'n-o' should be mapped to 'nu' and 'o'.
// We could achieve that by storing a simple vector or hash table or trie with the word that it corresponds to.
// What we also need: think if for a shorter form there exists more than one possibility.
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <map>

#define MAX_LEN 10000     // change this for bigger texts
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
const unsigned MAX_SPECIFICS = 9;
const unsigned MAX_VERY_SPECIFICS = 2;
const char specifics[MAX_SPECIFICS] = {'-', '\'', '?', '!', ':', ',', '.', ';', '"'};
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
      //cerr << "wir sind bei ..." << c << "..." << endl;
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

//---------------------------------------
// Code for when the smaller words derivate from bigger, but some could have more meanings.
#define BEFORE 0
#define AFTER 1

// stores the two possibilities for a smaller word ->
// the one that could appear before a specific (before) and the one after a specific (after).
struct pairWord {
  string before, after;

  pairWord() {
  }

  pairWord(string singleWord) {
    this -> before = singleWord;
  }

  pairWord(string before, string after) {
    this -> before = before;
    if (!after.empty())
      this -> after = after;
  }
};

// Initialize the map.
map<string, pairWord> completedWords;

void complete(string incompleteWord, string beforeSpecific, string afterSpecific) {
  pairWord addNew = pairWord(beforeSpecific, afterSpecific);
  completedWords[incompleteWord] = addNew;
}

// Put the smaller words together with the respective bigger ones.
// Don't forget to also place those which begin with capitel letter.
// Words like 'aib' (from aiba) don't count because they are already stored in trie.
// The more special are those which are composed only out of one letter.
void initCompletedWords() {
  complete("n", "nu", "în");
  complete("N", "Nu", "");
  complete("Ș", "Și", "");
  complete("ș", "și", "");
  complete("l", "la", "îl");
  complete("i", "", "fi"); // mi-i alunga, iarna-i aici -> hm, we've got a problem, but it could be solved if we know what type of functionality the words has (substantiv)
}

// Determinates, depending on the mode, what bigger word we should take.
string shouldBeCompleted(string word, int mode) {
  map<string, pairWord>::iterator it;
  string ret;
  for (it = completedWords.begin(); it != completedWords.end(); ++it) {
    if (word == it -> first) {
      ret = (mode == BEFORE) ? it -> second.before : it -> second.after;
      goto returnString;
    }
  }
  returnString : {
    return ret;
  }
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

    if (word[index] == ' ') {
      if (wordBuilder.size())
        wordsList.push_back(wordBuilder);
      wordBuilder.clear();
      ++index;
      goto checkForSpecific;
    }

    // Separates the smaller words. If the specific characters are the end, nothing extra should be done.
    char c = word[index];
    if (isSpecific(c)) {
      if (!wordBuilder.empty()) {
        // We know that the word is before the specific, so search for a "before-word".
        if (shouldBeCompleted(wordBuilder, BEFORE).empty()) {
          wordsList.push_back(wordBuilder);
        } else {
          wordsList.push_back(shouldBeCompleted(wordBuilder, BEFORE));
        }
      }
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
          if (!wordBuilder.empty()) {
            wordsList.push_back(wordBuilder);
          }

          // Jump over those very special characters that I can't find in UTF-8 :)
          wordBuilder.clear();
          do {
            ++index;
          } while (word[index] & 0x80);
          goto checkForSpecific;
          //fprintf(stderr, "%s\n", word.data());
          cerr << "We've got some problems " << word << endl;
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
    // At the end, look upon the smaller words which appear in completedWords (mode should be put on after).
    vector<string>::iterator it;
    for (it = wordsList.begin(); it != wordsList.end(); ++it) {
      if (!shouldBeCompleted(*it, AFTER).empty()) {
        *it = shouldBeCompleted(*it, AFTER);
      }
    }
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

  // initialise the incompleted words that are separated by specific characters.
  initCompletedWords();

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
