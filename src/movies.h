#include <iostream>
#include <vector>
#include <ctime>

namespace kiosk {
class Movie {
  public:
  std::string name;
  std::string year;
  std::vector<time_t> timings;

  // void getShowtimings();
};

static std::vector<Movie> getCurrentlyRunningMovies() {
  std::vector<Movie> movies;
  return movies;
}
}