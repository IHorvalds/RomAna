#ifndef UTIL_H
#define UTIL_H

#include <string>

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

class EssenceGenerator {
private:
   constexpr static double procentOfWork = 20;
   std::string poet;
   std::string distributionType;
   bool sortValues;
public:
  EssenceGenerator(std::string poet, std::string distributionType, bool sortValues) : poet(poet), distributionType(distributionType), sortValues(sortValues) {};
  void save() {
    std::string pyCommand1 = "python3 helpers/compute_local_essentials.py " + poet + " " + distributionType + " " + std::to_string(procentOfWork);
    int warning = system(pyCommand1.data());
    
    if (sortValues) {
      std::string pyCommand2 = "python3 helpers/sort_local_distribution.py " + poet + " essential";
      warning = system(pyCommand2.data());
    }
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
