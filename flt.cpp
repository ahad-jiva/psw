#include <stdlib.h>
#include <stdio.h>
#include <math.h>


// input: an odd integer p that is congruent to ± 2 mod 5
// run Fermat primality test with base 2

int flt(int argc, char *argv[]){

    int p = atoi(argv[1]); //TODO: bignum

    // check first condition
    int test = p % 5;
    if ((test != 2) || (test != 3)){
        printf("Integer is not congruent to ±2 mod 5");
        return 0;
    }
    else
    {
        int flt = 2 << p - 1;
        int mod = flt % p;
        if (mod == 1){
            printf("Integer passes FLT");
            return 1;
        }
    }
}