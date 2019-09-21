#include <iostream>
#include <fstream>
#include <string>

#include "special_characters.hpp"

using namespace std;

class text {
  static constexpr unsigned BUFF_SIZE = 1024;  
  ifstream input;
  char buffer[BUFF_SIZE];
  int fileSize;
  int currSize;
  int index;

  static constexpr unsigned COUNT_UNALLOWED = 5; 
  static constexpr char unallowed[COUNT_UNALLOWED] = {' ', ',', '\n', '-', '/'};
  
public:
  text() {
  }
    
  static bool isUnallowed(char c) {
    for (unsigned index = 0; index < COUNT_UNALLOWED; ++index) {
      if (unallowed[index] == c)
        return true;
    }
    return false;
  }
    
  static bool isSeparator(char c) {
    return isUnallowed(c) || ispunct(c);
  }
    
  // Better if we received a file with already preprocessed words in latin
  text(string filename) : index(0), currSize(0) {
    input.open(filename, ios::in | ios::binary);
    input.seekg(0, input.end);
    fileSize = input.tellg();
    input.seekg(0, input.beg);
  }
  
  string serveWord() {
    // It's not possible that we receive a white-space
    if ((currSize == fileSize) && (index + 1 >= input.gcount())) {
      return " <EOF> ";
    }
    if (index == input.gcount()) {
      input.read(buffer, BUFF_SIZE);
      index = 0;
      currSize += input.gcount();
    }
    string word;
    while ((index < BUFF_SIZE) && !isSeparator(buffer[index])) {
      word.append(1, buffer[index]);
      index++;
    }
    if ((index == BUFF_SIZE) && (currSize != fileSize)) {
      input.seekg(-word.length(), input.cur);
      fileSize += word.length();
      return serveWord();
    }
    while ((index < BUFF_SIZE) && isSeparator(buffer[index])) {
      index++;
    }
    specialChars::cleanUpWord(word);
    if (word.empty()) {
      word = serveWord();
    }
    return word;
  }
};
