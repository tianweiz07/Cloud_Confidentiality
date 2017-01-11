#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "cpuid.h"



void __attribute__ ((noinline)) cpuid(struct cpuidRegs *regs) {
  asm __volatile__ ("cpuid": "+a" (regs->eax), "+b" (regs->ebx), "+c" (regs->ecx), "+d" (regs->edx));
}

void cpuid_cacheInfo(int index, struct cacheInfo *cacheInfo) {
  struct cpuidRegs *regs = (struct cpuidRegs *)cacheInfo;
  regs->eax = 4;
  regs->ecx = index;
  cpuid(regs);
}

static struct cacheInfo l3Info;
static int getL3Info() {
  if (l3Info.level != 0)
    return 1;

  for (int i = 0; cpuid_cacheInfo(i, &l3Info), l3Info.type !=0; i++) 
    if (l3Info.level == 3)
      return 1;
  
  return 0;
}

int cpuid_l3size() {
  if (!getL3Info())
    return -1;

  return (l3Info.lineSize +1)*(l3Info.partitions + 1) *(l3Info.associativity + 1)*(l3Info.sets + 1);
}

int cpuid_l3colours() {
  if (!getL3Info())
    return -1;
  return (l3Info.sets + 1)*(l3Info.partitions + 1)*(l3Info.lineSize +1) / 4096;
}

int cpuid_l3assoc() {
  if (!getL3Info())
    return -1;
  return l3Info.associativity+1;
}
