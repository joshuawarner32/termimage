#include <stdio.h>
#include <stdlib.h>
#include <string.h>

class PiCalc {
 public:
  size_t size;
  uint64_t* values;
  size_t nines;
  int predigit;
  size_t j;
  size_t k;
  uint64_t q;

  enum State {
    Init,
    ZeroLoop,
    NineLoop,
  } state;

  PiCalc(size_t digits)
      : size(digits * 10 / 3 + 1),
        values((uint64_t *)malloc(size * sizeof(uint64_t))), nines(0),
        predigit(0), j(0), k(0), q(0), state(Init) {
    for(size_t k = 0; k < size; k++) {
      values[k] = 2;
    }
  }

  ~PiCalc() {
    free(values);
  }

  int nextDigit() {
    while(true) {
      switch(state) {
      case Init: {
        q = 0;
        for(size_t i = size; i > 0; i--) {
          uint64_t x = 10 * values[i - 1] + q * i;
          if(x > 0xfffffffffffffff) {
            fprintf(stderr, "\nRan out of precision!\n");
            exit(1);
          }
          values[i - 1] = x % (2 * i - 1);
          q = x / (2 * i - 1);
        }
        values[0] = q % 10;
        q /= 10;
        if(q == 9) {
          nines++;
        } else if(q == 10) {
          k = 0;
          state = ZeroLoop;
          return predigit + 1;
        } else {
          int ret = predigit;
          predigit = q;
          if(nines != 0) {
            k = 0;
            state = NineLoop;
          }
          return ret;
        }
      } break;
      case ZeroLoop:
        if(k++ < nines) {
          return 0;
        } else {
          predigit = 0;
          nines = 0;
          state = Init;
        }
        break;
      case NineLoop:
        if(k++ < nines) {
          return 9;
        } else {
          nines = 0;
          state = Init;
        }
        break;
      }
    }
  }
};

int main(int argc, char** argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  size_t digits = atoi(argv[1]);
  PiCalc piCalc(digits);
  piCalc.nextDigit();
  printf("%d.", piCalc.nextDigit());
  digits -= 2;
  while(--digits > 0) {
    printf("%d", piCalc.nextDigit());
  }
}