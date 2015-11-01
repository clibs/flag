
#include <stdio.h>
#include "flag.h"

#define VERSION "v1.0.0"

int
main(int argc, const char **argv) {
  int requests = 5000;
  int concurrency = 10;
  const char *url = ":3000";

  flag_int(&requests, "requests", "Number of total requests");
  flag_int(&concurrency, "concurrency", "Number of concurrent requests");
  flag_str(&url, "url", "Target url");
  flag_parse(argc, argv, VERSION);

  puts("");
  printf("     requests: %d\n", requests);
  printf("  concurrency: %d\n", concurrency);
  printf("          url: %s\n", url);
  puts("");

  return 0;
}