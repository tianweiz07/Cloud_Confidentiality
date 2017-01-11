#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>
#include <errno.h>
#include <err.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>

#define CPU 0
#define STRINGIZE(X) #X
#define MSR_FILE(CPU) "/dev/cpu/" STRINGIZE(CPU) "/msr"

#define ELEM_TYPE uint64_t

#define ONE addr = (uint64_t *)*addr;
#define FOUR ONE ONE ONE ONE
#define SIXTEEN FOUR FOUR FOUR FOUR
#define THIRTY_TWO SIXTEEN SIXTEEN
#define SIXTY_FOUR THIRTY_TWO THIRTY_TWO

int main(int argc, char **argv) {

  /**
   * Pin process on core CPU
   */
  cpu_set_t mask;
  CPU_ZERO(&mask);
  CPU_SET(CPU, &mask);
  if (sched_setaffinity(0, sizeof(mask), &mask) == -1) {
    printf("sched_setaffinity failed\n");
    return -1;
  }


  /**
   * Open msr file to get file descriptor.
   */
  char *msrfile = MSR_FILE(CPU);
  int fd = open(msrfile, O_RDWR);
  if (fd == -1) {
    printf("Couldn't open %s\n", msrfile);
    return -1;
  }

  /**
   * Clear the first general purpose MSR we'll use to count event.
   */
  off_t gp_cnt = 0xC1;
  uint64_t zero_buf = 0;
  ssize_t res = pwrite(fd, &zero_buf, 8, gp_cnt);
  if (res != 8) {
    printf("Couldn't clear msr 0xC1\n");
    return -1;
  }

  /**
   * Program OFFCORE_RSP event
   */
//  off_t offcore_rsp_0_msr = 0x1a6;
//  uint64_t offcore_rsp_0_value = 0x1033; // REMOTE_CACHE_FWD
//  res = pwrite(fd, &offcore_rsp_0_value, 8, offcore_rsp_0_msr);
//  if (res != 8) {
//    printf("Couldn't write to 0x1A6 msr\n");
//    return -1;
//  }

  /**
   * Start counting by writing to IA32_PERFEVTSEL
   */
  off_t ia32_perfevtsel_msr = 0x186;
  uint64_t ia32_perfevtsel_value = 0x5101b7;
  res = pwrite(fd, &ia32_perfevtsel_value, 8, ia32_perfevtsel_msr);
  if (res != 8) {
    printf("Couldn't write to 0x186 msr\n");
    return -1;
  }

  /**
   * Code to be profiled
   */

  /**
   * Stop counting by writing to IA32_PERFEVTSEL
   */
  ia32_perfevtsel_value = 0x1101b7;
  res = pwrite(fd, &ia32_perfevtsel_value, 8, ia32_perfevtsel_msr);
  if (res != 8) {
    printf("Couldn't write to 0x186 msr\n");
    return -1;
  }

  /**
   * Read the counter
   */
  uint64_t count;
  res = pread(fd, &count, 8, gp_cnt);
  if (res == -1) {
    printf("Couldn't read 0xC1 msr\n");
    return -1;
  }
  printf("remote cache count= %lu\n", count);

  return 0;
}
