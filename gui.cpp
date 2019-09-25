#include <iostream>
#include <cstdlib>
#include <cassert>
#include <fstream>
#include <algorithm>
#include <set>

#include "util.hpp"
#include "derivative_function.hpp"
#include "trie.cpp"
#include "parserOltean.hpp"
#include "analyze_poet.hpp"

using namespace std;
using Coord = util::Coord;

#define BUILD_TRIE 0
#define PROCESS_TEXT 1
#define UPDATE_DICT 2
#define PROCESS_POET 3

void errorArgs(int32_t option, int32_t argc, int32_t expected) {
  if (argc != expected) {
    std::cout << "You have chosen the option " << option << ", but the number of args is false! There should be " << expected << " args " << std::endl;
    exit(0);
  }
}

// TODO: Keep them for debug
#if 0
void testFittingWithoutSums() {
  // Test sample spline with poets
  std::vector<Coord> spline;
  auto addToSpline = [&spline](double x, double y) { spline.push_back(std::make_pair(x, y)); };
  
  std::ifstream file("input_spline.in");
  std::vector<double> ys;
  double y;
  
  while (file >> y)
    ys.push_back(y * 1000000);
  file.close();
  
  std::sort(ys.begin(), ys.end());
  
  double init = 100000, curr = 0;
  for (auto y: ys) {
    curr += init;
    addToSpline(curr, y);
  }
  
#if 0
  // Vorsicht! The spline should be sorted by x-coordinates!
  addToSpline(0.1, 0.1);
  addToSpline(0.2, 0.5);
  addToSpline(0.3, 0.65);
  addToSpline(0.6, 0.80);
  addToSpline(0.9, 0.87);
  addToSpline(1.0, 1);
#endif
  
  // Don't exceed 16. After this degree, it could be possible that a SegFault occurs.
  const double maxDegreeAccepted = 16;
  std::vector<double> poly = polynomial_util::getBestFittingPoly(spline, maxDegreeAccepted);
 
  unsigned pow = 0;
  for (auto elem: poly) {
    std::cout << elem << " * x^" << (pow++) << "+ ";
  }
  std::cout << std::endl;
  
  
  // Compute errors and derivatives
  double avgError = 0;
  for (auto coord: spline) {
    double x = coord.first;
    double real = coord.second;
    double eval = polynomial_util::hornerEvaluation(poly, x);
    
    double error = real - eval;
    if (error < 0)
      error = -error;
    avgError += error;
    
    std::cout << "For " << x << " error = " << error << " and p'(" << x << ") = "<< polynomial_util::derivate(poly, x) << std::endl;
  }
  avgError /= spline.size();
  std::cout << "Degree = " << poly.size() - 1 << " & Fitting Error = " << avgError << std::endl;
}

void testFittingWithSums() {
  // Test sample spline with poets
  std::vector<Coord> spline;
  auto addToSpline = [&spline](double x, double y) { spline.push_back(std::make_pair(x, y)); };
  
  std::ifstream file("input_spline.in");
  std::vector<double> ys;
  double y, total = 0;
  while (file >> y) {
    ys.push_back(y);
  }
  file.close();
  
  std::sort(ys.begin(), ys.end());
  
  double init = 1 / ys.size(), curr = 0, currSum = 0;
  for (auto y: ys) {
    curr += init;
    currSum += y;
    addToSpline(curr, currSum);
  }
  
#if 0
  // Vorsicht! The spline should be sorted by x-coordinates!
  addToSpline(0.1, 0.1);
  addToSpline(0.2, 0.5);
  addToSpline(0.3, 0.65);
  addToSpline(0.6, 0.80);
  addToSpline(0.9, 0.87);
  addToSpline(1.0, 1);
#endif
  
  // Don't exceed 16. After this degree, it could be possible that a SegFault occurs.
  const double maxDegreeAccepted = 16;
  std::vector<double> poly = polynomial_util::getBestFittingPoly(spline, maxDegreeAccepted);
  
  unsigned pow = 0;
  for (auto elem: poly) {
    if (elem < 0) {
      std::cout << "-" << -elem << " * x^" << (pow++) 
    }
    std::cout << elem << " * x^" << (pow++) << "+ ";
  }
  std::cout << std::endl;
  
  // Compute errors and derivatives
  double avgError = 0;
  for (auto coord: spline) {
    double x = coord.first;
    double real = coord.second;
    double eval = polynomial_util::hornerEvaluation(poly, x);
    
    double error = real - eval;
    if (error < 0)
      error = -error;
    avgError += error;
    
    std::cout << "For " << x << " error = " << error << " and p'(" << x << ") = "<< polynomial_util::derivate(poly, x) << std::endl;
  }
  avgError /= spline.size();
  std::cout << "Degree = " << poly.size() - 1 << " & Fitting Error = " << avgError << std::endl;
}
#endif

void testDerivative() {
  std::ifstream gini_file("input_spline.in");
  std::ifstream poets_file("poets.txt");
  
  std::set<std::pair<double, std::string>> freqToPoet;
  double gini;
  std::string poet;
  while (gini_file >> gini) {
    poets_file >> poet;
    freqToPoet.insert(std::make_pair(gini, poet));
  }
  
  std::vector<std::pair<double, std::string>> derivativeToPoet = derivative::getDerivatives(freqToPoet);
  for (auto elem: derivativeToPoet) {
    std::cerr << elem.second << " -> " << elem.first << std::endl;
  }
}

void testPolynomial() {
  std::vector<double> poly = {1, 4, -1, 3};
  assert(polynomial_util::derivate(poly, 5) == 219);
}

void dictionaryTask(trie& dict, char* textName) {
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

int main(int argc, char** argv) {
  if (argc < 3) {
    // For Alex: Debug -> Test polynomial derivative - Insert: ./gui -1
    if ((argc == 2) && (atoi(argv[1]) == -1)) {
      testDerivative();
      return 0;
    }
    
    std::cout << "The following options are available: 0(BUILD_TRIE) or 1 (PROCESS_TEXT) or 2 (UPDATE_DICT) or 3 (PROCESS_POET)]" << std::endl;
    std::cout << "You should insert one of the following schemas: " << std::endl;
    std::cout << "BUILD_TRIE: " << argv[0] << " 0 [file to read the inflections from] [file in which to save the dictionary]" << std::endl;
    std::cout << "PROCESS_TEXT: " << argv[0] << " 1 [dictionary file name] [input file name] [output file name]" << std::endl;
    std::cout << "UPDATE_DICT: " << argv[0] << " 2 [dictionary file name] [input file name] [output file name] [new dictionary file name]" << std::endl;
    std::cout << "PROCESS_POET: " << argv[0] << " 3 [poet name from poets.txt]" << std::endl;
    return -1;
  }
  
  uint32_t option = atoi(argv[1]);
  switch (option) {
    case BUILD_TRIE : {
      errorArgs(BUILD_TRIE, argc, 4); 
      
      const char* file_name = argv[2];
      const char* save_into = argv[3];
      
      // Create the latin_file with python
      std::string tmp = file_name;
      std::string pythonCommand = "python3 convert_into_latin.py " + tmp;
      int warning = system(pythonCommand.data());
      tmp = "latin_" + tmp;
      const char* latin_file_name = tmp.data();
      
      // Alloc the dictionary
      trie A(true);
      
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
      std::string dictName = argv[2];
      trie dict(dictName.data());
      dictionaryTask(dict, argv[3]);
      
      // Print the frequencies obtained
      dict.showFreqs(argv[4]);
      break;
    }
    case UPDATE_DICT : {
      errorArgs(UPDATE_DICT, argc, 6); 
      
      // Use the trie while parsing the text
      std::string dictName = argv[2];
      trie dict(dictName.data()); 
      dictionaryTask(dict, argv[3]);
      
      // Print the frequencies obtained
      dict.showFreqs(argv[4]);
    
      // Save the new obtained dictionary
      dict.saveExternal(argv[5]);
      break;
    }
    case PROCESS_POET : {
      errorArgs(PROCESS_POET, argc, 3);
      
      std::string poetName = argv[2];  
      PoetAnalyzer analyzer(poetName);
      analyzer.saveFrequencies();
      break;
    }
    default : {
      std::cout << "The option " << argv[1] << " is not (yet) available!" << std::endl;
    }
  }
  return 0;
}
