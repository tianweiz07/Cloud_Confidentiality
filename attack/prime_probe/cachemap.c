#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
//#include <libcpuid.h>

#include "pageset.h"
#include "probe.h"
#include "cpuid.h"


#define L3_THRESHOLD 200


int debug = 0;


int mapsize(pageset_t ps) {
    
    /*get the maximum page number in a pageset*/
  int max = 0;
  for (int i = ps_size(ps); i--; )  {
    int t = ps_get(ps, i);
    if (t > max)
      max = t;
  }
  return max + 1;
}

short *makemap(pageset_t ps) {
 
    /*creating mapping arrary*/
  int ms = mapsize(ps);
  short *rv = malloc(ms * sizeof(short));
  for (int i = 0; i < ms; i++)
    rv[i] = -1;
  return rv;
}

short *splitPages(pageset_t ps) {
    
    /*split page into colors, using prime + probe for finding cache set conflicts
     ,return page-color mapping*/
    
    short *map = makemap(ps);
    int ncolours = 0;
    
    /*create a new pageset*/
    pageset_t cache = ps_new();
    ps_push(cache, ps_pop(ps));
    
    while (ps_size(ps) != 0) {
        /*adding a page into the "cache" pageset?*/
        int cand = ps_pop(ps);
        int time = probe(cache, cand, 0);
        
        if (debug) printf("%d: %d\n", ps_size(cache), time);
        
        /*cache set conflict?*/
        if (time > L3_THRESHOLD) {
            pageset_t c2 = ps_dup(cache);
            int set = -1;
            int new = 0;
            /*find which page in "cache" causes the conflict*/
            for (int i = 0; i < ps_size(cache); i++) {
                int page = ps_get(cache, i);
                ps_remove(c2, page);
                if (probe(c2, cand, 0) <= L3_THRESHOLD) {
                    if (map[page] >= 0) {
                        if (map[cand] != -1) {
                            printf("OOPS - bad probe on %d\n", page);
                            exit(1);
                        }
                        map[cand] = map[page];
                        break;
                    }
                    if (map[cand] == -1) {
                        if (debug) printf("%d: Allocating colour %d: ", ps_size(cache), ncolours);
                        /*find a new colour*/
                        map[cand] = ncolours++;
                        new = 1;
                    }
                    if (debug) printf("%d ", page);
                    map[page] = map[cand];
                }
                ps_push(c2, page);
            }
            if (debug && new)
                printf("\n");
            ps_delete(c2);
        } else {
            ps_push(cache, cand);
        }
    }
    
    return map;
}



void checkCache() {
  /*
  if (!cpuid_present()) {
    fprintf(stderr, "Your CPU does not support CPUID\n");
    exit(1);
  }
  struct cpu_raw_data_t raw; 
  struct cpu_id_t data;
  if (cpuid_get_raw_data(&raw) < 0) { 
    fprintf(stderr, "Sorry, cannot get the CPUID raw data.\n");
    fprintf(stderr, "Error: %s\n", cpuid_error()); 
    exit(1);
  }
  cpu_identify(&raw, &data);
  printf("Found: %s CPU\n", data.vendor_str);                
  printf("Processor model is `%s'\n", data.cpu_codename);     
  printf("The full brand string is `%s'\n", data.brand_str);    
  printf("The processor has %dK L1 cache, %dK L2 cache, %dK L3 cache\n", data.l1_data_cache, data.l2_cache, data.l3_cache);
  printf("The processor has %d cores and %d logical processors\n", data.num_cores, data.num_logical_cpus);                       
  */

  printf("size:%d (%dK), colours %d, assoc:%d\n", cpuid_l3size(), cpuid_l3size()/1024, cpuid_l3colours(), cpuid_l3assoc());
}


pageset_t *cachemap() {
    
    int npages = probe_npages();
    int colours = cpuid_l3colours();
    npages = (npages/32) * 32;
    
    /*selecting the first page in 32 pages, randomise the pageset stack. 
     These are the pages that may competing colours (representing colours), 
     according to sandy bridge hashing function*/
    pageset_t allPages = ps_new();
    for (int i = 0; i < npages; i+=32)
        ps_push(allPages, i);
    
    ps_randomise(allPages);
    
    /*obtain page-colour mapping*/
    short *map = splitPages(allPages);
    ps_delete(allPages);
    
    pageset_t *tmp = malloc(colours * sizeof(pageset_t));
    pageset_t *rv = malloc(colours * sizeof(pageset_t));
    for (int i = colours; i--;)
        tmp[i] = ps_new();
    
    /*seperating page into colours, sequential allocation with 32 pages*/
    /* one slice has 32 colours, map is actually the slice number */
    /* there are 32*slice l3 colours, each colour has a pageset */
    for (int i = 0; i < npages; i+=32)
        for (int j = 0; j < 32; j++) {
            ps_push(tmp[j+map[i]*32], i+j);
            if (i + j < colours)
                rv[i+j] = tmp[j+map[i]*32];
            
        }
    free(map);
    free(tmp);
    return rv;
}
