#include <cassert>
#include <chrono>
#include <immintrin.h>
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

  // [0|0|0|0]
  __m256i simd_sums = _mm256_setzero_si256();

  // process 4 values at once
  for (int64_t i = 0; i < SIZE; i += 4) {
    // load 4 ints
    __m256i vec_values = _mm256_loadu_si256((__m256i *)&vec[i]);
    // elementwise add them to the 4 partial sums
    simd_sums = _mm256_add_epi64(simd_sums, vec_values);
  }

  // NOTE: add is elementwise, hadd is pairwise (reduce operation)

  // simd_sums: [A₆₄ | B₆₄ | C₆₄ | D₆₄]

  // low:  [A₆₄ | B₆₄]
  // high: [C₆₄ | D₆₄]
  __m128i sum_low = _mm256_castsi256_si128(simd_sums);
  __m128i sum_high = _mm256_extracti128_si256(simd_sums, 1);

  // [A₆₄+C₆₄ | B₆₄+D₆₄]
  __m128i sum_128 = _mm_add_epi64(sum_high, sum_low);

  // simd_sum = (A₆₄+C₆₄) + (B₆₄+D₆₄)
  long long simd_sum =
      _mm_extract_epi64(sum_128, 0) + _mm_extract_epi64(sum_128, 1);

  end = chrono::high_resolution_clock::now();
  chrono::duration<double> simd_time = end - start;

  double vec_speedup = vec_time.count() / simd_time.count();
  double list_speedup = list_time.count() / simd_time.count();

  assert(simd_sum == vec_sum && simd_sum == lst_sum && "Sums must be equal");

  println("SIMD vector traversal time: {} (1x)", simd_time.count());
  println("Vector traversal time: {} ({:.2f}x)", vec_time.count(), vec_speedup);
  println("List traversal time: {} ({:.2f}x)", list_time.count(), list_speedup);
}
