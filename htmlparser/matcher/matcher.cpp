#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <regex>
#include <cassert>
#include <algorithm>
#include <chrono>
#include "lap.cpp"

using namespace std::chrono;

// TODO: update it to substitute diacritics into normal ones with cost of 1
// TODO: update it with adjacent transpositions (O(N) space!), since we need it for USA -> SUA (https://en.wikipedia.org/wiki/Damerau%E2%80%93Levenshtein_distance)
// TODO: in romanian, a name terminated in "a" represents a feminine person. Take that into account
unsigned GeneralizedLevensteinDistance(std::string source, std::string target)
// Efficiently compute the edit distance from "source" to "target"
{
  if (source.size() > target.size())
    return GeneralizedLevensteinDistance(target, source);

  const unsigned min_size = source.size(), max_size = target.size();
  std::vector<unsigned> levDist(min_size + 1);

  levDist[0] = 0;
  for (unsigned i = 1; i <= min_size; ++i)
    levDist[i] = i;

  for (unsigned j = 1; j <= max_size; ++j) {
    unsigned previous_diagonal = levDist[0], previous_diagonal_save;
    levDist[0] += 1;

    for (unsigned i = 1; i <= min_size; ++i) {
      previous_diagonal_save = levDist[i];
      
      if (source[i - 1] == target[j - 1]) {
        levDist[i] = previous_diagonal;
      } else {
        levDist[i] = std::min(std::min(levDist[i - 1] + 1, levDist[i] + 1), previous_diagonal + 1);
      }
      previous_diagonal = previous_diagonal_save;
    }
  }
  return levDist[min_size];
}

typedef std::pair<std::string, unsigned> SU;
typedef std::pair<int, int> PII;
typedef std::vector<SU> VSU;
typedef std::vector<std::string> VS;
typedef std::unordered_map<std::string, VS> tableVS;
typedef std::vector<int> VI;
typedef std::vector<VI> VVI;

// TODO: Words as: "A. Calin" should be instantly equally to "Alexandru Calin"
unsigned computeWeight(VS& xs, VS& ys)
// Compute the weight between the compounds of "x" ("xs") and those of "y" ("ys")
{
  assert(xs.size() <= ys.size());
  
  // Pad with empty words, in order to balance throughput of source and sink
  // This operation is exactly like a blind insertion of a word from "ys" into "xs".
  unsigned padWith = ys.size() - xs.size();
  unsigned size = ys.size();
  VVI cost(size, VI(size));
  for (unsigned i = 0; i != size; ++i) {
    std::string currWord;
    if (i < (size - padWith))
      currWord = xs[i];
    for (unsigned j = 0; j != size; ++j)
      cost[i][j] = GeneralizedLevensteinDistance(currWord, ys[j]);
  }
  
  // Compute the minimum-weighted bipartite-matching
  VI leftMate, rightMate;
  lapSolver<int>(cost, leftMate, rightMate);
  
  // And its result
  // "perfectMatched" tells us if all compounds from "xs" has been paired through an edge of cost 0
  unsigned minCost = 0;
  bool perfectMatched = true;
  for (unsigned i = 0; i != size; ++i) {
    unsigned edgeCost = cost[i][leftMate[i]];
    minCost += edgeCost;
    if (i < (size - padWith))
      perfectMatched &= !edgeCost;
  }
  // Decide if the matching was a perfect matching
  return perfectMatched ? 0 : minCost;
}

unsigned process(VS& xs, VS& ys)
// Wrapper for the computation of weights
{
  if (xs.size() < ys.size())
    return computeWeight(xs, ys);
  else
    return computeWeight(ys, xs);
}

std::unordered_map<std::string, std::string> repl = {{"E2", "â"}, {"EE", "î"}, {"E3", "ă"}, {"FE", "ţ"}, {"BA", "ş"}, {"C3", "Ă"}, {"C2", "Â"}, {"AA", "Ş"}, {"CE", "Î"}, {"DE", "Ţ"}, {"2C", ","}, {"96", "-"}};

std::string refine(std::string str)
// Replace the codes in procent (e.g. "%E2") with the corresponding diacritic
{
  std::string acc;
  for (unsigned index = 0, limit = str.size(); index != limit; ++index) {
    if (str[index] == '%') {
      if (index + 2 < limit) {
        acc += repl[str.substr(index + 1, 2)];
        index += 2;
      }
    } else {
      acc += str.substr(index, 1);
    }
  }
  return acc;
}

VS splitIntoWords(std::string str)
// Split the current string into its compounds
{
  std::regex rgx("[-+,.]+");
  std::sregex_token_iterator iter(str.begin(), str.end(), rgx, -1), end;
  
  VS result;
  for (; iter != end; ++iter)
    result.push_back(*iter);
  return result;
}

uint32_t fileSize(std::string poetRef)
// Return the number of lines of the file owned by "poetRef"
{
  std::string fileName = "../../poets/poetry/" + poetRef + "_poems.txt", line;
  std::ifstream input(fileName);

  if (!input.is_open())
    return 0;
  uint32_t count = 0;
  while (std::getline(input, line))
    ++count;
  return count;
}

void solve(VSU& refs, tableVS& splitted)
// Determine the pairs of names which designates the same poet
{
  // Compute the values of the edges between each pair of different names
  unsigned size = refs.size();
  VVI cost(size, VI(size));
  int maxWeight = 0;
  for (unsigned i = 0; i != size; ++i) {
    for (unsigned j = 0; j != size; ++j) {
      if (i == j) continue;
      cost[i][j] = process(splitted[refs[i].first], splitted[refs[j].first]);
      maxWeight = std::max(maxWeight, cost[i][j]);
    }
  }
  
  // Set the edges between same names as inexistent
  for (unsigned i = 0; i != size; ++i)
    cost[i][i] = 2 * size * maxWeight + 1;
  
  // Compute the minimum-weighted bipartite-matching
  // Note that both partitions represent the set of the names
  VI leftMate, rightMate;
  lapSolver<int>(cost, leftMate, rightMate);
  
  // Gather all the pairs
  std::vector<std::pair<double, std::pair<std::string, std::string>>> corr(size);
  unsigned ptr = 0;
  for (unsigned i = 0; i != size; ++i)
    corr[ptr++] = std::make_pair(cost[i][leftMate[i]], std::make_pair(refs[i].first, refs[leftMate[i]].first));
  
  // And sort them by the weight of the edge between the names of the pair
  std::sort(corr.begin(), corr.end());
  for (auto elem : corr)
    std::cout << elem.first << ": " << elem.second.first << " -> " << elem.second.second << std::endl;
}

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cerr << "Usage: " << argv[0] << " <file with poets>" << std::endl;
    return 0;
  }
#if 1
  // Open the file of poets
  std::string file = argv[1];
  std::ifstream input(file);
  
  // Read all poets and check if the file contains anything
  std::string poet;
  VSU refs;
  tableVS splitted;
  while (input >> poet) {
    unsigned countOfPoems = fileSize(poet);
    if (!countOfPoems)
      continue;
    // Clean up the name of the poet
    std::string refined = refine(poet);
    
    // Save the name along with its number of poems
    refs.push_back(std::make_pair(refined, countOfPoems));
    
    // And split up the name into its compounds
    splitted[refined] = splitIntoWords(refined);
  }
  
  solve(refs, splitted);
#else
  std::string x = "D.H.+Lawrence";
  std::string y = "D.H.+Lawrence,1885+-+1930";
  VS xs, ys;
  xs = splitIntoWords(x);
  ys = splitIntoWords(y);
  std::cerr << process(xs, ys);

#if 0
  std::string tmp("Simona+Petrişor");
  std::cerr << GeneralizedLevensteinDistance("Simona+Petrişor", "Simona+Petrisor");
  for (auto c : tmp) {
    std::cerr << static_cast<int>(c) << " ";
  }
#endif
  
#endif
  return 0;
}
