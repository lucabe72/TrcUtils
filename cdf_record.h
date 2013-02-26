struct record;

struct record *cdf_record_update(struct record *l, unsigned int value);
void cdf_print(FILE *out, struct record *l);
void cdf_free(struct record *l);

