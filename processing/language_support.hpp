#ifndef DIACRITICA_H
#define DIACRITICA_H

#include <cctype>
#include <string>

namespace romanian {
  static constexpr unsigned ROMANIAN_SIGMA = 31;
  static constexpr unsigned ENGLISH_SIGMA = 26;
  
  static constexpr unsigned COUNT_OF_DIACRITICS = ROMANIAN_SIGMA - ENGLISH_SIGMA;
  static std::string solveDiacritics[COUNT_OF_DIACRITICS] = {"ă", "â", "î", "ș", "ț"};
  static uint8_t diacritics[2 * COUNT_OF_DIACRITICS][2] = 
  {
    {0xC4, 0x83}, // ă
    {0xC3, 0x83}, // Ă
    
    {0xC3, 0xA2}, // â
    {0xC3, 0x82}, // Â
    
    {0xC3, 0xAE}, // î 
    {0xC3, 0x8E}, // Î
    
    {0xC8, 0x99}, // ș
    {0xC8, 0x98}, // Ș
  
    {0xC8, 0x9B}, // ț
    {0xC8, 0x9A}  // Ț
  };
  
  static bool isEnglishAlphabetIndex(int32_t alphabetIndex) {
    return (0 <= alphabetIndex) && (alphabetIndex < ENGLISH_SIGMA);
  }
  
  // TODO: is it really precise? Use the tables!!!
  static bool isDiacritic(uint8_t byte) {
    for (unsigned index = 0; index < 2 * COUNT_OF_DIACRITICS; ++index)
      if (diacritics[index][0] == byte)
        return true;
    return false;
  }

  // If "c" is a letter, returns the alphabetic order of c. Otherwise, -1.
  static int32_t encode(char c) {
    int val = tolower(c) - 'a';
    if (isEnglishAlphabetIndex(val))
      return val;
    else
      return -1;
  }
  
  // Decode "encoding" into a romanian character
  static std::string decode(int32_t encoding) {
    std::string ret;
    if (isEnglishAlphabetIndex(encoding)) {
      ret = 'a' + encoding;
    } else {
      // Case for diacritics
      ret = "UNKNOWN";
      for (unsigned index = 0; index < COUNT_OF_DIACRITICS; ++index)
        if ((ENGLISH_SIGMA + index) == encoding)
          ret = solveDiacritics[encoding - ENGLISH_SIGMA];
    }
    return ret;
  }

  // Check two bytes from string to see if you got a diacritică
  static int32_t diacriticEncode(uint8_t byte1, uint8_t byte2) {
    for (unsigned index = 0; index < 2 * COUNT_OF_DIACRITICS; ++index)
      if ((diacritics[index][0] == byte1) && (diacritics[index][1] == byte2))
        return ENGLISH_SIGMA + (index >> 1);
    return encode(byte1);
  }
  
  // Check if "str" is a romanian word: this mean, it doesn't contain other characters than the romanian ones
  static bool isRomanian(std::string str, unsigned index = 0) {
    if (index == str.size())
      return true;
    if (index == str.size() - 1)
        return encode(str.back()) != -1;
    else {
      int ret = romanian::diacriticEncode(str[index], str[index + 1]);
      if (ret == -1)
        return false;
      return isRomanian(str, index + 1 + !isEnglishAlphabetIndex(ret)); 
    }
  }
};

#endif // DIACRITICA_H
