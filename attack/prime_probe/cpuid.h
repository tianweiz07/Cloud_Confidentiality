#ifndef __CPUID_H__
#define __CPUID_H__


struct cpuidRegs {
  uint32_t eax;
  uint32_t ebx;
  uint32_t ecx;
  uint32_t edx;
};





#define CACHETYPE_NULL		0
#define CACHETYPE_DATA		1
#define CACHETYPE_INSTRUCTION	2
#define CACHETYPE_UNIFIED	3

struct cacheInfo {
  uint32_t	type:5;
  uint32_t	level:3;
  uint32_t	selfInitializing:1;
  uint32_t	fullyAssociative:1;
  uint32_t	reserved1:4;
  uint32_t	logIds:12;
  uint32_t	phyIds:6;

  uint32_t	lineSize:12;
  uint32_t	partitions:10;
  uint32_t	associativity:10;

  uint32_t	sets:32;

  uint32_t	wbinvd:1;
  uint32_t	inclusive:1;
  uint32_t	complexIndex:1;
  uint32_t	reserved2:29;
};

void cpuid(struct cpuidRegs *regs);
void getCacheInfo(int index, struct cacheInfo *cacheInfo);


int cpuid_l3size();
int cpuid_l3colours();
int cpuid_l3assoc();

#endif // __CPUID_H__
