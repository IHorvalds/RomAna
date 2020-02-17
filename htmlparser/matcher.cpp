#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <regex>
#include <cassert>
#include <algorithm>
#include <chrono>

using namespace std::chrono;

// TODO: update it to substitute diacritics into normal ones with cost of 1
// We can do a trick here: call the function with the current best distance to source. Then if in step "i" distance[min_size - i] goes over "current_min", stop it and return "current_min" + 1 
unsigned GeneralizedLevensteinDistance(std::string source, std::string target) {
  if (source.size() > target.size())
    return GeneralizedLevensteinDistance(target, source);

  const unsigned min_size = source.size(), max_size = target.size();
  std::vector<unsigned> levDist(min_size + 1);

  levDist[0] = 0;
  for (unsigned i = 1; i <= min_size; ++i)
    levDist[i] = levDist[i - 1] + 1;

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

// Min cost circulation
// Generic flow using an adjacency matrix.  If you
// want just regular max flow, setting all edge costs to 1 gives
// running time O(|E|^2 |V|).
//
// Running time: O(min(|V|^2 * totflow, |V|^3 * totcost))
//
// INPUT: cap -- a matrix such that cap[i][j] is the capacity of
//               a directed edge from node i to node j
//
//        cost -- a matrix such that cost[i][j] is the (positive)
//                cost of sending one unit of flow along a 
//                directed edge from node i to node j
//
//        excess -- a vector such that the total flow from i == excess[i]
//
//
// OUTPUT: cost of the resulting flow; the matrix flow will contain
//         the actual flow values (all nonnegative).
//         The vector excess will contain node excesses that could not be
//         eliminated.  Remember to check it.
//
// To use this, create a MinCostCirc object, and call it like this:
//
//   MinCostCirc circ(N);
//   circ.cap = <whatever>; circ.cost = <whatever>;
//   circ.excess[foo] = bar;
//   circ.flow[i][j] = something;  (if you want)
//   int finalcost = circ.solve();
//
// If you want min-cost max-flow, leave excess blank and call min_cost_max_flow.
// Andy says to use caution in min-cost max-flow mode if you have negative
// costs.

typedef std::pair<std::string, unsigned> SU;
typedef std::pair<int, int> PII;
typedef std::vector<SU> VSU;
typedef std::vector<std::string> VS;
typedef std::unordered_map<std::string, VS> tableVS;
typedef std::vector<int> VI;
typedef std::vector<VI> VVI;
typedef std::vector<int64_t> VI64;
typedef std::vector<VI64> VVI64;

const int64_t INF = (1LL << 60);

class MinCostCirc {
public:
  int N;
  VVI64 cap, flow, cost;
  VI dad, found, src, add;
  VI64 pi, dist, excess;
  int cnt;
  
  MinCostCirc(int N) : N(N), cap(N, VI64(N)), flow(cap), cost(cap),
		       dad(N), found(N), src(N), add(N),
		       pi(N), dist(N+1), excess(N) {
             cnt = 0;
          }
	       
  void search() {
    std::fill(found.begin(), found.end(), false);
    std::fill(dist.begin(), dist.end(), INF);

    int here = N;
    for (int i = 0; i < N; i++) {
      if (excess[i] > 0) {
        src[i] = i;
        dist[i] = 0;
        here = i;
      }
    }
    while (here != N) {
      int best = N;
      found[here] = 1;
      for (int k = 0; k < N; k++) {
        if (found[k]) continue;
        int64_t x = dist[here] + pi[here] - pi[k];
        if (flow[k][here]) {
          int64_t val = x - cost[k][here];
          assert(val >= dist[here]);
          if (dist[k] > val) {
            dist[k] = val;
            dad[k] = here;
            add[k] = 0;
            src[k] = src[here];
          }
        }
        if (flow[here][k] < cap[here][k]) {
          int64_t val = x + cost[here][k];
          assert(val >= dist[here]);
          if (dist[k] > val) {
            dist[k] = val;
            dad[k] = here;
            add[k] = 1;
            src[k] = src[here];
          }
        }
        if (dist[k] < dist[best])
          best = k;
      }
      here = best;
    }

    for (int k = 0; k < N; k++) {
      if (found[k])
        pi[k] = std::min(pi[k] + dist[k], INF);
    }
  }
    
  int64_t solve() {
    int64_t totcost = 0;
    int source, sink;
    for(int i = 0; i < N; i++) {
      for(int j = 0; j < N; j++) {
        if (cost[i][j] < 0) {
          flow[i][j] += cap[i][j];
          totcost += cost[i][j] * cap[i][j];
          excess[i] -= cap[i][j];
          excess[j] += cap[i][j];
        }
      }
    }
    
    bool again = true;
    while (again) {
      search();
      int64_t amt = INF;
      fill(found.begin(), found.end(), false);
      again = false;
      for(int sink = 0; sink < N; sink++) {
        if ((excess[sink] >= 0) || (dist[sink] == INF) || (found[src[sink]]++))
          continue;
        again = true;
        int source = src[sink];
	  
        for (int x = sink; x != source; x = dad[x])
          amt = std::min(amt, flow[x][dad[x]] ? flow[x][dad[x]] : cap[dad[x]][x] - flow[dad[x]][x]);
        
        amt = std::min(amt, std::min(excess[source], -excess[sink]));
        for (int x = sink; x != source; x = dad[x]) {
          if (add[x]) {
            flow[dad[x]][x] += amt;
            totcost += amt * cost[dad[x]][x];
          } else {
            flow[x][dad[x]] -= amt;
            totcost -= amt * cost[x][dad[x]];
          }
          excess[x] += amt;
          excess[dad[x]] -= amt;
        }
        assert(amt != 0);
        break;  // Comment out at your peril if you need speed.
      }
    }
    return totcost;
  }

  // returns (flow, cost)
  PII min_cost_max_flow(int source, int sink) {
    excess[source] = INF;
    excess[sink] = -INF;
    PII ret;
    ret.second = solve();
    ret.first = INF - excess[source];
    return ret;
  }
};

unsigned computeWeight(VS& xs, VS& ys) {
  assert(xs.size() <= ys.size());
  
  // Pad with empty words, in order to balance throughput of source and sink
  // This operation is exactly like a blind insertion of a word from "ys" into "xs".
  unsigned padWith = ys.size() - xs.size();
  unsigned size = ys.size(), sourceIndex = 0, sinkIndex = 1 + (size << 1);
  MinCostCirc mcmf(sinkIndex + 1);
  for (unsigned i = 0; i != size; ++i) {
    std::string currWord;
    
    // Are we still in the area of non-padded words?
    if (i < (size - padWith))
      currWord = xs[i];
    for (unsigned j = 0; j != size; ++j) {
      mcmf.cap[1 + i][1 + size + j] = 1;
      mcmf.cost[1 + i][1 + size + j] = GeneralizedLevensteinDistance(currWord, ys[j]);
    }
  }
  for (unsigned i = 0; i != size; ++i) {
    mcmf.cap[sourceIndex][1 + i] = 1;
    mcmf.cost[sourceIndex][1 + i] = 0;
  }
  for (unsigned j = 0; j != size; ++j) {
    mcmf.cap[1 + size + j][sinkIndex] = 1;
    mcmf.cost[1 + size + j][sinkIndex] = 0;
  }
  
  PII ret = mcmf.min_cost_max_flow(sourceIndex, sinkIndex);
  
  // First check the edges from the non-padded words
  if (padWith) {
    for (unsigned i = 0; i != (size - padWith); ++i) {
      for (unsigned j = 0; j != size; ++j) {
        if ((mcmf.flow[1 + i][1 + size + j]) && (mcmf.cost[1 + i][1 + size + j]))
          return ret.second;
      }
    }
    return 0;
  }
  return ret.second;
}

std::unordered_map<std::string, std::string> repl = {{"E2", "â"}, {"EE", "î"}, {"E3", "ă"}, {"FE", "ţ"}, {"BA", "ş"}, {"C3", "Ă"}, {"C2", "Â"}, {"AA", "Ş"}, {"CE", "Î"}, {"DE", "Ţ"}, {"2C", ","}, {"96", "-"}};

std::string refine(std::string str) {
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

VS splitIntoWords(std::string str) {
  std::regex rgx("[-+,.]+");
  std::sregex_token_iterator iter(str.begin(), str.end(), rgx, -1), end;
  
  VS result;
  for (; iter != end; ++iter)
    result.push_back(*iter);
  return result;
}

unsigned process(VS& xs, VS& ys) {
  if (xs.size() < ys.size())
    return computeWeight(xs, ys);
  else
    return computeWeight(ys, xs);
}

uint32_t fileSize(std::string poetRef) {
  std::string fileName = "../poets/poetry/" + poetRef + "_poems.txt", line;
  std::ifstream input(fileName);

  if (!input.is_open())
    return 0;
  uint32_t count = 0;
  while (std::getline(input, line))
    ++count;
  return count;
}

void solve(VSU& refs, tableVS& splitted) {
  unsigned size = refs.size(), sourceIndex = 0, sinkIndex = 1 + (size << 1);
  MinCostCirc mwbm(sinkIndex + 1);
  
  std::cout << "Init the network" << std::endl;
  auto initStart = high_resolution_clock::now();
  for (unsigned i = 0; i != size; ++i) {
    for (unsigned j = 0; j != size; ++j) {
      if (i == j) continue;
      mwbm.cap[1 + i][1 + size + j] = 1;
      mwbm.cost[1 + i][1 + size + j] = process(splitted[refs[i].first], splitted[refs[j].first]);
    }
  }
  for (unsigned i = 0; i != size; ++i) {
    mwbm.cap[sourceIndex][1 + i] = mwbm.cap[1 + size + i][sinkIndex] = 1;
    mwbm.cost[sourceIndex][1 + i] = mwbm.cost[1 + size + i][sinkIndex] = 0;
  }
  auto initStop = high_resolution_clock::now();
  std::cout << "Init the network took: " << duration_cast<milliseconds>(initStop - initStart).count() << " ms" << std::endl;
  
  std::cout << "Run MinCost-MaxFlow" << std::endl;
  auto runStart = high_resolution_clock::now();
  PII result = mwbm.min_cost_max_flow(sourceIndex, sinkIndex);
  auto runStop = high_resolution_clock::now();
  std::cout << "Run MinCost-MaxFlow took: " << duration_cast<milliseconds>(runStop - runStart).count() << " ms" << std::endl;
 
  std::cout << "Find correspondances" << std::endl;
  auto corrStart = high_resolution_clock::now();
 
  // TODO: search with SIMD
  std::vector<std::pair<double, std::pair<std::string, std::string>>> corr(size);
  unsigned ptr = 0;
  for (unsigned i = 0; i != size; ++i) {
    for (unsigned j = 0; j != size; ++j) {
      if (mwbm.flow[1 + i][1 + size + j]) {
        corr[ptr++] = std::make_pair(mwbm.cost[1 + i][1 + size + j], std::make_pair(refs[i].first, refs[j].first));
        break;
      }
    }
  }
  auto corrStop = high_resolution_clock::now();
  std::cout << "Find correspondances took: " << duration_cast<milliseconds>(corrStop - corrStart).count() << " ms" << std::endl;
 
  std::sort(corr.begin(), corr.end());
  for (auto elem : corr) {
    std::cout << elem.first << ": " << elem.second.first << " -> " << elem.second.second << std::endl;
  }
}

int main(int argc, char** argv) {
  if (argc == 1) {
    std::cerr << "Usage: " << argv[0] << " <file with poets>" << std::endl;
    return 0;
  }
  
#if 1
  std::string file = argv[1];
  std::ifstream input(file);
  
  std::cout << "Start checking the poets" << std::endl;
  std::string poet;
  VSU refs;
  tableVS splitted;
  while (input >> poet) {
    unsigned countOfPoems = fileSize(poet);
    if (!countOfPoems)
      continue;
    std::string refined = refine(poet);
    refs.push_back(std::make_pair(refined, countOfPoems));
    splitted[refined] = splitIntoWords(refined);
  }
  
  solve(refs, splitted);
#else
  std::string x = "D.H.+Lawrence";
  std::string y = "D.H.+Lawrence,1885+-+1930";
  VS xs, ys;
  xs = splitIntoWords(x);
  ys = splitIntoWords(y);
  process(xs, ys);
  
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
