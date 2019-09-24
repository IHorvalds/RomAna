#ifndef POLYNOMIAL_UTIL_H
#define POLYNOMIAL_UTIL_H

#include "../util.hpp"
#include "boost-hdrs/remez.hpp"
#include <cmath>

// !!! Remarks for polynomials structure !!!:
// In std::vector<double> poly, the polynomial function has the form:
// poly[0] + poly[1] * x + poly[2] * x * x etc, exactly like in maths a0 + a1 * x ...
// Also important: degree = poly.size() - 1, so the coefficient for the biggest power is poly[degree]

namespace polynomial_util {
  using Coord = util::Coord;
  
  // for comparisons between floating points
  static constexpr double precision = std::numeric_limits<double>::epsilon();
  
  static double derivate(std::vector<double>& currPoly, double x) 
  // compute currPoly'(x) - derivative of currPoly at point "x"
  {
    uint32_t currPolyDegree = currPoly.size() - 1;
    double ret = currPoly[currPolyDegree] * currPolyDegree;
   
    // Don't consider the coefficent of x^0, since through derivative we get rid of it
    for (unsigned i = currPolyDegree; i != 1; --i)
        ret = ret * x + currPoly[i - 1] * (i - 1);
    return static_cast<double>(ret);
  }
  
  static double hornerEvaluation(std::vector<double>& currPoly, double x)
  // currPoly comes from the Remez's algorithm
  // evaluate the polynomial with Horner's schema
  {
    uint32_t currPolyDegree = currPoly.size() - 1;
    double ret = currPoly[currPolyDegree];
    for (unsigned i = currPolyDegree; i != 0; --i)
        ret = ret * x + currPoly[i - 1];
    return static_cast<double>(ret);
  }
  
  static double interpolate(const double pos)
  // Helper function for defineFunction. Evaluates the spline at pos.
  {
    auto iter = std::lower_bound(util::globalSpline.begin(), util::globalSpline.end(), pos, [](const Coord& a, double b) { return a.first < b; });
    if (iter->first == pos) 
        return iter->second;

    // Compute slope
    double dx = (iter + 0)->first - (iter - 1)->first;
    double dy = (iter + 0)->second - (iter - 1)->second;

    // Compute offset of the linear function
    double ofs = pos - (iter - 1)->first;
    return (iter - 1)->second + ofs * (dy / dx);
  }
  
  static double defineFunction(const double x) 
  // defines for the Remez's algorithm how the spline works through interpolation
  {
    return interpolate(x);
  }
  
  static std::vector<double> fitSpline(std::vector<Coord>& splineToFit, unsigned currPolyDegree)
  // use Remez's algorithm to fit a polynomial to the spline globalSpline
  // Use "long double" to allow a better precision for the algorithm
  {
    // Prepare parameters
    util::globalSpline = splineToFit;
    
    // choose the degree of the polynomial to be fitting
    // the algorithm also has the choice to fit a polynomial fraction, but that's not our case
    unsigned sizeOfNominator = currPolyDegree, sizeOfDenominator = 0;
    
    // limits of x-coordinates
    double a = util::globalSpline.front().first, b = util::globalSpline.back().first;
    
    // pin says: must be the first coefficient zero? 
    bool pin = false;
    
    // Should we optimize for relative error?
    // No. In this idea we use only the chebyshev error.
    bool relError = false;
    
    // Default values for the algorithm
    int skew = 0, workingPrecision = boost::math::tools::digits<long double>() * 2;
    
    // Activate the algorithm with long double to get a better precision. It's recommanded to keep it so. Otherwise it could be the case
    // that an internal function from the algorithm fails because of the poor precision of double
    boost::math::tools::remez_minimax<long double> remez(defineFunction, sizeOfNominator, sizeOfDenominator, a, b, pin, relError, skew, workingPrecision);
    
    // Copy the result and return it
    std::vector<double> polyRet(currPolyDegree + 1);
    for (unsigned index = 0; index <= currPolyDegree; ++index)
      polyRet[index] = static_cast<double>(remez.numerator()[index]);
    return polyRet;
  }
  
  static std::pair<double, double> getPolyFitErrors(std::vector<Coord>& splineToFit, std::vector<double> currPoly)
  // compute the errors of currPoly applied to the spline
  {
    std::pair<double, double> errs = std::make_pair(0, 0);
    for (auto elem: splineToFit) {
      double eval = hornerEvaluation(currPoly, elem.first);
      double error = fabs(eval - elem.second);
      
      // Bigger error found?
      if (error > errs.second)
        errs.second = error;
      errs.first += error;
    }
    errs.first /= splineToFit.size();
    return errs;
  }
  
  static void compareErrors(unsigned& winnerPolyDegree, std::pair<double, double>& bestErrors, unsigned currPolyDegree, std::pair<double, double> 
  freshErrors) 
  // get the best possible error out there and update it
  {
    // Compare the avg
    if (freshErrors.first < bestErrors.first) {
      bestErrors = freshErrors;
      winnerPolyDegree = currPolyDegree;
    // If same avg, choose the best maxError
    } else if ((fabs(freshErrors.first - bestErrors.first) < precision) && (freshErrors.second < bestErrors.second)) {
      bestErrors = freshErrors;
      winnerPolyDegree = currPolyDegree;
    }
  }
  
  static uint32_t findPolyDegree(std::vector<Coord>& splineToFit, const unsigned maximalPolyDegree)
  // returns the polyDegree of the polynomial which best fits the spline
  {
    std::pair<double, double> bestFitErrors = std::make_pair(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
    unsigned winnerPolyDegree = 0;
    
    // Compute the maximum polynomial degree
    // Allow a limit for the degree of the polynomial - the size of spline itself
    unsigned lim = maximalPolyDegree;
    if (splineToFit.size() < lim)
      lim = splineToFit.size();
    for (unsigned currPolyDegree = 1; currPolyDegree <= lim; ++currPolyDegree)
      compareErrors(winnerPolyDegree, bestFitErrors, currPolyDegree, getPolyFitErrors(splineToFit, fitSpline(splineToFit, currPolyDegree)));
    return winnerPolyDegree;
  }
  
  static std::vector<double> getBestFittingPoly(std::vector<Coord>& splineToFit, const unsigned maximalPolyDegree)
  // returns the polynomial which best fits the spline
  {
    return fitSpline(splineToFit,
      findPolyDegree(splineToFit, maximalPolyDegree)
    );
  }
};

#endif //POLYNOMIAL_UTIL_H
  
