#pragma once

#include "application/CourseInfo.hpp"

namespace application {

class CourseRepository {
public:
  virtual ~CourseRepository() = default;
  virtual CourseInfo loadHole(int hole_number) const = 0;
};

} // namespace application
