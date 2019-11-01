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

class authorGolden {
private:
    std::string poet;

    void call(std::string folder, std::string distr) {
        std::string pythonCommand = "python3 helpers/sort_final.py " + poet + " " + folder + " " + distr;
        int warning = system(pythonCommand.data());
    }
public:
  authorGolden(std::string poet) : poet(poet) {};
  void save() {
    std::string rCommand = "Rscript helpers/compute_golden.R " + poet;
    int warning = system(rCommand.data());
    
    call("final", "gini");
    call("final", "idf");
    call("final", "mixed");
    call("normal", "normal");
  }
};

#endif // GEN_GINI_H
