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

TCCState* generate_tune(tune_t *f) {
  TCCState *s = tcc_new();
  char gen[100];
  char start[] = "int a_tune(int t) { return ";
  char end[] = "; }";
  char full_function[sizeof(start) + sizeof(gen) + sizeof(end)] = "";

  if (s == NULL) {
    fprintf(stderr, "could not create tcc context\n");
    tcc_delete(s);
    return (TCCState*) -1;
  }

  srand(time(NULL));

  int i, x;
  char c;
  char ops[] = {'+', '-', '*', '&', '|', '>', '<'};
  bool a = true;
  bool p = false;
  for (i = 0; i < 41; i++) {
    if (a) {
      x = rand() % (p ? 2 : 3);
      if (x == 0) {
        c = 't';
        a = !a;
      }
      else if (x == 1) {
        c = '1' + rand() % 9;
        a = !a;
      }
      else {
        c = '(';
        p = true;
      }
    }
    else {
      if (rand() % (p ? 2 : 1) == 0) {
        c = ops[rand() % sizeof(ops)];
        if (c == '<' || c == '>') {
          gen[i] = c;
          i++;
        }
        a = !a;
      }
      else {
        c = ')';
        p = !p;
      }
    }
    gen[i] = c;
  }
  int n = 41;
  if (a) {
    gen[n] = 't';
    n++;
  }
  if (p) {
    gen[n] = ')';
    n++;
  }
  gen[n] = 0;
  strcat(full_function, start);
  strcat(full_function, gen);
  strcat(full_function, end);
  fprintf(stderr, "%s\n", full_function);
  if (tcc_compile_string(s, full_function) == -1) {
    fprintf(stderr, "could not compile code\n");
    tcc_delete(s);
    return (TCCState*) -1;
  }
  tcc_relocate(s, TCC_RELOCATE_AUTO);

  // This is forbidden, but whatever.
  *f = (tune_t) tcc_get_symbol(s, "a_tune");

  return s;
}

int main(int argc, char **argv) {
  tune_t a_tune;
  TCCState *s = generate_tune(&a_tune);
  run_generator(a_tune);
  tcc_delete(s);
  return EXIT_SUCCESS;
}
