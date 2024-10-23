#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <list>
#include <print>
#include <vector>

using namespace std;

const int SIZE = 100'000'000;

static_assert(SIZE % 8 == 0, "SIZE must be divisible by 8 for AVX");

int main() {
  vector<int64_t> vec(SIZE);
  for (int64_t i = 0; i < SIZE; ++i)
    vec[i] = i;

  list<int64_t> lst;
  for (int64_t i = 0; i < SIZE; ++i)
    lst.push_back(i);

  // Measure time for vector traversal
  auto start = chrono::high_resolution_clock::now();
  long long vec_sum = 0;
  for (int64_t i = 0; i < SIZE; ++i)
    vec_sum += vec[i];
  auto end = chrono::high_resolution_clock::now();
  chrono::duration<double> vec_time = end - start;

  // Measure time for list traversal
  start = chrono::high_resolution_clock::now();
  long long lst_sum = 0;
  for (const auto &item : lst)
    lst_sum += item;
  end = chrono::high_resolution_clock::now();
  chrono::duration<double> list_time = end - start;

  // Measure time for SIMD vector traversal
  start = chrono::high_resolution_clock::now();
  __m256i simd_sums = _mm256_setzero_si256();
  // simd_sum:               [0, 0, 0, 0,  0, 0, 0, 0]
  // simd_sum 1st iteration: [0, 1, 2, 3,  4, 5, 6, 7]
  // simd_sum 2nd iteration: [8, 9, 10, 11,  12, 13, 14, 15]
  // simd_sum 3rd iteration: [16, 17, 18, 19,  20, 21, 22, 23]
  // ...
  // simd_sum final result:
  // [sum(0,8,16,...), sum(1,9,17,...), sum(2,10,18,...), sum(3,11,19,...),
  // sum(4,12,20,...), sum(5,13,21,...), sum(6,14,22,...), sum(7,15,23,...)]
  for (int64_t i = 0; i < SIZE; i += 4) {
    __m256i vec_values = _mm256_loadu_si256((__m256i *)&vec[i]);
    simd_sums = _mm256_add_epi64(simd_sums, vec_values);
  }

  // NOTE: add is elementwise, hadd is pairwise

  // __m256i hi_sum = _mm256_permute2x128_si256(simd_sums, simd_sums, 1);
  // __m256i sum_1 = _mm256_add_epi32(simd_sums, hi_sum);

  // __m256i sum_2 = _mm256_hadd_epi32(sum_1, sum_1);
  // __m256i sum_3 = _mm256_hadd_epi32(sum_2, sum_2);
  // long long simd_sum =
  //     _mm256_extract_epi32(sum_3, 0) + _mm256_extract_epi32(sum_3, 4);

  // __m256i sum_1 = _mm256_hadd_epi32(simd_sums, simd_sums);
  // __m256i sum_2 = _mm256_hadd_epi32(sum_1, sum_1);
  // __m256i sum_3 = _mm256_hadd_epi32(sum_2, sum_2);

  // long long simd_sum = _mm256_extract_epi32(sum_3, 0);

  // ???
  __m128i sum_high = _mm256_extracti128_si256(simd_sums, 1);
  __m128i sum_low = _mm256_castsi256_si128(simd_sums);
  __m128i sum_128 = _mm_add_epi64(sum_high, sum_low);
  long long simd_sum =
      _mm_extract_epi64(sum_128, 0) + _mm_extract_epi64(sum_128, 1);

  end = chrono::high_resolution_clock::now();
  chrono::duration<double> simd_time = end - start;

  double vec_speedup = vec_time.count() / simd_time.count();
  double list_speedup = list_time.count() / simd_time.count();

  println("SIMD vector traversal time: {} (1x)", simd_time.count());
  println("Vector traversal time: {} ({:.2f}x)", vec_time.count(), vec_speedup);
  println("List traversal time: {} ({:.2f}x)", list_time.count(), list_speedup);
  println("Sum: {} {} {}", simd_sum, vec_sum, lst_sum);
  println("Sums equal: {}", simd_sum == vec_sum && simd_sum == lst_sum);
}
