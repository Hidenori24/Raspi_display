#pragma once

#include "application/CourseRepository.hpp"
#include <string>

namespace infrastructure {

class FileCourseRepository : public application::CourseRepository {
public:
  explicit FileCourseRepository(std::string path);
  application::CourseInfo loadHole(int hole_number) const override;

private:
  std::string path_;
};

} // namespace infrastructure
