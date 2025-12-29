#include "infrastructure/FileCourseRepository.hpp"

#include <fstream>
#include <sstream>

namespace infrastructure {

FileCourseRepository::FileCourseRepository(std::string path)
  : path_(std::move(path)) {}

application::CourseInfo FileCourseRepository::loadHole(int hole_number) const {
  application::CourseInfo info;
  info.hole_number = hole_number;

  std::ifstream file(path_);
  if (!file.is_open()) {
    return info;  // fallback defaults
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;
    std::stringstream ss(line);
    int hole = 0;
    int par = 0;
    double dist = 0.0;
    char comma;
    if (ss >> hole >> comma >> par >> comma >> dist) {
      if (hole == hole_number) {
        if (par > 0) info.par = par;
        if (dist > 0.0) info.pin_distance_m = dist;
        break;
      }
    }
  }

  return info;
}

} // namespace infrastructure
