#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>
#include <cmath>
#include <iostream>
#include <gmp.h>


// input: an integer n and a modulus p
// output: the nth Fibonacci number mod p
int fast_fib(int n, int p){
    
    mpz_t fib;
    mpz_t r;
    mpz_t mod;

    mpz_init(fib);
    mpz_init(r);
    mpz_init(mod);

    mpz_set_ui(mod, p);

    mpz_fib_ui(fib, n);
    mpz_fdiv_r(r, fib, mod);

    int return_val = mpz_get_ui(r);

    mpz_clear(fib);
    mpz_clear(r);
    mpz_clear(mod);

    return return_val;
}

int verify(int candidate){
    int sup = sqrt(candidate);
    for (int i = 3; i <= sup; i+=2){
        if (candidate % i == 0){
            std::cout << candidate << std::endl;
            throw std::invalid_argument(" not a prime\n");
        }
    }
    return 1;
}

// computes base^power % mod
int bin_exp(int base, int power, int mod){
    int result = 1;
    base %= mod;

    while (power > 0){
        if (power & 1){
            result = (1LL * result * base) % mod;
        }
        base = (1LL * base * base) % mod;
        power >>= 1;
    }
    return result;
}


// input: an odd integer p
// run Fermat primality test with base 2
int flt(int p){

    // check first condition (congruent to ±2 mod 5)
    int test = p % 5;
    if ((test == 2) || (test == 3)){
        return bin_exp(2, p-1, p);
    }
    else
    {
        return 0; // not congruent to ±2 mod 5
    }
}


int main(int argc, char *argv[]){    // input: an odd integer p

    for (int i = 37335333; i < 2147483647; i += 2){
        if (fast_fib(i+1, i) == 0 && flt(i) == 1){
            std::cout << i << " Passed PSW test. Verifying..." << std::endl;
            try {
                verify(i);
                std::cout << i << " verified as prime. 🟢" << std::endl;
            }
            catch (std::invalid_argument& e){
                std::cout << i << " Failed verification, not a prime. 🔴" << std::endl;
                return 0;
            }
        } 
    }
    return 0;

}

// HOW TO COMPILE:
// clang++ main.cpp -o main -I /opt/homebrew/include -L/opt/homebrew/lib -lgmp