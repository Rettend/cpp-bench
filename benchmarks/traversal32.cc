import utils;

#include <algorithm>
#include <cassert>
#include <chrono>
#include <immintrin.h>
#include <list>
#include <print>
#include <tuple>
#include <vector>

using namespace std;

const int SIZE = 10'000;
static_assert(SIZE % 8 == 0, "SIZE must be divisible by 8 for AVX");

const int ITERS = 10'000;
const int WARMUP_ITERS = 2;

template <typename F>
chrono::duration<double> benchmark(F f, int warmup_iters = WARMUP_ITERS,
                                   int iters = ITERS) {
  for (int i = 0; i < warmup_iters; ++i)
    f();
  auto start = chrono::high_resolution_clock::now();
  for (int i = 0; i < iters; ++i)
    f();
  auto end = chrono::high_resolution_clock::now();
  return (end - start) / iters;
}

int run_vec() {
  vector<int> vec(SIZE);
  for (int i = 0; i < SIZE; ++i)
    vec[i] = i;

  int vec_sum = 0;
  for (int i = 0; i < SIZE; ++i)
    vec_sum += vec[i];

  return vec_sum;
}

int run_list() {
  list<int> lst;
  for (int i = 0; i < SIZE; ++i)
    lst.push_back(i);

  int lst_sum = 0;
  for (auto it = lst.begin(); it != lst.end(); ++it)
    lst_sum += *it;

  return lst_sum;
}

int run_simd() {
  alignas(32) vector<int> vec(SIZE);

  for (int i = 0; i < SIZE; i += 8) {
    __m256i indices =
        _mm256_set_epi32(i + 7, i + 6, i + 5, i + 4, i + 3, i + 2, i + 1, i);
    _mm256_store_si256((__m256i *)&vec[i], indices);
  }

  // [0|0|0|0|0|0|0|0]
  __m256i simd_sums = _mm256_setzero_si256();

  // simd_sums:               [0, 0, 0, 0,  0, 0, 0, 0]
  // simd_sums 1st iteration: [0, 1, 2, 3,  4, 5, 6, 7]
  // simd_sums 2nd iteration: [8, 9, 10, 11,  12, 13, 14, 15]
  // simd_sums 3rd iteration: [16, 17, 18, 19,  20, 21, 22, 23]
  // ...
  // simd_sums final result:
  // [sum(0,8,16,...), sum(1,9,17,...), sum(2,10,18,...), sum(3,11,19,...),
  // sum(4,12,20,...), sum(5,13,21,...), sum(6,14,22,...), sum(7,15,23,...)]
  for (int i = 0; i < SIZE; i += 8) {
    __m256i vec_values = _mm256_load_si256((__m256i *)&vec[i]);
    simd_sums = _mm256_add_epi32(simd_sums, vec_values);
  }

  // simd_sums: [A₃₂|B₃₂|C₃₂|D₃₂|E₃₂|F₃₂|G₃₂|H₃₂]

  // low:  [A₃₂|B₃₂|C₃₂|D₃₂]
  // high: [E₃₂|F₃₂|G₃₂|H₃₂]
  __m128i sum_high = _mm256_extracti128_si256(simd_sums, 1);
  __m128i sum_low = _mm256_castsi256_si128(simd_sums);

  // [A₃₂+E₃₂|B₃₂+F₃₂
  // |C₃₂+G₃₂|D₃₂+H₃₂]
  __m128i sum_128 = _mm_add_epi32(sum_high, sum_low);

  // (A₃₂+E₃₂)+(B₃₂+F₃₂) | (C₃₂+G₃₂)+(D₃₂+H₃₂)
  sum_128 = _mm_hadd_epi32(sum_128, sum_128);

  // ((A₃₂+E₃₂)+(B₃₂+F₃₂)) + ((C₃₂+G₃₂)+(D₃₂+H₃₂))
  sum_128 = _mm_hadd_epi32(sum_128, sum_128);

  // convert scalar int128 to int32
  auto simd_sum = _mm_cvtsi128_si32(sum_128);

  return simd_sum;
}

constexpr int run_comptime() {
  int sum = 0;
  for (int i = 0; i < SIZE; ++i)
    sum += i;

  return sum;
}

int main() {
  // Measure time for vector traversal
  long long vec_sum = run_vec();
  auto vec_time = benchmark([&] { run_vec(); });

  // Measure time for list traversal
  long long lst_sum = run_list();
  auto list_time = benchmark([&] { run_list(); });

  // Measure time for SIMD vector traversal
  long long simd_sum = run_simd();
  auto simd_time = benchmark([&] { run_simd(); });

  // Measure time for compile time sum
  long long comptime_sum = run_comptime();
  auto comptime_time = benchmark([&] { run_comptime(); });

  auto times = std::make_tuple("Compile time sum", comptime_time.count(),
                               "SIMD vector traversal", simd_time.count(),
                               "Vector traversal", vec_time.count(),
                               "List traversal", list_time.count());

  auto min_time = std::min({vec_time.count(), list_time.count(),
                            simd_time.count(), comptime_time.count()});

  print_time_and_speedup("Compile time sum", comptime_time.count(), min_time);
  print_time_and_speedup("SIMD vector traversal", simd_time.count(), min_time);
  print_time_and_speedup("Vector traversal", vec_time.count(), min_time);
  print_time_and_speedup("List traversal", list_time.count(), min_time);
}
