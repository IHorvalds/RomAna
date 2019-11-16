#include <iostream>
#include <fstream>
#include <string>

#include "special_characters.hpp"

class text {
  static constexpr unsigned BUFF_SIZE = 1024;
  std::ifstream input;
  char buffer[BUFF_SIZE];
  unsigned LIMIT_BUFFER;
  int fileSize;
  int currSize;
  int index;

  static constexpr unsigned COUNT_UNALLOWED = 6; 
  static constexpr char unallowed[COUNT_UNALLOWED] = {' ', ',', '\n', '-', '/', '\t'};
  
public:
  text() {
  }
   
  void close() {
    input.close();
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
  text(std::string filename) : index(BUFF_SIZE), LIMIT_BUFFER(BUFF_SIZE), currSize(0) {
    input.open(filename, std::ios::in | std::ios::binary);
    input.seekg(0, input.end);
    fileSize = input.tellg();
    input.seekg(0, input.beg);
  }
  
  char getByte() {
    if (currSize == fileSize)
      return 0;
    if (index == LIMIT_BUFFER) {
      index = 0;
      input.read(buffer, BUFF_SIZE);
      LIMIT_BUFFER = input.gcount();
    }
    ++currSize;
    return buffer[index++];
  }
  
  std::string serveWord() {
#if 0
    // It's not possible that we receive a white-space
    if ((currSize == fileSize) && (index + 1 >= input.gcount())) {
      return " <EOF> ";
    }
    if (index == input.gcount()) {
      input.read(buffer, BUFF_SIZE);
      index = 0;
      currSize += input.gcount();
    }
    std::string word = "";
    while ((index < BUFF_SIZE) && !isSeparator(buffer[index])) {
      word.append(1, buffer[index]);
      index++;
    }
    if ((index == BUFF_SIZE) && (currSize != fileSize)) {
      input.seekg(-word.length(), input.cur);
      fileSize += word.length();
      currSize += word.length();
      return serveWord();
    }
    while ((index < BUFF_SIZE) && isSeparator(buffer[index])) {
      index++;
    }
    specialChars::cleanUpWord(word);
    if (word.empty())
      word = serveWord();
    return word;
  }
#else
    char c;
    do {
      c = getByte();
    } while ((c != 0) && isSeparator(c));
    
    if (!c)
      return " <EOF> ";
    
    std::string word = "";
    do {
      word.append(1, c);
      c = getByte();
    } while (!isSeparator(c));
    specialChars::cleanUpWord(word);
    return word;
  }
#endif
  
};
