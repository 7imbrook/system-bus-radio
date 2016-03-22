// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken

#include <stdio.h>
#include <stdlib.h>
#include <emmintrin.h>
#include <mach/mach_traps.h>
#include <mach/mach_time.h>
#include <math.h>
#include <string.h>

#include "smaz.h"

__m128i reg;
__m128i reg_zero;
__m128i reg_one;
mach_port_t clock_port;
mach_timespec_t remain;

static inline void square_am_signal(float time, float frequency) {
    // printf("Playing / %0.3f seconds / %4.0f Hz\n", time, frequency);
    uint64_t period = NSEC_PER_SEC / frequency;

    uint64_t start = mach_absolute_time();
    uint64_t end = start + time * NSEC_PER_SEC;

    while (mach_absolute_time() < end) {
        uint64_t mid = start + period / 2;
        uint64_t reset = start + period;
        while (mach_absolute_time() < mid) {
            _mm_stream_si128(&reg, reg_one);
            _mm_stream_si128(&reg, reg_zero);
        }
        clock_sleep_trap(clock_port, TIME_ABSOLUTE, reset / NSEC_PER_SEC, reset % NSEC_PER_SEC, &remain);
        start = reset;
    }
}

#define WAIT 0.05
void char_transmit(char x) {
  for (int i = 1; i <= 8; i ++) {
    int up = !!((x << i) & 0x80);
    printf("%d", up);
    if (up) {
      square_am_signal(WAIT, 2900);
    } else {
      square_am_signal(WAIT, 2100);
    }
  }
}

void char_raw_bit_stream_stdin () {
  while(1) {
      char x = getchar();
      printf("%3d -> ", x);
      char_transmit(x);
      printf("\n");
  }
}

void str_transmit(char in[]) {
  for (int is = 0; in[is] != '\0'; is++) {
    char_transmit(in[is]);
  }
}

void char_encoded_bit_stream () {
  char input_str[] = "Hello my name is Michael Timbrook";
  char out_str[1024];
  printf("%s\n", input_str);
  int res = smaz_compress(&input_str, strlen(input_str), &out_str, sizeof(out_str));
  printf("%s\n", out_str);

  str_transmit(out_str);

}

void test_signal () {
  while (1) {
    printf("\rHigh"); fflush(stdout);
    square_am_signal(0.400, 2900);
    printf("\rLow "); fflush(stdout);
    square_am_signal(0.400, 2100);
  }
}

int main(int argc, char **argv)
{
    mach_timebase_info_data_t theTimeBaseInfo;
    mach_timebase_info(&theTimeBaseInfo);
    puts("TESTING TIME BASE: the following should be 1 / 1");
    printf("  Mach base: %u / %u nanoseconds\n\n", theTimeBaseInfo.numer, theTimeBaseInfo.denom);

    uint64_t start = mach_absolute_time();
    uint64_t end = mach_absolute_time();
    printf("TESTING TIME TO EXECUTE mach_absolute_time()\n  Result: %lld nanoseconds\n\n", end - start);

    reg_zero = _mm_set_epi32(0, 0, 0, 0);
    reg_one = _mm_set_epi32(-1, -1, -1, -1);

    int opt;
    while ((opt = getopt(argc, argv, "t")) != -1)
    {
      switch (opt)
      {
        case 't':
          test_signal();
          exit(0);
      }
    }
    char_encoded_bit_stream();
    return 0;
}
