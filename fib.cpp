#include <stdlib.h>
#include <stdio.h>
#include <thread>
#include <iostream>
//#include <gmp.h>

// input: an integer k and an odd number p, where p is congruent to ±2 mod 5
// output: the (k+1)th Fibonacci number mod p

int x = 10;

void func(int i){
    std::cout << "input is " << i << std::endl;
    std::cout << "heres a global var " << x << std::endl;
}

unsigned int isqrt(unsigned int n){
    unsigned int x = n;
    unsigned int y = (x + 1) / 2;
    while (y < x){
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

// Computes (F(n), F(n+1)) mod `mod` using fast doubling
// void fib_mod(mpz_t Fn, int n, const mpz_t mod) {
//     mpz_t a, b, t1, t2, temp;
//     mpz_inits(a, b, t1, t2, temp, NULL);

//     mpz_set_ui(a, 0);  // F(0)
//     mpz_set_ui(b, 1);  // F(1)

//     for (int i = 31 - __builtin_clz(n); i >= 0; --i) {
//         // Loop invariant: a = F(k), b = F(k+1)
//         // Compute:
//         // F(2k) = F(k) * [2*F(k+1) − F(k)]
//         // F(2k+1) = F(k)^2 + F(k+1)^2

//         // t1 = 2*b - a
//         mpz_mul_ui(t1, b, 2);
//         mpz_sub(t1, t1, a);
//         mpz_mod(t1, t1, mod);

//         // t1 = a * t1
//         mpz_mul(t1, a, t1);
//         mpz_mod(t1, t1, mod);  // t1 = F(2k)

//         // t2 = a^2 + b^2
//         mpz_mul(t2, a, a);
//         mpz_mul(temp, b, b);
//         mpz_add(t2, t2, temp);
//         mpz_mod(t2, t2, mod);  // t2 = F(2k+1)

//         if ((n >> i) & 1) {
//             mpz_set(a, t2);        // F(n) = F(2k+1)
//             mpz_add(b, t1, t2);    // F(n+1) = F(2k) + F(2k+1)
//             mpz_mod(b, b, mod);
//         } else {
//             mpz_set(a, t1);        // F(n) = F(2k)
//             mpz_set(b, t2);        // F(n+1) = F(2k+1)
//         }
//     }

//     mpz_set(Fn, a);  // F(n) mod mod

//     mpz_clears(a, b, t1, t2, temp, NULL);
// }

// // Main wrapper: returns F(n) mod p
// int fast_fib(int n, int p) {
//     mpz_t Fn, mod;
//     mpz_inits(Fn, mod, NULL);
//     mpz_set_ui(mod, p);

//     fib_mod(Fn, n, mod);
//     int result = mpz_get_ui(Fn);

//     mpz_clears(Fn, mod, NULL);
//     return result;
// }

int verify(unsigned int candidate){

    if (candidate % 3 == 0) {
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
    }

    unsigned int sup = isqrt(candidate);

    for (unsigned int base = 0; base <= sup; base += 30){
        unsigned int d = 0;

        d = base + 29;
        if (d <= sup && candidate % d == 0) goto fail;

        d = base + 23;
        if (candidate % d == 0) goto fail; 

        d = base + 19;
        if (candidate % d == 0) goto fail; 

        d = base + 17;
        if (candidate % d == 0) goto fail;
        
        d = base + 13;
        if (candidate % d == 0) goto fail;

        d = base + 11;
        if (candidate % d == 0) goto fail;

        d = base + 7;
        if (candidate % d == 0) goto fail;

        d = base + 1;
        if (d > 5 && candidate % d == 0) goto fail;
      
    }

    return 1;

    fail:
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
}


int main(int argc, char* argv[]){
    std::cout << verify(3174114907) << std::endl;
    return 0;
}

