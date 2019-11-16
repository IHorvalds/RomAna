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

#endif // UTIL_H
