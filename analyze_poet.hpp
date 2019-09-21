// TODO: for poets with few poems it works. But for eminescu, segfault
#ifndef ANALYZE_POET_H
#define ANALYZE_POET_H

#include <fstream>
#include <map>
#include "trie.hpp"

class PoetAnalyzer {
  private:
  unsigned countPoems = 0;
  std::string poet;
  std::map<std::string, std::vector<uint32_t>> wordToFreqs;
  
  void dictionaryTask(trie& dict, const char* textName) {
    // Open the normal file
    text txt(textName);
    
    // Open the latin_file
    std::string tmp = textName;
    std::string pythonCommand = "python3 convert_into_latin.py " + tmp;
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

  void receivePoem(std::string poem) {
    std::string pythonCommand = "python3 htmlparser/extract_poetry.py " + poem;
    
    // The poetry will be saved in "parsed.txt"
    int warning = system(pythonCommand.data());
  }
  
  void processPoems() {
    std::string filename = "poets/" + poet + "_list_poems.txt";
    ifstream in(filename);
    
    // Don't alloc the dictionary
    trie dict(false);
    
    // Read each poem from the list
    std::string currPoem; 
    while (in >> currPoem) {
      ++countPoems;
      
      // Read the poetry from the html file
      receivePoem(currPoem);
      
      // Reset the trie with the initial dictionary
      dict.reset("dict.bin");
      
      // Parse the poetry and save the frequencies in trie
      dictionaryTask(dict, "parsed.txt");
      
      // Get the frequencies from inflections
      std::vector<std::pair<uint32_t, std::string>> freqs = dict.getFrequencies();
      
      // Update the map with frequencies per word
      for (auto iter: freqs) {
        std::string word = iter.second;
        uint32_t currFreq = iter.first;
        
        wordToFreqs[word].push_back(currFreq);
      }
    }
  }

  public:
  
  PoetAnalyzer(std::string poet_name) : poet(poet_name) {
    countPoems = 0;
    wordToFreqs.clear();
  }
  
  void setPoet(std::string poet) {
    this->poet = poet;
    countPoems = 0;
    wordToFreqs.clear();
  }
  
  std::string getPoet() const {
    return this->poet;
  }
  
  void saveFrequencies() {
    std::string filename = "poets/" + poet + "_words_frequencies.txt";
    ofstream out(filename);
    
    // Parse all his/her poems
    processPoems();
    
    // Save in the file "poet_words_frequencies.txt"
    // The encoding is:
    // At the beginning of the file: countPoems = [how many poems the poet has]
    // word = [word] count = [number of poems the words appears in] [list of "count" frequencies]
    out << countPoems << "\n";
    for (auto iter: wordToFreqs) {
      std::string word = iter.first;
      std::vector<uint32_t> freqs = iter.second;

      // Sort the frequencies
      std::sort(freqs.begin(), freqs.end(), [](auto& left, auto& right) {
        return (left < right);
      });

      // And print the file
      out << word << " " << freqs.size();
      for (auto freq: freqs)
        out << " " << freq; 
      out << "\n";
    }
  }
};

#endif // ANALYZE_POET_H
