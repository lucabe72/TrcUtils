#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX 5000

int main(int argc, char *argv[])
{
    static double pmf[MAX];
    double sum = 0;
    int i;

    while(!feof(stdin)) {
        int res, val;
        double p;

        res = scanf("%d %lf\n", &val, &p);
        if (res == 2) {
            pmf[val] = p;
        }
    }

    for (i = 0; i < MAX; i++) {
      printf("%d %16.14f\n", i * 100, sum);
      sum += pmf[i];
    }

    return 0;
}
