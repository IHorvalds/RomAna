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
  
  // Open the normal file
  text txt(textName);
  
  // Open the latin_file
  string latin_text_name = textName;
  latin_text_name = "latin_" + latin_text_name;
  text latin_txt(latin_text_name); 
  
  string word, latin_word;
  
  // Read the first words
  word = txt.serveWord();
  latin_word = latin_txt.serveWord();
  while ((word != " <EOF> ") && (latin_word != " <EOF> ")) {
    // Update the frequencies
    dict.updateFreq(word, latin_word);
    word = txt.serveWord();
    latin_word = latin_txt.serveWord();
  }
  
  dict.showFreqs(outName);

  if (argc == 6)
    dict.saveExternal(argv[5]);
  return 0;
}
