#include <stdio.h>
#include <math.h>
#include <float.h>

int main(int argc, char *argv[])
{
  double val, sum = 0, sum2 = 0, min = DBL_MAX, max = 0.0;
  int res, n = 0;

  while(!feof(stdin)) {
    res = scanf("%lf\n", &val);
    if (res == 1) {
      n++;
      sum += val;
      sum2 += val * val;
      if (val < min) min = val;
      if (val > max) max = val;
    }
  }

  printf("%lf %lf %lf %lf\n", sum / n, sqrt(sum2 / n - (sum / n) * (sum / n)), min, max);

  return 0;
}

