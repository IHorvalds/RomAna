#ifndef DIACRITICA_H
#define DIACRITICA_H

#include <cctype>
#include <string>

namespace romanian {
  static constexpr unsigned ROMANIAN_SIGMA = 31;
  static constexpr unsigned ENGLISH_SIGMA = 26;
  
  // TODO: This should be used in diacriticEncode
#if 0
  static constexpr unsigned COUNT_OF_DIACRITICS = 10;
  static romanian_diacritics[COUNT_OF_DIACRITICS][2] = 
  {
    {, }, // ă
    {0xc3, 0x83}, // big ă
    
    {, }, // â
    {0xc3, 0x82}, // big â
    
    {, }, // î 
    {0xc3, 0x8e}, // big î
    
    {, }, // ș
    {, }, // big ș
  
    {, }, // ț
    {, }  // big ț
  };
#endif
  static bool isEnglishAlphabetIndex(int32_t alphabetIndex) {
    return (0 <= alphabetIndex) && (alphabetIndex < ENGLISH_SIGMA);
  }
  
  // TODO: is it really precise?
  static bool isDiacritic(char c) {
    return ((c > -62) && (c < -55));
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
        default:
          ret = "UNKNOWN";
      }
    }
    return ret;
  }


  // Check two bytes from string to see if you got a diacritică
  // TODO: Are you sure? Is there other pair of (byte1, byte2) which is not a diacritica, but will still be considered one 
  static int32_t diacriticEncode(int8_t byte1, int8_t byte2) {
    int32_t c = byte1 + byte2;
    if ((c == -186) || (c == -185)) // ă
      return 26;
    if ((c == -187) || (c == -155)) // â
      return 27;
    if ((c == -175) || (c == -143)) // î
      return 28;
    if ((c == -160) || (c == -159)) // ș
      return 29;
    if ((c == -158) || (c == -157)) // ț
      return 30;
    // no diacritică? ok, call function for regular letter
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
      if (ret == -1) {
        // std::cerr << str << std::endl;
        return false;
      }
      return isRomanian(str, index + 1 + !isEnglishAlphabetIndex(ret)); 
    }
  }
};

#endif // DIACRITICA_H
