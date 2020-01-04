#include <iostream>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <algorithm>

#include "processing/InflexEngine.hpp"
#include "processing/InflexEngineCore.cpp"
#include "processing/InflexEngineUtil.cpp"
#include "processing/parserOltean.hpp"
#include "processing/analyze_poet.hpp"
#include "processing/util.hpp"

using namespace std;

#define BUILD_TRIE 0
#define PROCESS_TEXT 1
#define UPDATE_DICT 2
#define ANALYZE_POET 3
#define GINIFY_POET 4
#define ESSENCE_POET 5
#define COMPUTE_SIMILARITY 6

void errorArgs(int32_t option, int32_t argc, int32_t expected) {
  if (argc != expected) {
    std::cout << "You have chosen the option " << option << ", but the number of args is false! There should be " << expected << " args " << std::endl;
    exit(0);
  }
}

void dictionaryTask(InflexEngine& dict, char* textName) {
  // Open the normal file
  text txt(textName);
  
  // Open the latin_file
  std::string tmp = textName;
  std::string pythonCommand = "python3 processing/convert_into_latin.py " + tmp;
  int warning = system(pythonCommand.data());
  tmp = "latin_" + tmp;
  text latin_txt(tmp); 

  // Read the first words
  std::string word = txt.serveWord();
  std::string latin_word = latin_txt.serveWord();
  
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
    std::cout << "The following options are available: 0(BUILD_TRIE) or 1 (PROCESS_TEXT) or 2 (UPDATE_DICT) or 3 (PROCESS_POET) or 4 (GINIFY_POET) or 5 (ESSENCE_POET) or 6 (COMPUTE_SIMILARITY)" << std::endl;
    std::cout << "You should insert one of the following schemas: " << std::endl;
    std::cout << "BUILD_TRIE: " << argv[0] << " 0 [file to read the inflections from] [file in which to save the dictionary]" << std::endl;
    std::cout << "PROCESS_TEXT: " << argv[0] << " 1 [dictionary file name] [input file name] [output file name]" << std::endl;
    std::cout << "UPDATE_DICT: " << argv[0] << " 2 [dictionary file name] [input file name] [output file name] [new dictionary file name]" << std::endl;
    std::cout << "ANALYZE_POET: " << argv[0] << " 3 [poet name from poets.txt]" << std::endl;
    std::cout << "GINIFY_POET: " << argv[0] << " 4 [poet name from poets.txt]" << std::endl;
    std::cout << "ESSENCE_POET: " << argv[0] << " 5 [poet name from poets.txt] [distribution type - richness or relative] [sortValues - if the ratios should be sorted]" << std::endl;
    std::cout << "COMPUTE_SIMILARITY: " << argv[0] << " 6 [poet name from poets.txt]" << std::endl;
    return -1;
  }
  
  uint32_t option = atoi(argv[1]);
  switch (option) {
    case BUILD_TRIE : {
      errorArgs(BUILD_TRIE, argc, 4); 
      
      const char* filename = argv[2];
      const char* saveInto = argv[3];
      
      // Create the latin file with python
      std::string tmp = filename;
      std::string pythonCommand = "python3 processing/convert_into_latin.py " + tmp;
      int warning = system(pythonCommand.data());
      size_t pos = tmp.find_last_of("/");
      if (pos == std::string::npos)
        tmp = "latin_" + tmp;
      else
        tmp = tmp.substr(0, pos) + "/latin_" + tmp.substr(pos + 1, tmp.size());
      const char* latinFilename = tmp.data();
      
      // Alloc the dictionary
      InflexEngine A(InflexEngine::Build, filename, latinFilename);
      
      std::cerr << "Size of InflexEngine = " << A.getSize() << std::endl;
      
      // And save the InflexEngine
      A.saveExternal(saveInto);
      
      // Erase the temporary latin file
      std::string eraseCommand = "rm " + tmp;
      warning = system(eraseCommand.data());
      break;
    }
    case PROCESS_TEXT : {
      errorArgs(PROCESS_TEXT, argc, 5); 
      
      // Use the InflexEngine while parsing the text
      std::string dictName = argv[2];
      InflexEngine dict(InflexEngine::Load, dictName.data());
      dictionaryTask(dict, argv[3]);
     
      // Print the frequencies obtained
      dict.showFreqs(argv[4]);
      break;
    }
    case UPDATE_DICT : {
      errorArgs(UPDATE_DICT, argc, 6); 
      
      // Use the InflexEngine while parsing the text
      std::string dictName = argv[2];
      InflexEngine dict(InflexEngine::Load, dictName.data()); 
      
      dictionaryTask(dict, argv[3]);
      
      // Print the frequencies obtained
      dict.showFreqs(argv[4]);
    
      // Save the new obtained dictionary
      dict.saveExternal(argv[5]);
      break;
    }
    case ANALYZE_POET : {
      errorArgs(ANALYZE_POET, argc, 3);
      
      // Compute the local frequencies of each word
      std::string poetName = argv[2];  
      PoetAnalyzer analyzer(poetName);
      analyzer.saveFrequencies();
      break;
    }
    case GINIFY_POET : {
      errorArgs(GINIFY_POET, argc, 3);
      
      // Ginify the poet, i.e, compute the local ginis
      std::string poetName = argv[2];
      LocalGiniGenerator gini(poetName);
      gini.save();
      break;
    }
    case ESSENCE_POET : {
      errorArgs(ESSENCE_POET, argc, 5);
      
      // Ginify the poet, i.e, compute the gini coefficient for each word
      std::string poetName = argv[2];
      std::string distributionType = argv[3];
      bool sortValues = atoi(argv[4]);
      EssenceGenerator essence(poetName, distributionType, sortValues);
      essence.save();
      break;
    }
    case COMPUTE_SIMILARITY : {
      errorArgs(COMPUTE_SIMILARITY, argc, 3);
      
      // Ginify the poet, i.e, compute the gini coefficient for each word
      std::string poetName = argv[2];
      SimilarityGenerator cmp(poetName);
      cmp.save();
      break;
    }
    default : {
      std::cout << "The option " << argv[1] << " is not (yet) available!" << std::endl;
    }
  }
  return 0;
}
