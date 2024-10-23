module;

#include <print>

export module utils;

using namespace std;

export void print_time_and_speedup(const char *name, double time,
                                   double min_time) {
  if (time == min_time) {
    println("{} time: {} (1x)", name, time);
  } else {
    println("{} time: {} ({:.2f}x)", name, time, time / min_time);
  }
}
