#include <iostream>
#include <fstream>
#include <string>
#include "specialcharacters.cpp"

using namespace std;

#define BUFF_SIZE 1024

class text {
  ifstream input;
  char buffer[BUFF_SIZE];
  int fileSize;
  int currSize;
  int index;


public:
  text() {
  }
    
  text(string filename) : index(0), currSize(0) {
    input.open(filename, ios::in | ios::binary);
    input.seekg(0, input.end);
    fileSize = input.tellg();
    input.seekg(0, input.beg);
  }
  static void cleanUpWord(string& word) {
    int amount;
    for (int i = 0; i < word.size(); i++) {
      amount = isPunctuation(&word[i]) + isNumber(word[i]);
      word.erase(i, amount);
      i -= (amount > 0);
    }
  }
  string serveWord() {
    if (currSize == fileSize && index + 1 >= input.gcount()) {
      return " EOF";
    }
    if (index == input.gcount()) {
      input.read(buffer, BUFF_SIZE);
      index = 0;
      currSize += input.gcount();
    }
    string word;
    while (index < BUFF_SIZE && buffer[index] != ' ' && buffer[index] != '\n' && buffer[index] != '-' && buffer[index] != '/') {
      word.append(1, buffer[index]);
      index++;
    }
    if (index == BUFF_SIZE && currSize != fileSize) {
      input.seekg(-word.length(), input.cur);
      fileSize += word.length();
      return serveWord();
    }
    while (index < BUFF_SIZE && (buffer[index] == ' ' || buffer[index] == '\n' || buffer[index] == '-' || buffer[index] == '/')) {
      index++;
    }
    cleanUpWord(word);
    if (!word.size())
      word = serveWord();
    return word;
  }
};
