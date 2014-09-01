/* An interactive bytebeat player by Niels.
 *
 * After compiling, run:
 *
 * $ ./clive | aplay # Now type characters from a-z0-9 to play music
 *
 * For better realtime handling (and if you want to keep the buffer size at its
 * current small size), run:
 *
 * # ionice -c 1 -n 0 ./clive | ionice -c 1 -n 0 aplay
 *
 * To save to a file, run:
 * $ ./clive | sox -r 8000 -c 1 -t s8 - output.wav
 *
 * To play AND save to a file, run:
 *
 * $ mkfifo output.raw && \
       sox -r 8000 -c 1 -t s8 output.raw output.wav && \
       ./clive | tee output.raw | aplay && rm output.raw
 *
 * though you might need to use ionice or similar in these cases as well.
 *
 * Read more about the concept of bytebeat on
 * http://canonical.org/~kragen/bytebeat/
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>

/* The number of samples to be generated between each check for extra sleep:
   Hopefully this program spits out samples much faster than needed for 8kHz
   sound, so it sleeps once in a while so it never generates too far ahead. */
#define SAMPLES_IN_ONE_GO 200

/* Size of buffer.  You need to set this much higher if you don't run the
   program as realtime. */
#define SAMPLES_BUFFER 200

#define DEFTUNE(name, exp) int name (int t) { return exp; }

typedef int (*tune_t)(int);

struct keytune {
  char key;
  tune_t tune;
};

/* From http://pelulamu.net/countercomplex/music_formula_collection.txt */
DEFTUNE (sawtooth, t)
DEFTUNE (sierpinski, t&t>>8)
DEFTUNE (fortytwo, t*(42&t>>10))
DEFTUNE (danharaj0, t|t%255|t%257)
DEFTUNE (droid0, t>>6&1?t>>5:-t>>4)
DEFTUNE (roy0, t*(t>>9|t>>13)&16)
DEFTUNE (krcko0, (t&t>>12)*(t>>4|t>>8))
DEFTUNE (viznut0, (t*5&t>>7)|(t*3&t>>10))
DEFTUNE (tejeez0, (t*(t>>5|t>>8))>>(t>>16))
DEFTUNE (miiro0, t*5&(t>>7)|t*3&(t*4>>10))
DEFTUNE (robert0, (t>>13|t%24)&(t>>7|t%19))
DEFTUNE (roy1, (t*((t>>9|t>>13)&15))&129)
DEFTUNE (viznut1, (t&t%255)-(t*3&t>>13&t>>6))
DEFTUNE (krcko1, (t&t>>12)*(t>>4|t>>8)^t>>6)
DEFTUNE (blueberry0, t*(((t>>9)^((t>>9)-1)^1)%13))
DEFTUNE (rrola0, t*(0xCA98>>(t>>9&14)&15)|t>>8)
DEFTUNE (tonic0, (t/8)>>(t>>9)*t/((t>>14&3)+4))
DEFTUNE (freefull0, (~t/100|(t*3))^(t*3&(t>>5))&t)
DEFTUNE (red0, (t|(t>>9|t>>7))*t&(t>>11|t>>9))
DEFTUNE (harism0, ((t>>1%128)+20)*3*t>>14*t>>18)
/* DEFTUNE (droid1, t&(sin(t&t&3)*t>>5)/(t>>3&t>>6)) // Meh */
DEFTUNE (viznut2, t*(((t>>12)|(t>>8))&(63&(t>>4))))
DEFTUNE (visy0, t*(((t>>9)|(t>>13))&(25&(t>>6))))
DEFTUNE (n216, t*(t^t+(t>>15|1)^(t-1280^t)>>10))
DEFTUNE (tejeez1, t*(((t>>11)&(t>>8))&(123&(t>>3))))
DEFTUNE (viznut3, (t>>7|t|t>>6)*10+4*(t&t>>13|t>>6))
DEFTUNE (stephth0, (t*9&t>>4|t*5&t>>7|t*3&t/1024)-1)
DEFTUNE (visy1, t*(t>>((t>>9)|(t>>8))&(63&(t>>4))))
DEFTUNE (viznut4, (t>>6|t|t>>(t>>16))*10+((t>>11)&7))
DEFTUNE (yumeji0, (t>>1)*(0xbad2dea1>>(t>>13)&3)|t>>5)
DEFTUNE (ryg0, (t>>4)*(13&(0x8898a989>>(t>>11&30))))
DEFTUNE (marmakoide, (t>>(t&7))|(t<<(t&42))|(t>>7)|(t<<5))
DEFTUNE (robert1, (t>>7|t%45)&(t>>8|t%35)&(t>>11|t%20))
DEFTUNE (lucasvb0, (t>>6|t<<1)+(t>>5|t<<3|t>>3)|t>>2|t<<1)
DEFTUNE (bear0, t+(t&t^t>>6)-t*((t>>9)&(t%16?2:6)&t>>9))

struct keytune tunes[] = {
  { 'a', sawtooth },
  { 'b', sierpinski },
  { 'c', fortytwo },
  { 'd', danharaj0 },
  { 'e', droid0 },
  { 'f', roy0 },
  { 'g', krcko0 },
  { 'h', viznut0 },
  { 'i', tejeez0 },
  { 'j', miiro0 },
  { 'k', robert0 },
  { 'l', roy1 },
  { 'm', viznut1 },
  { 'n', krcko1 },
  { 'o', blueberry0 },
  { 'p', rrola0 },
  { 'r', tonic0 },
  { 's', freefull0 },
  { 't', red0 },
  { 'u', harism0 },
  /* { 'v', droid1 }, */
  { 'w', viznut2 },
  { 'x', visy0 },
  { 'y', n216 },
  { 'z', tejeez1 },
  { '0', viznut3 },
  { '1', stephth0 },
  { '2', visy1 },
  { '3', viznut4 },
  { '4', yumeji0 },
  { '5', ryg0 },
  { '6', marmakoide },
  { '7', robert1 },
  { '8', lucasvb0 },
  { '9', bear0 }
};
#define N_TUNES sizeof(tunes) / sizeof(struct keytune)

tune_t *tunes_active;

struct termios cur_t;

pid_t child_pid = -1;

void reset_terminal() {
  ioctl(0, TCSETS, &cur_t);
}

void run_at_end() {
  reset_terminal();
  if (child_pid != -1) {
    kill(child_pid, SIGINT);
  }
}

void signal_handler(int sig) {
  if (sig == SIGINT) {
    run_at_end();
  }
  signal(sig, SIG_DFL);
  kill(getpid(), sig);
}

void setup_term_and_handlers() {
  struct termios new_t;
  ioctl(0, TCGETS, &cur_t);
  new_t = cur_t;
  new_t.c_lflag &= ~ECHO; /* echo off */
  new_t.c_lflag &= ~ICANON; /* one char at a time */
  atexit(run_at_end);
  signal(SIGINT, signal_handler);
  ioctl(0, TCSETS, &new_t);
}

void input_loop() {
  char c;
  int i;
  while (true) {
    c = getchar();
    if (c == 'q') {
      break;
    }
    for (i = 0; i < N_TUNES; i++) {
      if (tunes[i].key == c) {
        if (tunes_active[i] == NULL) {
          tunes_active[i] = tunes[i].tune;
        }
        else {
          tunes_active[i] = NULL;
        }
      }
    }
  }
}

void run_generator() {
  int i, j, t, n_active, val;
  struct timeval t0, t1;
  long long sum, n_samples_ahead, elapsed;

  gettimeofday(&t0, 0);
  t = 0;
  while (true) {
    for (i = 0; i < SAMPLES_IN_ONE_GO; i++, t++) {
      sum = 0;
      n_active = 0;
      for (j = 0; j < N_TUNES; j++) {
        if (tunes_active[j] != NULL) {
          sum += ((tune_t) tunes_active[j])(t);
          n_active++;
        }
      }
      val = (n_active > 0) ? sum / n_active : 0;
      putchar(val);
    }
    gettimeofday(&t1, 0);
    elapsed = (t1.tv_sec - t0.tv_sec) * 1000000LL + t1.tv_usec - t0.tv_usec;
    n_samples_ahead = t - elapsed * 0.008;
    if (n_samples_ahead - SAMPLES_BUFFER > 0) {
      usleep(((double) n_samples_ahead - SAMPLES_BUFFER) / 0.008);
    }
  }
}

void create_input_loop_and_generator() {
  tunes_active = mmap(NULL, sizeof(tune_t) * N_TUNES,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS,
                      -1, 0);
  if (tunes_active == MAP_FAILED) {
    fprintf(stderr, "mmap failed\n");
    exit(1);
  }
  int i;
  for (i = 0; i < N_TUNES; i++) {
    tunes_active[i] = NULL;
  }
  
  child_pid = fork();
  if (child_pid < 0) {
    fprintf(stderr, "fork failed\n");
    exit(1);
  }
  else if (child_pid == 0) {
    run_generator();
  }
  else {
    setup_term_and_handlers();
    input_loop();
  }
}

int main(int argc, char **argv) {
  int i;
  if (argc == 2) {
    for (i = 0; argv[1][i] != 0; i++) {
      ungetc(argv[1][i], stdin);
    }
  }
  create_input_loop_and_generator();
}
