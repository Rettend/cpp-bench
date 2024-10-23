#include <chrono>
#include <immintrin.h>
#include <iostream>
#include <print>
#include <vector>

using namespace std;

const int SIZE = 1'000'000;
static_assert(SIZE % 8 == 0, "SIZE must be divisible by 8 for AVX");

int main() {
  vector<int64_t> vec(SIZE);
  for (int64_t i = 0; i < SIZE; ++i)
    vec[i] = i;

  // Regular sum for comparison
  long long vec_sum = 0;
  for (int64_t i = 0; i < SIZE; ++i)
    vec_sum += vec[i];

  // SIMD sum
  auto start = chrono::high_resolution_clock::now();

  __m256i simd_sums = _mm256_setzero_si256();
  for (int64_t i = 0; i < SIZE; i += 4) {
    __m256i vec_values = _mm256_loadu_si256((__m256i *)&vec[i]);
    simd_sums = _mm256_add_epi64(simd_sums, vec_values);
  }

  __m128i sum_high = _mm256_extracti128_si256(simd_sums, 1);
  __m128i sum_low = _mm256_castsi256_si128(simd_sums);
  __m128i sum_128 = _mm_add_epi64(sum_high, sum_low);
  long long simd_sum =
      _mm_extract_epi64(sum_128, 0) + _mm_extract_epi64(sum_128, 1);

  auto end = chrono::high_resolution_clock::now();
  chrono::duration<double> simd_time = end - start;

  println("Vector sum: {}", vec_sum);
  println("SIMD sum: {}", simd_sum);
  println("Sum equal: {}", simd_sum == vec_sum);
}
