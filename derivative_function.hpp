#ifndef DERIVATIVE_FUNCTION_H
#define DERIVATIVE_FUNCTION_H

#include "util.hpp"
#include "polynomial/polynomial_util.hpp"
#include <string>
#include <vector>

namespace derivative {
  using Coord = util::Coord;
  static constexpr double EXPAND_COORD_X = 1e3; // tuning-parameter for boost errors
  static constexpr double EXPAND_COORD_Y = 1e4; // tuning-parameter for boost errors

  static std::vector<std::pair<double, std::string>> getDerivatives(std::set<std::pair<double, std::string>>& freqToPoet)
  // Receives the normalized set of pairs (frequency, poet)
  // and returns the values of derivatives of each poet
  {
    std::vector<Coord> spline;
    
    // Expand the coordinates when inserting
    auto addToSpline = [&spline](double x, double y) { spline.push_back(std::make_pair(x * EXPAND_COORD_X, y * EXPAND_COORD_Y)); };
    
    unsigned countOfPoets = freqToPoet.size();
    double init = 1.0 / countOfPoets, curr = 0, currSum = 0;
    
    // Create the spline, which begins with (0, 0) and ends in (1, 1) - which is the last author
    addToSpline(0, 0);
    for (auto elem: freqToPoet) {
      curr += init;
      currSum += elem.first;
      addToSpline(curr, currSum);
    }
    
    // Don't exceed degree 16. After this degree, it could be possible that a segFault occurs.
    const double maxDegreeAccepted = 16;
    std::vector<double> poly = polynomial_util::getBestFittingPoly(spline, maxDegreeAccepted);

    // Ptr shows the current position in spline. For that, skip the first position, which contains (0, 0)
    unsigned ptr = 0;
    std::vector<std::pair<double, std::string>> derivativeToPoet;
    for (auto elem: freqToPoet) {
      double x_coordinate = spline[++ptr].first;
      derivativeToPoet.push_back(std::make_pair(polynomial_util::derivate(poly, x_coordinate), elem.second));
    }
    
#if 0
    // Compute errors and derivatives
    double avgError = 0;
    for (auto coord: spline) {
      double x = coord.first;
      double real = coord.second;
      double eval = polynomial_util::hornerEvaluation(poly, x);
      
      double error = real - eval;
      if (error < 0)
        error = -error;
      avgError += error;
      
      std::cout << "For " << x << " error = " << error << " and p'(" << x << ") = "<< polynomial_util::derivate(poly, x) << std::endl;
    }
    avgError /= spline.size();
    std::cout << "Degree = " << poly.size() - 1 << " & Fitting Error = " << avgError << std::endl;
#endif
    return derivativeToPoet;
  }
};

#endif // DERIVATIVE_FUNCTION_H
