#include <stdint.h>
#include "clock.h"


unsigned long gettime() {
  volatile unsigned long time;
  asm ("rdtsc": "=a" (time): : "%edx" );
  return time;
}


/*question: the timestamp counter is 64 bits, only considering the lower 32 bits
 neglects the overflow in the lower part.*/
/* Answer: As log as the delay is less than 2^32 we can use simple modular arithmetic on 32 
 * bit integers. This avoids the overhead of looking at the MSB 32 bits */

void delay(uint32_t start, uint32_t cycles) {
    /*spinning  for N cycles*/
    
    if (start == 0)
        start = gettime();
    while (gettime() - start < cycles)
        ;
}


