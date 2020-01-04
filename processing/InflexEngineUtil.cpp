#include <vector>
#include <algorithm>
#include <iostream>
#include <string>
#include <cassert>
#include <cstdlib>
#include "InflexEngine.hpp"

// Constructor which covers all cases of generationMode
InflexEngine::InflexEngine(Mode generationMode, const char* filename, const char* latinFilename) : generationMode(generationMode) {
  // Initialize the auxiliar InflexEngine
  auxTrieSize = 0, auxTrie = nullptr;
  
  switch (generationMode) {
    // Simple initialization
    case Read : {
      // Necessary when deallocating
      bufferLastPtr = 0;
      break;
    }
    // Load from external binary file
    case Load : {
      loadExternal(filename);
      break;
    }
    // Build up the InflexEngine from the file with inflections
    case Build : {
      // Compute properties of the namespace
      size = SIZE;
      sigma = romanian::ROMANIAN_SIGMA;
      assert(sigma <= MAX_ACCEPTED_SIGMA);
      
      // Pay attention to the right shift when sigma == MAX_ACCEPTED_SIGMA
      fullBits = (sigma == MAX_ACCEPTED_SIGMA) ? 0xffffffff : ((1u << sigma) - 1);
      
      // Alloc staticTrie
      bufferLastPtr = 0;
      staticTrie = new InflexEngineNode[size];
      for (unsigned index = 0; index < size; index++) {
        staticTrie[index].code = 0;
        staticTrie[index].parent = 0;
        staticTrie[index].sons = new uint32_t[sigma]();
        staticTrie[index].configuration = fullBits;
      }
      
      // And consume the inflexions
      consumeInflexions(filename, latinFilename);
      break;
    }
    default : {
      std::cerr << "Mode of InflexEngine does not (yet) exist." << std::endl;
    }
  }
}

InflexEngine::~InflexEngine() {
  dealloc_();
}

void InflexEngine::dealloc_() {
  // Dealloc staticTrie
  for (unsigned i = 0; i <= bufferLastPtr; i++) {
    if (staticTrieAccess(i)->sons != nullptr)
      delete[] staticTrieAccess(i)->sons;
  }
  if (bufferLastPtr)
    delete[] staticTrie;

  // Free auxTrie, only if it has been allocated
  if (auxTrieSize)
    free(auxTrie);
}

void InflexEngine::reset(const char* filename) {
// Reset the InflexEngine with a new dictionary
  // First dealloc the trie
  dealloc_();
  // Set the generation mode
  generationMode = Load;
  
  // Initialize auxTrie
  auxTrieSize = 0, auxTrie = nullptr;
  loadExternal(filename);
}

uint32_t InflexEngine::getSize() {
// Return the current size of the InflexEngine
  return bufferLastPtr + 1;
}

void InflexEngine::addRoot(std::string str) {
  int32_t dummy;
  insert(ROOT, str, -1, 0, dummy);
}

void InflexEngine::addDerivated(std::string root, std::string derivated) {
  if (derivated == root)
    return;
  int32_t dummy, root_pointer = search(root, dummy);
  insert(ROOT, derivated, root_pointer, 0, dummy);
}

void InflexEngine::consumeInflexions(const char* filename, const char* latinFilename) {
  std::ifstream infile(filename);
  std::ifstream latinInfile(latinFilename);
   
  // Read both variants
  unsigned latinTotalCountOfRoots, totalCountOfRoots;
  infile >> totalCountOfRoots;
  latinInfile >> latinTotalCountOfRoots;
  
  // Compare for equality of count of words
  assert(latinTotalCountOfRoots == totalCountOfRoots);
  
  // Function to extract the inflexions from each line
  auto extractInflexionsFromLine = [&](std::string& line) -> std::vector<std::string> {
    const std::string delimiter = ",";
    std::vector<std::string> inflexions;
    size_t pos = 0;
    while ((pos = line.find(delimiter)) != std::string::npos) {
      inflexions.push_back(line.substr(0, pos));
      line.erase(0, pos + delimiter.length());
    }
    // If the root-word is alone in the list
    if (inflexions.empty())
      inflexions.push_back(line);
    return inflexions;
  };
   
  // Personal getline
  auto __getline = [&](std::ifstream& file, std::string& result) {
    result.clear();
    char c;
    while ((file.get(c)) && (c != '\n'))
      result += c;
  };
  
  // Jump over the newline character
  std::string line, latinLine;
  __getline(infile, line);
  __getline(latinInfile, latinLine);
  
  // Read both files
  while (totalCountOfRoots--) {
    __getline(infile, line);
    __getline(latinInfile, latinLine);
    
    // Extract the inflexions from each line
    std::vector<std::string> inflexions = extractInflexionsFromLine(line), latinInflexions = extractInflexionsFromLine(latinLine);
    assert(inflexions.size() == latinInflexions.size());
    
    std::string root = specialChars::cleanUpWord(inflexions.front()), latinRoot = specialChars::cleanUpWord(latinInflexions.front());
    
    // Check if the root is indeed a romanian one
    bool isInLanguage = romanian::isRomanian(root);
    addRoot(isInLanguage ? root : latinRoot);
    
    // Iterate over the inflexions
    for (unsigned index = 1, limit = inflexions.size(); index < limit; ++index) {
      std::string inflexion = specialChars::cleanUpWord(inflexions[index]), latinInflexion = specialChars::cleanUpWord(latinInflexions[index]);
      
      // Does the root-word have characters outside the language?
      if (!isInLanguage) {
        // It could be the case that some of the inflexions do have all their characters in the language
        if (romanian::isRomanian(inflexion)) {
          addDerivated(latinRoot, inflexion);
        } else {
          addDerivated(latinRoot, latinInflexion);
        }
      } else {
        // This means, the root-word contains only characters from the language. Thus we are sure that the inflexion also does.
        addDerivated(root, inflexion);
      }
    }
  }
  infile.close();
  latinInfile.close();
}

void InflexEngine::loadExternal(const char* filename) {
// Load class members from external binary file
  std::ifstream in;
  in.open(filename, std::ios::in | std::ios::binary);

  in.read((char*) &size,       sizeof(uint32_t));
  in.read((char*) &sigma,      sizeof(uint32_t));
  in.read((char*) &fullBits,  sizeof(uint32_t));

  bufferLastPtr = size - 1;
  staticTrie = new InflexEngineNode[size];
  for (unsigned i = 0; i < size; i++) {
    in.read((char*) &staticTrie[i].code,          sizeof(uint32_t));
    in.read((char*) &staticTrie[i].configuration, sizeof(uint32_t));
    in.read((char*) &staticTrie[i].parent,        sizeof(uint32_t));
    unsigned count = bitOp::countOfBits(staticTrie[i].configuration);
    if (count) {
      staticTrie[i].sons = new uint32_t[count];
      in.read((char*) staticTrie[i].sons, count * sizeof(uint32_t));
    }
  }
}

void InflexEngine::saveExternal(const char* filename) {
// Save class members to external binary file
  std::ofstream out;
  out.open(filename, std::ios::out | std::ios::binary);
  
  // "+ 1" because "bufferLastPtr" points on the last used pointer
  uint32_t writeSize = bufferLastPtr + 1;
  out.write((char*) &writeSize,  sizeof(uint32_t));
  out.write((char*) &sigma,      sizeof(uint32_t));
  out.write((char*) &fullBits,  sizeof(uint32_t));
  
  for (unsigned i = 0; i < writeSize; i++) {
    unsigned countOfSons = bitOp::countOfBits(staticTrieAccess(i)->configuration);
    
    // Computes configuration (the sons with non-zero values).
    if (staticTrieAccess(i)->configuration == fullBits) {
      staticTrieAccess(i)->configuration = 0;
      for (unsigned j = 0; j < sigma; j++)
        if (staticTrieAccess(i)->sons[j])
          staticTrieAccess(i)->configuration |= (1u << j);
    }
    out.write((char*) &staticTrieAccess(i)->code,          sizeof(uint32_t));
    out.write((char*) &staticTrieAccess(i)->configuration, sizeof(uint32_t));
    out.write((char*) &staticTrieAccess(i)->parent,        sizeof(uint32_t));

    // Writes only the sons with non-zero values.
    for (unsigned j = 0; j < countOfSons; j++)
      if (staticTrieAccess(i)->sons[j])
        out.write((char*) &staticTrieAccess(i)->sons[j], sizeof(uint32_t));
  }
  out.close();
}

std::vector<std::pair<uint32_t, std::string>> InflexEngine::getFrequencies() {
  std::vector<std::pair<uint32_t, std::string>> dummy;
  getAllFreqs(ROOT, dummy);
  return dummy;
}

void InflexEngine::getAllFreqs(uint32_t root, std::vector<std::pair<uint32_t, std::string>>& initVector) {
  // Push the frequence of from this node
  if ((staticTrieAccess(root)->code & 1) && (staticTrieAccess(root)->code >> 1))
    initVector.push_back(make_pair(staticTrieAccess(root)->code >> 1, formWord(root)));
  
  // And go further with the children
  for (unsigned i = 0; i < bitOp::countOfBits(staticTrieAccess(root)->configuration); i++)
    if (staticTrieAccess(root)->sons[i] != 0)
      getAllFreqs(staticTrieAccess(root)->sons[i], initVector);
}

void InflexEngine::showFreqs(std::string filename) {
  std::ofstream out(filename);

  // Get all frequencies from trie
  std::vector<std::pair<uint32_t, std::string>> init_vector;
  getAllFreqs(ROOT, init_vector);

  // Sort the accumulated frequencies. Where equality between frequencies, prefer lexicografically order on words
  std::sort(init_vector.begin(), init_vector.end(), [](auto& left, auto& right) {
    return (left.first > right.first) || ((left.first == right.first) && (left.second < right.second));
  });

  // Compute the sum of all frequencies
  double sumOfFreqs = 0;
  for (auto iter = init_vector.begin(); iter != init_vector.end(); ++iter)
    sumOfFreqs += iter->first;

  // And print them as word - (frequency, ratio)
  for (auto iter = init_vector.begin(); iter != init_vector.end(); ++iter)
    out << std::setprecision(3) << iter->second << " (" << iter->first << ", " << (iter->first / sumOfFreqs) * 100 << "%)" << std::endl;
}

#if 0
void InflexEngine::compressionTest(int root, int current, int& max, int& avg, int last, int& a, int& b) {
  if (bitOp::countOfBits(staticTrieAccess(root)->configuration) == 1) {
    if (!current)
      last = root;
    if (++current > max) {
      max = current;
      b = root;
      a = last;
    }
    if (last != root)
      avg++;
    compressionTest(staticTrieAccess(root)->sons[0], current, max, avg, last, a, b);
    return;
  }
  for (auto i = 0; i < countOfBits(staticTrieAccess(root)->configuration); i++) {
    compressionTest(staticTrieAccess(root)->sons[i], 0, max, avg, root, a, b);
  }
}
#endif
