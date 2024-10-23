#include <chrono>
#include <immintrin.h>
#include <list>
#include <print>
#include <vector>

using namespace std;

const int SIZE = 10'000'000;

static_assert(SIZE % 8 == 0, "SIZE must be divisible by 8 for AVX");

// Custom vector-like class optimized for SIMD operations
class SimdVector {
private:
  int *data;
  size_t size_;
  size_t capacity_;

public:
  SimdVector() : data(nullptr), size_(0), capacity_(0) {}

  ~SimdVector() {
    if (data)
      _mm_free(data);
  }

  void push_back(int value) {
    if (size_ == capacity_) {
      size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
      int *new_data = (int *)_mm_malloc(new_capacity * sizeof(int), 32);
      if (data) {
        memcpy(new_data, data, size_ * sizeof(int));
        _mm_free(data);
      }
      data = new_data;
      capacity_ = new_capacity;
    }
    data[size_++] = value;
  }

  size_t size() const { return size_; }
};

int main() {
  vector<int> vec;
  list<int> lst;
  SimdVector simd_vec;

  // Vector insertion
  auto start = chrono::high_resolution_clock::now();
  for (int i = 0; i < SIZE; ++i) {
    vec.push_back(i);
  }
  auto end = chrono::high_resolution_clock::now();
  chrono::duration<double> vec_time = end - start;

  // List insertion
  start = chrono::high_resolution_clock::now();
  for (int i = 0; i < SIZE; ++i) {
    lst.push_back(i);
  }
  end = chrono::high_resolution_clock::now();
  chrono::duration<double> list_time = end - start;

  // SIMD Vector insertion
  start = chrono::high_resolution_clock::now();
  for (int i = 0; i < SIZE; ++i) {
    simd_vec.push_back(i);
  }
  end = chrono::high_resolution_clock::now();
  chrono::duration<double> simd_time = end - start;

  println("SIMD vector insertion time: {} (1x)", simd_time.count());
  println("Vector insertion time: {} ({:.2f}x)", vec_time.count(),
          vec_time.count() / simd_time.count());
  println("List insertion time: {} ({:.2f}x)", list_time.count(),
          list_time.count() / simd_time.count());

  println("Vector size: {}", vec.size());
  println("List size: {}", lst.size());
  println("SIMD Vector size: {}", simd_vec.size());
}
