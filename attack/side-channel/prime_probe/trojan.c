#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#include "pageset.h"
#include "cpuid.h"
#include "cachemap.h"
#include "probe.h"
#include "clock.h"



#define PORT	1556
#define BLOCKSIZE 16

#define xstr(s) str(s)
#define str(s) #s


void function() {
}

void space() {
 asm(
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   "nop \n"
   );
}

int main(int c, char **v) {

  probe_init(NULL, 0);
  srandom(getpid());

  int colours = cpuid_l3colours();
  int assoc = cpuid_l3assoc();

  pageset_t *pms = cachemap();

  //long x= (long)axxa;
  char *x = malloc(8192);
  x = x + 4096 - ((unsigned long)x & 4095);

  pageset_t ps = ps_new();
  for (int j = 0; j < 1; j++)
    ps_push(ps, ps_get(pms[0],j));
  listEntry_t  prime = probe_makelist(ps, (void *) 0x100);

  unsigned long time;
  for (;;) {
    time = gettime();
    while (gettime() - time < 1000000) {
      function();
    }
    time = gettime();
    while (gettime() - time < 2000000) {
    }
  }


}




