module; // Global module fragment

#include <cmath>

export module math; // Module declaration

// Exported functions - these will be visible to importers
export double square(double x) { return x * x; }
