#include "trie.cpp"
#include "parserOltean.cpp"

int main(int argv, char** argc) {
  string dictName, textName, outName, newDictName;
  if (argv != 4 && argv != 6) {
    cout << "Usage: ./a.out [dictionary file name] [input file name] [output file name]" << endl;
    cout << "\tAdd the paramter -u [new dictionary file name] if you want to save the dictionary updated with the new words & frequencies found" << endl;
    return -1;
  }
  dictName = argc[1];
  textName = argc[2];
  outName  = argc[3];
  if (argv == 6)
    newDictName = argc[5];
  
  trie dict(&dictName[0]);
  text txt(textName);
  string word;
  
  word = txt.serveWord();
  int place;
  while (word != " ") {
    dict.updateFreq(word);
    word = txt.serveWord();
  }
  dict.showFreqs(outName);

  if (argv == 6)
    dict.saveExternal(argc[5]);
  
  return 0;
}
