#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <json_parser.h>

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  char *new_str = (char *)malloc(size + 1);
  if (new_str == NULL) {
    return 0;
  }
  memcpy(new_str, data, size);
  new_str[size] = '\0';

  json_value_t *retval = json_value_parse(new_str);
  if (retval) {
    json_value_destroy(retval);
  }

  free(new_str);
  return 0;
}