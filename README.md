# vector vs list vs simd array

works on my machine

## Traversal

### Cpp

#### Int32

size=10_000

| Data Structure | Time (s) | Speedup |
| -------------- | --------- | ------- |
| Compile time   | 4.565e-06 | 1x      |
| SIMD vector    | 9.6995e-05 | 21.25x  |
| Vector         | 0.000116879 | 25.60x  |
| List           | 0.000904024 | 198.03x |

#### Int64

size=10_000_000

| Data Structure | Time (s) | Speedup |
| -------------- | -------- | ------- |
| SIMD vector    | 0.142992 | 1x      |
| Vector         | 0.149760 | 1.05x   |
| List           | 0.614871 | 4.30x   |

### Mojo

size=10_000

| Description | Time (s) | Speedup vs c++ vector |
| ----------- | -------- | --------------------- |
| List traversal | 0.032144 | 0.032466x (c++ vector is 275x faster) |
| Unrolled list traversal | 1.9e-13 | 6_151_526_315_789x |
| Vectorized list traversal | 1.79e-13 | 6_493_277_777_778x |

## Insertion

TODO

<!-- (before switching to in64)

| Data Structure | Time (s) | Speedup |
| -------------- | --------- | ------- |
| SIMD vector    | 0.0597482 | 1x      |
| Vector         | 0.0927972 | 1.55x   |
| List           | 5.676612  | 95.01x  |   -->

<span style="text-align: center;">
<h3>Ode to Hardware, Not Your Fancy Algorithms</h3>
<pre>
Oh, ye algorithms, in textbooks so proud,
With Big O notations, you shout aloud.
Logarithms, trees, and lists you boast,
But in the halls of hardware, you're a ghost.

For what is an algorithm without its steel,
The transistors that make the math real?
Your mergesort may seem fast on the page,
But next to SIMD, it's a mere stone age.

The vector, aligned in memory's tight rows,
Flows like the wind wherever data goes.
The cache is king, the pipeline queen,
Not your recursion, not your pristine dream.

Insertion, you say, with logarithmic grace?
But the hardware laughs, picks up the pace.
SIMD strides forth, parallel lanes,
While your algorithm chokes, cries out in pain.

Oh, linked list, you promise constant insert,
But where's your cache? You only hurt.
Each node's a miss, a memory fail,
As the hardware races down its blazing trail.

So let Chris Lattner be witness this day,
Hardware, my love, is where I'll stay.
Your fancy algorithms, so clean, so fine,
Are nothing when the silicon shines.

For in the end, it's not just the thought,
It's the hardware's power that fights and fought.
So bring your algorithms, I'll give you a clue—
The machine knows best, not your CS crew.
</pre>  
</span>
