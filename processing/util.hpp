#ifndef UTIL_H
#define UTIL_H

class LocalGiniGenerator {
private:
  std::string poet;

public:
  LocalGiniGenerator(std::string poet) : poet(poet) {};
  void save() {
    std::string rCommand = "Rscript helpers/compute_local_ginis.R " + poet;
    int warning = system(rCommand.data());
    std::string pyCommand = "python3 helpers/compute_local_richness.py " + poet;
    warning = system(pyCommand.data());
  }
};

class GoldenGenerator {
private:
   std::string poet;
public:
  GoldenGenerator(std::string poet) : poet(poet) {};
  void save() {
    std::string rCommand = "Rscript helpers/compute_local_weights.R " + poet;
    int warning = system(rCommand.data());
    std::string pyCommand = "python3 helpers/sort_local_distribution.py " + poet + " " + "weight";
    warning = system(pyCommand.data());
  }
};

class SimilarityGenerator {
private:
  std::string poet;
   
  void call(std::string distributionType, bool shouldAppend) {
    std::string rCommand = "Rscript helpers/compute_local_similarity.R " + poet + " " + distributionType + (shouldAppend ? " >>" : " > ") + "poets/local/similarity/" + poet + "_local_similarities.txt";
    int warning = system(rCommand.data());
  }
public:
  SimilarityGenerator(std::string poet) : poet(poet) {};
  void save() {
    // The first frequency should write the file, the others will only append
    call("rank", false);
    call("weight", true);
    call("relative", true);
    call("richness", true);
    call("gini", true);
  }
};

#endif // UTIL_H
