/*
 * Copyright 2013,2014 The University of Adelaide
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include "mem.h"
#include "config.h"


#define NTIMING	100000
#define CUTOFF 120



inline unsigned long gettime() {
  volatile unsigned long tl;
  asm __volatile__("lfence\nrdtsc" : "=a" (tl): : "%edx");
  return tl;
}

inline int probe(char *adrs) {
  volatile unsigned long time;

  asm __volatile__ (
    "  mfence             \n"
    "  lfence             \n"
    "  rdtsc              \n"
    "  lfence             \n"
    "  movl %%eax, %%esi  \n"
    "  movl (%1), %%eax   \n"
    "  lfence             \n"
    "  rdtsc              \n"
    "  subl %%esi, %%eax  \n"
    "  clflush 0(%1)      \n"
    : "=a" (time)
    : "c" (adrs)
    :  "%esi", "%edx");
  return time;
}

inline void flush(char *adrs) {
  asm __volatile__ ("mfence\nclflush 0(%0)" : : "r" (adrs) :);
}



int main(int c, char **v) {
  config_t conf =readConfig(v[1]);
  if (!checkConfig(conf))
    exit(1);

  char *ip = map(conf->fileName, 0);
  int noffsets = conf->noffsets;
  char **ptrs = malloc(sizeof(char *) * noffsets);
  int i;
  for (i = 0; i < noffsets; i++)
    ptrs[i] = ip + ((conf->offsets[i] - conf->base) & ~0x3f);
  int *times = malloc(sizeof(long) * noffsets);
  int *touched = malloc(sizeof(int) * noffsets);
  int *timings = calloc(sizeof(int), noffsets * NTIMING);
  long tmp;
  int n = 0;
  int timing = 0;
  int ind = 0;
  int first = 10;
  int second = 10;
  unsigned int slotstart;
  unsigned int current;
  int hit;
  int debug = 1;

  char *buffer = malloc (NTIMING);
  setvbuf(stdout, buffer, _IOLBF, NTIMING);

  if (c >= 3 && strchr(v[2], 't') != NULL)
    timing = 1;
  if (c >= 3 && strchr(v[2], 'd') != NULL)
    debug = 0;

  for (i = 0; i < noffsets; i++)
    flush(ptrs[i]);
  slotstart = gettime();

    pid_t pid = getpid();
    printf("%d\n", pid);

//  while (ind < NTIMING) {
    while (1) {
    for (i = 0; i < noffsets; i++) {
      times[i] = probe(ptrs[i]);
//      timings[ind * noffsets + i] = times[i];
    }
    ind++;

    do {
      current = gettime();
    } while (current - slotstart < conf->slotSize);
    slotstart += conf->slotSize;

    while (current - slotstart > conf->slotSize) {
//      if (ind < NTIMING) {
//          for (i = 0; i < noffsets; i++)
//              timings[ind * noffsets + i] = -1;
//          ind++;
//      }
      slotstart += conf->slotSize;
    }
  }

//  for (i=0; i<NTIMING; i++) {
//    printf("%d	%d	%d\n", timings[i*noffsets], timings[i*noffsets+1], timings[i*noffsets+2]);
//  }
}
