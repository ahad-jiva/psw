#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <iostream>

// input: an integer k and an odd number p, where p is congruent to ±2 mod 5
// output: the (k+1)th Fibonacci number mod p

int x = 10;

void func(int i){
    std::cout << "input is " << i << std::endl;
    std::cout << "heres a global var " << x << std::endl;
}

int isqrt(int n){
    int x = n;
    int y = (x + 1) / 2;
    while (y < x){
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}
int verify(int candidate){  // using wheel factorization mod 30 with unrolled loop

    if (candidate % 3 == 0) {
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
    }

    int sup = isqrt(candidate);

    for (int base = 0; base <= sup; base += 30){
        int d = 0;

        d = base + 1;
        if (d > 5 && d <= sup && candidate % d == 0) goto fail;

        d = base + 7;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 11;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 13;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 17;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 19;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 23;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 29;
        if (d <= sup && candidate % d == 0) goto fail;
    }

    return 1;

    fail:
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
}

int main(int argc, char* argv[]){
    int sign = -1;
    for (int i = 22855967; i < 2147483647; i += (5 + sign)){
        verify(i);
        sign *= -1;
    }
}

