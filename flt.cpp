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

// int verify(int candidate){  // using wheel factorization mod 30 with unrolled loop

//     if (candidate % 3 == 0) {
//         std::cout << candidate << std::endl;
//         throw std::invalid_argument(" not a prime\n");
//     }

//     int sup = isqrt(candidate);

//     for (int base = 0; base <= sup; base += 30){
//         int d = 0;

//         d = base + 1;
//         if (d > 5 && d <= sup && candidate % d == 0) goto fail;

//         d = base + 7;
//         if (d <= sup && candidate % d == 0) goto fail;

//         d = base + 11;
//         if (d <= sup && candidate % d == 0) goto fail;

//         d = base + 13;
//         if (d <= sup && candidate % d == 0) goto fail;

//         d = base + 17;
//         if (d <= sup && candidate % d == 0) goto fail;

//         d = base + 19;
//         if (d <= sup && candidate % d == 0) goto fail;

//         d = base + 23;
//         if (d <= sup && candidate % d == 0) goto fail;

//         d = base + 29;
//         if (d <= sup && candidate % d == 0) goto fail;
//     }

//     return 1;

//     fail:
//         std::cout << candidate << std::endl;
//         throw std::invalid_argument(" not a prime\n");
// }