#include <stdlib.h>
#include <stdio.h>

#include "cdf_record.h"

struct record {
  unsigned int value;
  unsigned int cnt;
  struct record *next;
};

struct record *cdf_record_update(struct record *l, unsigned int value)
{
    struct record *p, *p1, *res;

    p1 = NULL;
    p = l;
    while ((p != NULL) && (p->value <= value)) {
        if (p->value == value) {
            p->cnt++;

            return l;
        }
        p1 = p;
        p = p->next;
    }

    res = malloc(sizeof(struct record));
    if (res == NULL) {
        return NULL;
    }
    res->value = value;
    res->cnt = 1;
    res->next = p;

    if (p1 == NULL) {
        return res;
    }
    p1->next = res;

    return l;
}

void cdf_print(FILE *out, struct record *l)
{
    int cnt, total;
    struct record *p;

    total = 0;
    p = l;
    while (p != NULL) {
        total += p->cnt;
        p = p->next;
    }

    cnt = 0;
    p = l;
    while (p != NULL) {
        cnt += p->cnt;
        fprintf(out, "%d %d %f\n", p->value, cnt, (double)cnt / (double)total);
        p = p->next;
    }
}

void cdf_free(struct record *l)
{
  while (l != NULL) {
    struct record *p;

    p = l->next;
    free(l);
    l = p;
  }
}
