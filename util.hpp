#ifndef UTIL_H
#define UTIL_H

#include <vector>

// Class used in the main application (in our case, gui.cpp)
namespace util {
  // For polynomial fitting
  using Coord = std::pair<double, double>;

  // Used by Remez's algorithm
  std::vector<Coord> globalSpline;
};

#endif // UTIL_H
