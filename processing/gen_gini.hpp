#ifndef GEN_GINI_H
#define GEN_GINI_H

class authorGini {
private:
  std::string poet;

public:
  authorGini(std::string poet) : poet(poet) {};
  void save() {
    std::string rCommand = "Rscript helpers/compute_ginis.R " + poet;
    int warning = system(rCommand.data());
  }
};

#endif // GEN_GINI_H
