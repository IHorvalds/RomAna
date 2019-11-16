#ifndef COMPUTE_LOCAL_GINIS_H
#define COMPUTE_LOCAL_GINIS_H

class authorGini {
private:
  std::string poet;

public:
  authorGini(std::string poet) : poet(poet) {};
  void save() {
    std::string rCommand = "Rscript helpers/compute_local_ginis.R " + poet;
    int warning = system(rCommand.data());
    std::string pyCommand = "python3 helpers/compute_local_richness.py " + poet;
    warning = system(pyCommand.data());
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
    call("final", "pure_dev");
    call("final", "norm_dev");
    call("final", "mixed");
    call("normal", "normal");
  }
};

#endif // COMPUTE_LOCAL_GINIS_H
