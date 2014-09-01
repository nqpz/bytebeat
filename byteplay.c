/* A bytebeat generator by Niels.
 *
 * After compiling, run:
 *
 * $ ./bibgen | aplay
 *
 * Read more about the concept of bytebeat on
 * http://canonical.org/~kragen/bytebeat/
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include <libtcc.h>

#define SAMPLES_BUFFER 8000

typedef int (*tune_t)(int);

void run_generator(tune_t tune) {
  int t;
  for (t = 0; ; t++) {
    putchar(tune(t));
  }
}

TCCState* compile_tune(char* exp, tune_t *f) {
  char start[] = "int a_tune(int t) { return ";
  char end[] = "; }";

  TCCState *s = tcc_new();

  if (s == NULL) {
    fprintf(stderr, "could not create tcc context\n");
    tcc_delete(s);
    return (TCCState*) -1;
  }

  char *full_function =
    (char*) malloc(sizeof(char)
                   * (sizeof(start) + strlen(exp) + sizeof(end)));
  *full_function = 0;
  strcat(full_function, start);
  strcat(full_function, exp);
  strcat(full_function, end);
  fprintf(stderr, "%s\n", full_function);
  if (tcc_compile_string(s, full_function) == -1) {
    fprintf(stderr, "could not compile code\n");
    tcc_delete(s);
    return (TCCState*) -1;
  }
  tcc_relocate(s, TCC_RELOCATE_AUTO);
  *f = tcc_get_symbol(s, "a_tune");
  return s;
}

int main(int argc, char **argv) {
  tune_t a_tune;
  if (argc < 2) {
    fprintf(stderr, "Specify a formula.\n");
    return EXIT_FAILURE;
  }
  TCCState *s = compile_tune(argv[1], &a_tune);
  run_generator(a_tune);
  tcc_delete(s);
  return EXIT_SUCCESS;
}
