#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>

#include <stdint.h>
#include "cpuid.h"
#include "pageset.h"
#include "probe.h"

int main(int c, char **v) {
  int shmid = shmget(SHMKEY, cpuid_l3size() * 2, SHM_HUGETLB|IPC_CREAT|0666);
  if (shmid < 0) {
    perror("shmget");
    exit(1);
  }
  printf("Got shmid=%d\n", shmid);
}

