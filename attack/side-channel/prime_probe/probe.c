#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "pageset.h"
#include "probe.h"
#include "cpuid.h"

/*cache line size 64bits*/
#define CACHELINE_BITS	6
#define CACHELINE_SIZE	(1<<CACHELINE_BITS)
#define CACHELINE_MASK	(CACHELINE_SIZE - 1)

#define LINES_PER_PAGE	(PAGE_SIZE / CACHELINE_SIZE)

#define NPAGES 4096
#define NPROBES 1000
#define DEFAULT_RUNS 20


/*locate the cacheline from probing buffer, the Nth in a page*/
#define LISTENTRY(page, line) cl2le(buffer[page].cacheLines+line)


/*
 * A linked lists of cache lines all in the same offset within a page.
 * The probe code assumes next is the first element of the listEntry.
 */
struct listEntry {
  listEntry_t probe;
  listEntry_t prime;
  int data;
};

/*recording probing data in a cacheline*/
union cacheLine {
  struct listEntry le;
  char dummy[CACHELINE_SIZE];
};

struct page {
  union cacheLine cacheLines[LINES_PER_PAGE];
};

#define le2cl(le) ((union cacheLine *)(le))
#define cl2le(cl) ((listEntry_t )(cl))
/*address to colour number*/
#define adrs2clnum(adrs)	((((uint64_t)(adrs))& PAGE_MASK) >> CACHELINE_BITS)

/*the buffer for probing*/
struct page *buffer;



listEntry_t makeList(pageset_t ps, int line) {
  listEntry_t last = NULL;
  listEntry_t cur;

    /*Linking all target cache line from a pageset into a chain for prime + probe
     one backward, one forward*/
  for (int i = ps_size(ps); i--; ) {
    cur = LISTENTRY(ps_get(ps, i), line);
    cur->prime = last;
    if (cur->prime != NULL)
      cur->prime->probe = cur;
    cur->probe = NULL;
    last = cur;
  }

  return cur;
}

listEntry_t doPrime(listEntry_t le) {
    /*traversing the list, returning the head of the list*/
  while (le->prime != NULL)  {
    //le->data++;
    le = le->prime;
  }
  return le;
}

void printTable(pageset_t ps) {
  for (int i = ps_size(ps); --i; ) 
    printf("%d, ", ps_get(ps, i));
  putchar('\n');
}

void printlist(listEntry_t le) {
  while (le) {
    printf("%p, ", le);
    le = le->prime;
  }
  putchar ('\n');
}

/*traversing the prime list until the NULL pointer*/
void __attribute__ ((noinline)) prime(listEntry_t probeLink) {
  asm __volatile__ (
    "L4:			\n"
    "  incl 16(%0)               \n"
    "  mov 8(%0), %0		\n"
    "  test %0, %0		\n"
    "  jne L4			\n"
  : : "r" (probeLink) : );
}

/*probing the link list for 32 rounds, then probe the target cache line entry
 returning the time for visiting the probe elements in the entry. */
int  __attribute__ ((noinline)) probe1(listEntry_t probeLink, void *entry) {
  volatile int l = 0;

  if (probeLink == NULL) {
    return adrs2clnum(&probeLink);
  }

  asm __volatile__ (
    "  movl (%2), %%edi		\n"
    "  movq %1, %%rdi		\n"
    "  movl $32, %%esi		\n"
    "L1: 			\n"
    "  mov (%1),%1		\n"
    "  test %1, %1		\n"
    "  jne L1			\n"
    "  movq %%rdi, %1		\n"
    "  decl %%esi		\n"
    "  jg L1			\n"
    "  movq %2, %%rdi		\n"
    "  xorl %%eax, %%eax	\n"
    "  cpuid			\n"
    "  rdtsc			\n"
    "  movl %%eax, %%esi	\n"
    "  movl (%%rdi), %%edi	\n"
    "  rdtscp			\n"
    "  subl %%esi, %%eax	\n"
    "  movl %%eax, %%edi	\n"
    "  xorl %%eax, %%eax	\n"
    "  cpuid			\n"
    "  movl %%edi, %%eax	\n"
      :"=a" (l)
      :"b" (probeLink), "c" (entry)
      :"%edx","%esi", "%rdi");
  return l;

}

/*assuming 200 cycle is memory latency? thus the cache line is evicted 
 from L3 cache? */
/*traverse the probe list until NULL pointer, and return the number of entries that 
 are contained in level 2 cache*/
int __attribute__ ((noinline)) probe_timelist(listEntry_t le) {
  volatile int l = 0;
  /*
  asm(
    "  mfence                   \n"
    "  lfence                   \n"
    "  rdtsc			\n"
    "  movl %%eax, %%esi	\n"
    "L2: 			\n"
    "  mov (%1),%1		\n"
    "  test %1, %1		\n"
    "  jne L2			\n"
    "  rdtscp			\n"
    "  subl %%esi, %%eax	\n"
      :"=a" (l)
      :"b" (le)
      :"%edx","%esi", "%rdi");
      */
  asm __volatile__ (
    "  xorl %%edi, %%edi	\n"
    "L2:			\n"
    "  mfence                   \n"
    "  lfence                   \n"
    "  rdtsc			\n"
    "  movl %%eax, %%esi	\n"
    "  mov (%1),%1		\n"
    "  rdtscp			\n"
    "  subl %%esi, %%eax	\n"
    "  cmpl $200, %%eax		\n"
    "  jle L3			\n"
    "  incl %%edi		\n"
    "L3:			\n"
    "  test %1, %1		\n"
    "  jne L2			\n"
    "  movl %%edi, %0		\n"
      :"=a" (l)
      :"b" (le)
      :"%edx","%esi", "%rdi", "%ecx");
  return l;
}

int probe_time(void *p) {
  volatile int l = 0;
  asm(
    "  mfence                   \n"
    "  lfence                   \n"
    "  rdtsc			\n"
    "  movl %%eax, %%esi	\n"
    "  mov (%1),%1		\n"
    "  rdtscp			\n"
    "  subl %%esi, %%eax	\n"
      :"=a" (l)
      :"b" (p)
      :"%edx","%esi", "%rdi");
  return l;
}



static int line = -1;

void setrandline(int block) {
  char map[LINES_PER_PAGE];
  bzero(map, sizeof(map));
    
  map[adrs2clnum(probe1)] = 1;
  map[(adrs2clnum(probe1) + 1) % LINES_PER_PAGE] = 1;
  map[block] = 1;
  map[(block + 1) % LINES_PER_PAGE] = 1;
  map[(block +LINES_PER_PAGE - 1) % LINES_PER_PAGE] = 1;

  for (int i = 0; i < LINES_PER_PAGE; i++) {
    line = random() % LINES_PER_PAGE;
    if (map[line] == 0) {
      break;
    }
  }
}

void randline() {
  line = -1;
}

int probe(pageset_t pages, int cand, int num) {
    
    int t = probe1(NULL, NULL);
    if (t == line || t == ((line + 1) % LINES_PER_PAGE) || t == ((line + LINES_PER_PAGE - 1) % LINES_PER_PAGE))
        line = -1;
    
    if (line < 0)
        setrandline(t);
    
    
    if (num <=0)
        num = DEFAULT_RUNS;
    
    pageset_t ps = ps_dup(pages);
    listEntry_t cp = LISTENTRY(cand, line);
    int sum = 0;
    
    /*For N runs, prime the target cache line in the ps,
     then probe the existing ps */
    for (int i = 0; i < num; i++) {
        ps_randomise(ps);
        listEntry_t le = doPrime(makeList(ps, line));
        /*measuing the time for visiting the cp entry, after probing the le*/
        int t = probe1(le, cp);
        /*cp conflicts with le, 1000 cycles latency?*/
        if (t > 1000)
            i--;
        else
            sum += t;
    }
    ps_delete(ps);
  
    return (sum + num / 2) / num;
}

listEntry_t probe_makelist(pageset_t pages, void *adrs) {
  return makeList(pages, adrs2clnum(adrs));
}

static int npages;

int probe_npages() {
    
    /*npages in the probe buffer, created in probe_init*/
  return npages;
}


void probe_init(void *buf, int size) {
    
    /*init a buffer via mmap, size = L3 cache size * 2*/
    
  if (buffer != NULL) {
    fprintf(stderr, "probe_init: Already initialised\n");
    exit(1);
  }
  if (size == 0) {
    if (buf != NULL) {
      fprintf(stderr, "probe_init: missing buffer size\n");
      exit(1);
    }
    size = cpuid_l3size() * 2 / PAGE_SIZE;
  }
  npages = size;


  if (buf == NULL) {
    /*
    int shmid = shmget(SHMKEY, npages * PAGE_SIZE, 0);
    if (shmid < 0) {
      perror("shmget");
      exit(1);
    }
    buf = shmat(shmid, NULL, 0);
    if (buf == (void *) -1) {
      perror("shmat");
      exit(1);
    }
    */
      /*using UHUGETLB, avoiding noise*/
    buf = mmap(0, npages * PAGE_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_HUGETLB, -1, 0);
    if (buf == (void *) -1) {
      perror("mmap");
      exit(1);
    }
  }

  if ((((uint64_t)buf) & PAGE_MASK) != 0) {
    fprintf(stderr, "Buffer is not page aligned\n");
    exit(1);
  }
  buffer = buf;

}
