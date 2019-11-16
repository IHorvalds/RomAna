#ifndef ANALYZE_POET_H
#define ANALYZE_POET_H

#include <fstream>
#include <map>
#include <numeric>
#include <vector>
#include <cassert>
#include "trie.hpp"

class PoetAnalyzer {
  private:
  uint32_t countPoems;
  uint64_t countWords;
  std::string poet;
  std::vector<uint32_t> sizeOfPoems;
  
  // For which word we store the absolute frequency and the index of the respective poem
  // We do so, in order not to lose the precision when reading from files
  std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> wordToFreqs;
    
  void dictionaryTask(trie& dict, const char* textName) {
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
    txt.close();
  }

  void receivePoem(std::string poem) {
    std::string pythonCommand = "python3 htmlparser/extract_poetry.py " + poem;
    
    // The poetry will be saved in "parsed.txt"
    int warning = system(pythonCommand.data());
  }
  
  void processPoems() {
    std::string filename = "poets/poems/" + poet + "_list_poems.txt";
    std::ifstream in;
    
    // Compute the number of poems
    in.open(filename);
    uint32_t totalCountOfPoems = 0;
    std::string str;
    while (in >> str)
      ++totalCountOfPoems;
    in.close();
    
    // Don't alloc the dictionary
    trie dict;
    
    // Read each poem from the list
    in.open(filename);
    std::string currPoem; 
    std::cout << "Start the analysis of " << this->poet << std::endl;
    while (in >> currPoem) {
      ++countPoems;
      
      std::cout << "[" << static_cast<unsigned>(100 * (double)(countPoems - 1) / totalCountOfPoems) << "%]" << ":  currently analyzing '" << currPoem << "'" << std::endl;
      
      // Read the poetry from the html file
      receivePoem(currPoem);
      
      // Reset the trie with the initial dictionary
      dict.reset("dict.bin");
      
      // Parse the poetry and save the frequencies into trie
      dictionaryTask(dict, "parsed.txt");
    
      // Get the frequencies of inflections
      std::vector<std::pair<uint32_t, std::string>> freqs = dict.getFrequencies();
    
      // Compute the size of the current poem and store it
      uint32_t poemSize = 0;
      for (auto e : freqs)
        poemSize += e.first;
      assert(poemSize);
      sizeOfPoems.push_back(poemSize);
    
      // Update the map with frequencies per word
      for (auto iter: freqs) {
        std::string word = iter.second;
        uint32_t currFreq = iter.first;
        
        // Store the index of the poem along the absolute frequency
        wordToFreqs[word].push_back(std::make_pair(currFreq, countPoems - 1));
      }
    }
    std::cout << "[100%]: " << this->poet << " is finished" << std::endl;
  }

  public:
  
  PoetAnalyzer(std::string poet_name) : poet(poet_name) {
    countPoems = 0;
    countWords = 0;
    wordToFreqs.clear();
  }

  void setPoet(std::string poet) {
    this->poet = poet;
    countPoems = 0;
    countWords = 0;
    wordToFreqs.clear();
  }
  
  std::string getPoet() const {
    return this->poet;
  }
  
  void saveFrequencies() {
    std::string filename = "poets/local/frequency/" + poet + "_local_frequencies.txt";
    std::ofstream out(filename);
    
    // Parse all his/her poems
    processPoems();
    
    /** Save in the file "poet_words_frequencies.txt"
      The encoding is:
      On the first line of the file: [countPoems] [sizeOfEachPoem]
      word = [word] count = [number of poems the words appears in] [list of pair(absolute frequency, index of poem)]
    **/
    
    // Print the first 2 lines
    out << countPoems;
    for (auto e : sizeOfPoems)
        out << " " << e;
    out << "\n";
    
    // And the rest
    for (auto iter : wordToFreqs) {
      std::string word = iter.first;
      std::vector<std::pair<uint32_t, uint32_t>> freqs = iter.second;

      // Sort the frequencies
      std::sort(freqs.begin(), freqs.end(), [](auto& left, auto& right) {
        return (left.first < right.first);
      });

      // And print the file
      out << word << " " << freqs.size();
      for (auto elem : freqs)
        out << " " << elem.first << " " << elem.second; 
      out << "\n";
    }
  }
};

#endif // ANALYZE_POET_H
