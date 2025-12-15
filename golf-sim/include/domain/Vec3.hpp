#pragma once

namespace domain {

struct Vec3 {
  double x = 0.0;
  double y = 0.0;
  double z = 0.0;
  
  Vec3() = default;
  Vec3(double x_, double y_, double z_) : x(x_), y(y_), z(z_) {}
  
  Vec3 operator+(const Vec3& other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }
  
  Vec3 operator-(const Vec3& other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
  }
  
  Vec3 operator*(double scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
  }
  
  double length() const;
  Vec3 normalized() const;
};

} // namespace domain
