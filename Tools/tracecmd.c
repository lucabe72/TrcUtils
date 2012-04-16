#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int initial_read(FILE *f)
{
  int c, res;
  char buff[16];

  res = fread(buff, 10, 1, f);
  if (res != 1) {
    return -1;
  }
  if ((buff[0] != 0x17) ||
      (buff[1] != 0x08) ||
      (buff[2] != 0x044)||
      memcmp(buff + 3, "tracing", 7)) {
    printf("%x %x %x %s\n", buff[0], buff[1], buff[2], buff + 3);
    return -2;
  }

  printf("Version: ");
  c = 1;
  while (c) {
    c = fgetc(f);
    if (c == EOF) {
      return -1;
    }
    printf("%c", (char)c);
  }
  printf("\n");

  res = fread(buff, 6, 1, f);
  if (res != 1) {
    return -1;
  }
  printf("Endianess: %d\tBytes per Long: %d\tPage size: %d\n", buff[0], buff[1],
         (buff[2] << 24) + (buff[3] << 16) + (buff[4] << 8) + buff[5]);

  return 0;
}

int header_info_format_read(FILE *f)
{
  char buff[16], *header_page, *header_event;
  int res;
  uint64_t size;

  res = fread(buff, 12, 1, f);
  if (res != 1) {
    return -1;
  }

  if (memcmp(buff, "header_page\0", 12)) {
    return -2;
  }

  res = fread(&size, 8, 1, f);
  if (res != 1) {
    return -1;
  }
  printf("Page header information size: %llu\n", size);
  header_page = malloc(size);
  res = fread(header_page, size, 1, f);
  if (res != 1) {
    return -1;
  }
  printf("Header Page: %s\n", header_page);

  res = fread(buff, 13, 1, f);
  if (res != 1) {
    return -1;
  }

  if (memcmp(buff, "header_event\0", 12)) {
    return -2;
  }

  res = fread(&size, 8, 1, f);
  if (res != 1) {
    return -1;
  }
  printf("Next size: %llu\n", size);
  header_event = malloc(size);
  res = fread(header_event, size, 1, f);
  if (res != 1) {
    return -1;
  }
  printf("Header Event: %s\n", header_event);

  return 0;
}

int event_format_file_read(FILE *f)
{
  int i, res;
  uint32_t num;

  res = fread(&num, 4, 1, f);
  if (res != 1) {
    return -1;
  }

  for (i = 0; i < num; i++) {
    uint64_t size;
    char *event_format;

    res = fread(&size, 8, 1, f);
    if (res != 1) {
      return -1;
    }
    event_format = malloc(size);
    res = fread(event_format, size, 1, f);
    if (res != 1) {
      return -1;
    }
    printf("Event %d format: %s\n", i, event_format);
    free(event_format);
  }

  return 0;
}
int main(int argc, char *argv[])
{
  FILE *f;
  int res;

  f = fopen(argv[1], "r");
  if (f == NULL) {
    perror("FOpen");

    return -1;
  }

  res = initial_read(f);
  printf("Initial Header Read: %d\n", res);
  res = header_info_format_read(f);
  printf("Header Info Format Read: %d\n", res);
  res = event_format_file_read(f);
  printf("Event Format File Read: %d\n", res);

  return res;
}
