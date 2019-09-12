#include <cstring>

using namespace std;

#define COUNT_PUNCTUATION_ONE 16
#define COUNT_PUNCTUATION_TWO 2
#define COUNT_PUNCTUATION_THREE 4


int punctuationOne[COUNT_PUNCTUATION_ONE] = {44,45,46,58,59,63,33,40,41,91,93,123,125,34,39,92};
int punctuationTwo[COUNT_PUNCTUATION_TWO] = {15787, 15803};
int punctuationThree[COUNT_PUNCTUATION_THREE] = {-1933402, -1933410, -1933411, -1933421};
// int diacritic[] = {15235,15234,15522,15490,14233,14232,14235,14234,15534,15502};

int isPunctuation(char* str) {
  for (int i = 0; i < COUNT_PUNCTUATION_ONE; i++) {
    if (str[0] == punctuationOne[i])
      return 1;
  }
  if (strlen(str) == 1)
    return 0;
  int hash = (str[0] << 8) ^ str[1];
  for (int i = 0; i < COUNT_PUNCTUATION_TWO; i++) {
    if (hash == punctuationTwo[i]) {
      return 2;
    }
  }
  if (strlen(str) == 2)
    return 0;
  hash = (hash << 8) ^ str[2];
  for (int i = 0; i < COUNT_PUNCTUATION_THREE; i++) {
    if (hash == punctuationThree[i]) {
      return 3;
    }
  }
  return 0;
}

int isNumber(char ch) {
  return (ch >= '0') && (ch <= '9');
}

/*
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
*/
