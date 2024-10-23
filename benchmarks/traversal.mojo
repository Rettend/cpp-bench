from time import perf_counter
from testing import assert_true
from algorithm.functional import vectorize
from sys import simdwidthof
import benchmark

alias SIZE = 10_000
var cpp_speed = 0.000116879


fn run():
    var vec = List[Int](capacity=SIZE)

    for i in range(SIZE):
        vec.append(i)

    var list_sum: Int = 0

    for i in range(SIZE):
        list_sum += vec[i]


fn run_unroll():
    var vec = List[Int](capacity=SIZE)

    @parameter
    for i in range(SIZE):
        vec.append(i)

    var list_sum: Int = 0

    @parameter
    for i in range(SIZE):
        list_sum += vec[i]


alias simd_width = simdwidthof[DType.int64]()


fn run_vectorize():
    var p = UnsafePointer[Int64].alloc(SIZE)

    @parameter
    fn store[wd: Int](i: Int):
        p.store(i, i)

    vectorize[store, simd_width](SIZE)

    var list_sum = SIMD[DType.int64, simd_width]()

    @parameter
    fn sum[wd: Int](i: Int):
        # wd is simd_width but the types are wrong
        list_sum += p.load[width=simd_width](i)

    vectorize[sum, simd_width](SIZE)


fn main() raises:
    assert_true(SIZE % simd_width == 0, "SIZE must be a multiple of simd_width")

    var report = benchmark.run[run]()
    var report_unroll = benchmark.run[run_unroll]()
    var report_vectorize = benchmark.run[run_vectorize]()
    var sum = _run()
    var sum_vectorize = _run_vectorize()

    assert_true(sum == sum_vectorize, "Sums do not match")

    print(String("List traversal time: {}").format(report.mean() * 10_000))
    print(
        String("Unrolled list traversal time: {}").format(
            report_unroll.mean() * 10_000
        )
    )
    print(
        String("Vectorized list traversal time: {}").format(
            report_vectorize.mean() * 10_000
        )
    )

    print(
        String("Runs: {}, {}").format(report.iters(), report_vectorize.iters())
    )
    print(String("List speedup vs cpp: {}").format(report.mean() / cpp_speed))
    print(
        String("Unrolled list speedup vs cpp: {}").format(
            cpp_speed / report_unroll.mean()
        )
    )
    print(
        String("Vectorized list speedup vs cpp: {}").format(
            cpp_speed / report_vectorize.mean()
        )
    )


fn _run() -> Int64:
    var vec = List[Int64](capacity=SIZE)
    for i in range(SIZE):
        vec.append(i)

    var list_sum: Int64 = 0
    for i in range(SIZE):
        list_sum += vec[i]
    return list_sum


fn _run_vectorize() -> Int64:
    var p = UnsafePointer[Int64].alloc(SIZE)

    @parameter
    fn init[wd: Int](i: Int):
        p.store(i, i)

    vectorize[init, simd_width](SIZE)

    var list_sum = SIMD[DType.int64, simd_width]()

    @parameter
    fn sum[wd: Int](i: Int):
        list_sum += p.load[width=simd_width](i)

    vectorize[sum, simd_width](SIZE)
    return list_sum.reduce_add()
