#ifndef SPECIAL_CHARACTERS_H
#define SPECIAL_CHARACTERS_H

#include <string>
#include <cstring>
#include <cctype>

namespace specialChars {
  static constexpr unsigned count_punctuation_1byte = 17;
  static constexpr unsigned count_punctuation_2byte = 2;
  static constexpr unsigned count_punctuation_3byte = 4;

  // TODO: add the coresponding character to each encoding 
  static int32_t punctuation_1byte[count_punctuation_1byte] = 
  {
    35,  // #
    44,  //  
    45,  //
    46,  //
    58,  // 
    59,  //
    63,  //
    33,  //
    40,  //
    41,  //
    91,  //
    93,  //
    123, //
    125, //
    34,  //
    39,  //
    92   //
  };
  
  static int32_t punctuation_2byte[count_punctuation_2byte] = 
  {
    15787, //
    15803  //
  };
  
  static int32_t punctuation_3byte[count_punctuation_3byte] = 
  {
    -1933402, // 
    -1933410, //
    -1933411, //
    -1933421  //
  };
  
  // TODO: do we still need this one?
  // int diacritic[] = {15235,15234,15522,15490,14233,14232,14235,14234,15534,15502};

  int isPunctuation(char* str) {
    for (unsigned i = 0; i < count_punctuation_1byte; i++)
      if (str[0] == punctuation_1byte[i])
        return 1;
    if (strlen(str) == 1)
      return 0;
    
    // TODO: Alex the problem is: str[1] can be ngative, so it moves the sign-bit forward and changes everything.
    // Encore un problem: str[0] is a byte. When you shift by 8...
    int hash = (str[0] << 8) ^ str[1];
    for (unsigned i = 0; i < count_punctuation_2byte; i++)
      if (hash == punctuation_2byte[i])
        return 2;
    if (strlen(str) == 2)
      return 0;
    
    hash = (hash << 8) ^ str[2];
    for (unsigned i = 0; i < count_punctuation_3byte; i++)
      if (hash == punctuation_3byte[i])
        return 3;
    return 0;
  }

  // Clean up the word of punctuations
  static void cleanUpWord(std::string& word) {
    int amount;
    for (int i = 0; i < word.size(); i++) {
      amount = isPunctuation(&word[i]) + isdigit(word[i]);
      word.erase(i, amount);
      i -= (amount > 0);
    }
  }  
#if 0
  int isDiacritic(char* str) {
    if (strlen(str) < 2)
      return 0;
    int hash = (str[0] << 8) ^ str[1];
    for (int i = 0; i < 10; i++) {
      if (hash == diacritic[i]) {
        return 1;
      }
    }
    return 0;
  }
#endif
}

#endif // SPECIAL_CHARACTERS_H
