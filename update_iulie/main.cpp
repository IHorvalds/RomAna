#include "trie.cpp"
#include "parserOltean.cpp"

int main(int argc, char** argv) {
  string dictName, textName, outName, newDictName;
  if (argc != 4 && argc != 6) {
    cout << "Usage: " << argv[0] << " [dictionary file name] [input file name] [output file name]" << endl;
    cout << "\tAdd the paramter -u [new dictionary file name] if you want to save the dictionary updated with the new words & frequencies found" << endl;
    return -1;
  }
  dictName = argv[1];
  textName = argv[2];
  outName  = argv[3];
  if (argc == 6)
    newDictName = argv[5];
  
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

  if (argc == 6)
    dict.saveExternal(argv[5]);
  
  return 0;
}
