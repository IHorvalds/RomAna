#include <iostream>
#include <cstdlib>

#include "trie.cpp"
#include "parserOltean.cpp"

using namespace std;

#define BUILD_TRIE 0
#define PROCESS_TEXT 1
#define UPDATE_DICT 2

void errorArgs(int32_t option, int32_t argc, int32_t expected) {
  if (argc != expected) {
    std::cout << "You have chosen the option " << option << ", but the number of args is false! There should be " << expected << " args " << std::endl;
    exit(0);
  }
}

void dictionaryTask(trie& dict, char* textName) {
  // Open the normal file
  text txt(textName);
  
  // Open the latin_file
  string latin_text_name = textName;
  latin_text_name = "latin_" + latin_text_name;
  text latin_txt(latin_text_name); 
  
  // Read the first words
  string word = txt.serveWord();
  string latin_word = latin_txt.serveWord();
  
  // Continue parsing the text until its end
  while ((word != " <EOF> ") && (latin_word != " <EOF> ")) {
    // Update the frequencies
    dict.updateFreq(word, latin_word);
    word = txt.serveWord();
    latin_word = latin_txt.serveWord();
  }
}

int main(int argc, char** argv) {
  if (argc < 3) {
    std::cout << "The following options are available: 0(BUILD_TRIE) or 1 (PROCESS_TEXT) or 2 (UPDATE_DICT)]" << std::endl;
    std::cout << "You should insert one of the following schemas: " << std::endl;
    std::cout << "BUILD_TRIE : python3 convert_into_latin.py [file to read the inflections from] && " << argv[0] << " 0 [file to read the inflections from] [file in which to save the dictionary]" << std::endl;
    std::cout << "PROCESS_TEXT : python3 convert_into_latin.py [input file name] && " << argv[0] << " 1 [dictionary file name] [input file name] [output file name]" << std::endl;
    std::cout << "UPDATE_DICT : python3 convert_into_latin.py [input file name] && " << argv[0] << " 2 [dictionary file name] [input file name] [output file name] [new dictionary file name]" << std::endl;
    return -1;
  }
  
  uint32_t option = atoi(argv[1]);
  switch (option) {
    case BUILD_TRIE : {
      errorArgs(BUILD_TRIE, argc, 4); 
      
      const char* file_name = argv[2];
      const char* save_into = argv[3];
      string tmp = file_name;
      tmp = "latin_" + tmp;
      const char* latin_file_name = tmp.data();
      
      trie A;
      
      // Read the files
      A.consumeInflexions(file_name, latin_file_name);
      
      // And save the trie
      A.saveExternal(save_into);
      
#if 0
      std::cerr << "Has this step been reached? Then your dictionary has been succesfully stored in " << save_into << ", although it might exist some problems with the allocation. We will get rid of them in short time." << std::endl;
#endif
      break;
    }
    case PROCESS_TEXT : {
      errorArgs(PROCESS_TEXT, argc, 5); 
      
      // Use the trie while parsing the text
      string dictName = argv[2];
      trie dict(dictName.data());
      dictionaryTask(dict, argv[3]);
      
      // Print the frequencies obtained
      dict.showFreqs(argv[4]);
      break;
    }
    case UPDATE_DICT : {
      errorArgs(UPDATE_DICT, argc, 6); 
      
      // Use the trie while parsing the text
      string dictName = argv[2];
      trie dict(dictName.data());
      dictionaryTask(dict, argv[3]);
      
      // Print the frequencies obtained
      dict.showFreqs(argv[4]);
      
      // Save the new obtained dictionary
      dict.saveExternal(argv[5]);
      break;
    }
    default : {
      std::cout << "The option " << argv[1] << " is not (yet) available!" << std::endl;
    }
  }
  return 0;
}
