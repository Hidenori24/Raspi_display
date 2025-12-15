#include "domain/Vec3.hpp"
#include <cmath>

namespace domain {

double Vec3::length() const {
  return std::sqrt(x * x + y * y + z * z);
}

Vec3 Vec3::normalized() const {
  double len = length();
  if (len < 1e-10) return Vec3(0, 0, 0);
  return Vec3(x / len, y / len, z / len);
}

} // namespace domain
