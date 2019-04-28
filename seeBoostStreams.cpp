#include <string>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emmintrin.h>
#include <pmmintrin.h>
#include <smmintrin.h>

#include <boost/iostreams/device/mapped_file.hpp>

using namespace std;

#define TEST_SIZE 10000000
#define MAX_SIZE_PATTERNS 2000
#define MAX_LEN 2000

typedef struct {
  short int ell, per, len;
  bool flag;
} cell;

#include <sys/time.h>

long long start;

inline long long getTime() {
  struct timeval tv;

  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000000LL + tv.tv_usec;
}

int N, len, ss, n, diff;
const char* text;
string query;
char stack[MAX_LEN + 2];
bool any = false, startWithWord = false, endWithWord = false;
int countProzent = 0, countWords = 0;             // counters.
int countWinner = 0;

// Words that are extracted from the query.
int size = 0;
char* patterns[MAX_SIZE_PATTERNS];
cell save[MAX_SIZE_PATTERNS];
int *sums; // the partial sums of the lenghts of the patterns.

int MAX(int x, int y) {
  return x > y ? x : y;
}

int maxSuf(char *x, int m, int *p) {
   int ms, j, k;
   char a, b;

   ms = -1;
   j = 0;
   k = *p = 1;
   while (j + k < m) {
      a = x[j + k];
      b = x[ms + k];
      if (a < b) {
         j += k;
         k = 1;
         *p = j - ms;
      }
      else
         if (a == b)
            if (k != *p)
               ++k;
            else {
               j += *p;
               k = 1;
            }
         else { /* a > b */
            ms = j;
            j = ms + 1;
            k = *p = 1;
         }
   }
   return(ms);
}

/* Computing of the maximal suffix for >= */
int maxSufTilde(char *x, int m, int *p) {
   int ms, j, k;
   char a, b;

   ms = -1;
   j = 0;
   k = *p = 1;
   while (j + k < m) {
      a = x[j + k];
      b = x[ms + k];
      if (a > b) {
         j += k;
         k = 1;
         *p = j - ms;
      }
      else
         if (a == b)
            if (k != *p)
               ++k;
            else {
               j += *p;
               k = 1;
            }
         else { /* a < b */
            ms = j;
            j = ms + 1;
            k = *p = 1;
         }
   }
   return(ms);
}

// returns 1 only if l == r for "size" characters.
int myMemcmp(char *l, char *r, int size) {
  int i = 0;
  while (i < size && l[i] == r[i]) {
    i++;
  }
  return i == size;
}

/** Preprocessing. **/
void analyze(char *pattern, int pos, int len) {
  int p, q;
  int i = maxSuf(pattern, len, &p);
  int j = maxSufTilde(pattern, len, &q);
  if (i > j) {
    save[pos].ell = i;
    save[pos].per = p;
  } else {
    save[pos].ell = j;
    save[pos].per = q;
  }
  save[pos].len = len;

  // Fuer die Verbesserung der Zeit in der Suche.
  save[pos].flag = false;
  if (myMemcmp(pattern, pattern + save[pos].per, save[pos].ell + 1) == 1) {
    save[pos].flag = true;
    save[pos].per = MAX(save[pos].ell + 1, len - save[pos].ell - 1) + 1;
  }
}

/** From sequences as '%%%%' keep only one sign of '%'. **/
void reduce(char *s, int *len) {
  int i = 1, buff = 1;
  while (i < (*len)) {
    if (!(s[i] == '%' && s[buff - 1] == '%')) {
      s[buff++] = s[i];
    }
    i++;
  }
  s[buff] = '\0';
  // change the new length.
  (*len) = buff;
}

/** Takes a word from the stack and saves it. **/
void formWord() {
  if (ss == 0) {
    return;
  }
  // Only if the stack isn't empty!
  countWords++;
  stack[ss] = '\0';
  patterns[size] = (char*)calloc(ss + 1, sizeof(char));
  strcpy(patterns[size], stack);
  analyze(patterns[size], size, ss);

  size++;
  // Reset the stack.
  ss = 0;
}

/** Split the string "s" into words, separated by '%'. */
void split(char *s, int len) {
  if (len == 0) {
    fprintf(stderr, "Your query is empty!");
    return;
  }

  // Nach diesem Aufruf kann "len" mofiziert werden werden.
  reduce(s, &len);

  // Verifiziere, ob alle Eintraege gueltig sind <-> any == true.
  if (len == 1 && s[0] == '%') {
    any = 1;
    return;
  }

  if (s[0] != '%') {
    startWithWord = 1;
  }
  if (s[len - 1] != '%') {
    endWithWord = 1;
  }

  int i = 0;
  while (i < len) {
    if (s[i] == '%') {
      countProzent++;
      // Falls nur, der Stapel ist nicht leer.
      formWord();
    } else {
      stack[ss++] = s[i];
    }
    i++;
  }
  // Das hier ist sehr wichtig, es kann ein Wort im Stapel bleiben.
  formWord();
}

// Berechne alles Notwendige fuer die Query.
void prelucrate(char *query) {
  len = strlen(query);
  split(query, len);

  //fprintf(stdout, "any = %d, s = %d, e = %d, p = %d, w = %d\n", any, startWithWord, endWithWord, countProzent, countWords);
  sums = (int*)calloc(size + 1, sizeof(int));
  sums[size] = 0;
  for (int i = size - 1; i >= 0; i--) {
    sums[i] = sums[i + 1] + save[i].len;
  }
}

/** Two-way algorithm: search patterns[pos] in "str" from "fromIndex" and returns
the first index (+ m) (the first index after the sought word) where pattersn[pos] appears in str. **/
// Ausbesserung: ob es Sinn macht, weiterzugehen, obwohl sums[pos] ueberpsrungen wurde.
int mainSearch(int pos, int fromIndex) {
  int i, j, lim, memory;
  int ell = save[pos].ell, per = save[pos].per, m = save[pos].len;
  char *needle = patterns[pos];

  if (save[pos].flag) {
    j = fromIndex;
    memory = -1;
    // Falls wir weitermachen koennten (betrachte sums[pos + 1]).
    lim = n - m + diff - sums[pos + 1];
    //printf("lim = ------------%d\n", lim);
    while (j <= lim) {
      i = MAX(ell, memory) + 1;
      while (i < m && needle[i] == text[i + j]) {
        i++;
      }
      if (i >= m) {
        i = ell;
        while (i > memory && needle[i] == text[i + j]) {
          i--;
        }
        if (i <= memory) {
          return j + m;
        }
        j += per;
        memory = m - per - 1;
      } else {
        j += (i - ell);
        memory = -1;
      }
    }
  } else {
    // Wir haben schon "per" geaendert in analyze.
    j = fromIndex;
    // Falls wir weitermachen koennten (betrachte sums[pos + 1]).
    lim = n - m + diff - sums[pos + 1];
    //printf("lim = %d\n", lim);
    //printf("lim = %d de %d %d %d\n", lim, j, sums[pos + 1] - diff, m);
    while (j <= lim) {
      i = ell + 1;
      while (i < m && needle[i] == text[i + j]) {
        i++;
      }
      if (i >= m) {
        i = ell;
        while (i >= 0 && needle[i] == text[i + j]) {
          i--;
        }
        if (i < 0) {
          return j + m;
        }
        j += per;
      } else {
        j += (i - ell);
      }
    }
  }
  return -1;
}

// Verbesserung (ja, das macht die Sachen besser) des mainSearch.
// Sucht den ersten Index i, wo str[i] == patterns[pos][0].
int search(int pos, int fromIndex) {
  if (save[pos].len == 1) {
    int upper_bound = n - (sums[pos] - diff);
    while (fromIndex <= upper_bound && text[fromIndex] != patterns[pos][0]) {
      fromIndex++;
    }
    return (fromIndex > upper_bound) ? -1 : fromIndex + 1;//mainSearch(pos, fromIndex);
  } else {
    int upper_bound = n - (sums[pos] - diff);
    //printf("%d..%s\n", upper_bound, str + upper_bound);
    if (false && (fromIndex<= upper_bound)) {
       auto test=static_cast<const char*>(memchr(text+fromIndex,patterns[pos][0],upper_bound-fromIndex+1));
       if (!test) return -1;
       fromIndex=test-text;
    }
    while (fromIndex <= upper_bound && (text[fromIndex] != patterns[pos][0] || text[fromIndex + 1] != patterns[pos][1])) {
      fromIndex++;
    }
    //printf("other way %d\n", fromIndex);
    return (fromIndex > upper_bound) ? -1 : mainSearch(pos,fromIndex);
  }
  //return mainSearch(pos, fromIndex);
}

// Diff bedeutet: falls wir suffix haben, dann muessen wir jeder Summe diff (patterns[size - 1].len) abziehen.
int searchAll(int from, int to, int index) {
  int i = from, next = index;
  bool flag = true;
  //printf("%d %d %d\n", from, to, diff);
  while (i < to && flag) {
    next = search(i, next);
    if (next == -1) {
      flag = false;
    // Besonderer Fall: wir sind am letzten Pattern.
    } else if (sums[i + 1] - diff + next > n) {
      flag = false;
    }
    //flag = (next != -1) && (sums[i + 1] - diff + next <= n);
    i++;
  }
  return flag;
}

// Ueberpruefung, ob patterns[pos] Prefix von text ist.
bool prefix(int pos) {
  if (n < save[pos].len) {
    return false;
  }
  int i = 0;
  while (i < save[pos].len && text[i] == patterns[pos][i]) {
    i++;
  }
  return i == save[pos].len;
}

// Analog zu Prefix.
bool suffix(int pos) {
  if (n < save[pos].len) {
    return false;
  }
  int i = 0;
  while (i < save[pos].len && patterns[pos][i] == text[n - save[pos].len + i]) {
    i++;
  }
  return i == save[pos].len;
}

// Das Query nur noch aus einem Wort besteht wurde ausgeschlossen.
int queryWord() {
#if 1
  diff = 0;
  if (any) {
    return 1;
  }
  if (sums[0] > n) {
    return 0;
  }
  if (startWithWord) {
    if (!prefix(0)) {
      return 0;
    }
    if (endWithWord) {
      if (!suffix(size - 1)) {
        return 0;
      }
      // D.h. es gibt ein Suffix, also wir terminieren wo das letzte Pattern beginnt.
      n -= save[size - 1].len;
      diff = save[size - 1].len;
      return searchAll(1, size - 1, save[0].len);
    } else {
      // Ganz normal, wir suchen ab Index save[0].len bis n.
      return searchAll(1, size, save[0].len);
    }
  } else if (endWithWord) {
    if (!suffix(size - 1)) {
      return 0;
    }
    n -= save[size - 1].len;
    // Ganz normal, suche ab Index 0, bis n und betrachte nicht mehr das letzte Pattern.
    diff = save[size - 1].len;
    return searchAll(0, size - 1, 0);
  } else {
    // Es gibt keinen besonderen Fall.
    return searchAll(0, size, 0);
  }
#else
   auto reader=text,limit=reader+n;
   for (int index=0;index!=size;++index) {
      auto sep=static_cast<const char*>(memmem(reader,limit-reader,patterns[index],save[index].len));
      if (!sep) return false;
      reader=sep+save[index].len;
   }
   return true;
#endif
}

int myStrcmp(int pos) {
  if (save[pos].len != n) {
    return 0;
  }
  int i = 0;
  while (i < save[pos].len && text[i] == patterns[pos][i]) {
    i++;
  }
  return i == save[pos].len;
}

void match(const char* /*str*/) {
  countWinner++;
}

static inline const char* findNl(const char* reader,const char* readerLimit)
{
#if 0
   auto bars=_mm_set1_epi8('\n');
   auto limit=readerLimit-16;
   for (;reader<=limit;reader+=16) {
      auto nextChars=_mm_lddqu_si128(reinterpret_cast<const __m128i*>(reader));
      auto cmp=_mm_cmpeq_epi8(nextChars,bars);
      unsigned mask=_mm_movemask_epi8(cmp);
      if (mask)
         return reader=reader+__builtin_ctzl(mask);
   }
   for (;reader<=readerLimit;++reader)
      if ((*reader)=='\n')
         return reader;
   return nullptr;
#else
   return static_cast<const char*>(memchr(reader, '\n', readerLimit-reader));
#endif
}

int main(void) {
  int i;
  boost::iostreams::mapped_file_source f("gen12.out");
  auto fileBegin=f.data(),fileEnd=fileBegin+f.size(),reader=fileBegin;

  start = getTime();

  {
     auto eol=findNl(reader,fileEnd);
     query=string(reader,eol);
     reader=eol+1;
  }
  prelucrate(const_cast<char*>(query.c_str()));

  cout << query << endl;

  for (int i = 0; i < TEST_SIZE; i++) {
    if (!reader) break;
    text=reader;
    {
      auto eol=findNl(reader,fileEnd);
      n = eol?(eol-reader):(fileEnd-reader);
      reader = eol?(eol+1):nullptr;
    }

    //printf("%s\n", text);
    if (any) {
      match(text);
    } else if (countProzent == 0) {
      // Query besteht nur aus einem Wort.
      if (countWords != 0) {
        if (myStrcmp(0)) {
          match(text);
        }
      // Sowohl text als auch query sind leer.
      } else if (n == 0) {
        match(text);
      }
    } else {
      if (queryWord()) {
        match(text);
      }
    }
  }
  long long timeElapsed = getTime() - start;
  cout << countWinner << " -> time: " << timeElapsed << endl;
  return 0;
}
