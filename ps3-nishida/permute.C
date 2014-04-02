#include "permute.h"

void randomPermutation (int n, int *p)
{
  for (int i = 0; i < n; ++i)
    p[i] = i;
  for (int i = 0; i < n - 1; ++i) {
    int j = randomInteger(i, n - 1), ti = p[i];
    p[i] = p[j];
    p[j] = ti;
  }
}    

int randomInteger (int lb, int ub)
{
  double r = random()/(double) RAND_MAX;
  int d = (int) floor(r*(ub - lb + 1));
  return lb + d;
}
