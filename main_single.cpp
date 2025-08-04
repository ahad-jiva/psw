#include <stdexcept>
#include <cmath>
#include <iostream>
#include <gmp.h>
#include <thread>
#include <queue>
#include <ncurses.h>
#include <atomic>
#include <unistd.h>
#include <cstdint>

std::atomic_uint64_t progress;
std::atomic_bool printing;

void printer(){
    initscr();
    uint64_t last_progress = 0;
    while (printing){
        uint64_t current_progress = progress.load();
        if (current_progress != last_progress) {
            clear();
            printw("Testing candidate primes...\n");
            printw("Latest candidate: %lu\n", current_progress);
            refresh();
            last_progress = current_progress;
        }
        usleep(200000); // Sleep for 0.2 seconds
    }
    endwin();
}

int fast_fib(uint64_t n, uint64_t p) {
    mpz_t Fn, mod;
    mpz_inits(Fn, mod, NULL);
    mpz_set_ui(mod, p);

    mpz_t a, b, t1, t2, temp;
    mpz_inits(a, b, t1, t2, temp, NULL);

    mpz_set_ui(a, 0);
    mpz_set_ui(b, 1);

    for (int i = 63 - __builtin_clzll(n); i >= 0; --i) {
        // F(2k) = F(k) * [2*F(k+1) − F(k)]
        // F(2k+1) = F(k)^2 + F(k+1)^2

        // t1 = 2*b - a
        mpz_mul_ui(t1, b, 2);
        mpz_sub(t1, t1, a);
        mpz_mod(t1, t1, mod);

        // t1 = a * t1
        mpz_mul(t1, a, t1);
        mpz_mod(t1, t1, mod);  // t1 = F(2k)

        // t2 = a^2 + b^2
        mpz_mul(t2, a, a);
        mpz_mul(temp, b, b);
        mpz_add(t2, t2, temp);
        mpz_mod(t2, t2, mod);  // t2 = F(2k+1)

        if ((n >> i) & 1) {
            mpz_set(a, t2);        // F(n) = F(2k+1)
            mpz_add(b, t1, t2);    // F(n+1) = F(2k) + F(2k+1)
            mpz_mod(b, b, mod);
        } else {
            mpz_set(a, t1);        // F(n) = F(2k)
            mpz_set(b, t2);        // F(n+1) = F(2k+1)
        }
    }

    mpz_set(Fn, a);  // F(n) mod mod

    mpz_clears(a, b, t1, t2, temp, NULL);
    int result = mpz_get_ui(Fn);

    mpz_clears(Fn, mod, NULL);
    return result;
}

// fast integer square root
uint64_t isqrt(uint64_t n){
    if (n == 0) return 0;
    if (n < 4) return 1;
    
    // Use bit manipulation for faster initial approximation
    uint64_t x = 1ULL << ((63 - __builtin_clzll(n)) / 2);
    
    // Newton-Raphson iteration (usually converges in 2-3 steps)
    uint64_t y = (x + n / x) / 2;
    while (y < x) {
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

// deterministic primality verification
// using wheel factorization mod 30 with unrolled loop for speed
int verify(uint64_t candidate){

    // Handle special cases for small primes
    if (candidate == 3 || candidate == 5 || candidate == 7 || 
        candidate == 11 || candidate == 13 || candidate == 17 || 
        candidate == 19 || candidate == 23 || candidate == 29) {
        return 1;
    }
    
    if (candidate % 3 == 0) {
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
    }

    uint64_t sup = isqrt(candidate);

    for (uint64_t base = 0; base <= sup; base += 30){
        uint64_t d = 0;

        d = base + 29;
        if (d <= sup && d != candidate && candidate % d == 0) goto fail;

        d = base + 23;
        if (d != candidate && candidate % d == 0) goto fail; 

        d = base + 19;
        if (d != candidate && candidate % d == 0) goto fail; 

        d = base + 17;
        if (d != candidate && candidate % d == 0) goto fail;
        
        d = base + 13;
        if (d != candidate && candidate % d == 0) goto fail;

        d = base + 11;
        if (d != candidate && candidate % d == 0) goto fail;

        d = base + 7;
        if (d != candidate && candidate % d == 0) goto fail;

        d = base + 1;
        if (d > 5 && d != candidate && candidate % d == 0) goto fail;
      
    }

    return 1;

    fail:
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
}

// computes base^power % mod using binary exponentiation
// essentially Fermat primality test with base 2
uint64_t bin_exp(uint64_t base, uint64_t power, uint64_t mod){
    mpz_t result, b, m;
    mpz_inits(result, b, m, NULL);
    
    mpz_set_ui(result, 1);
    mpz_set_ui(b, base);
    mpz_set_ui(m, mod);
    
    mpz_mod(b, b, m);  // base % mod
    
    while (power > 0){
        if (power & 1){
            mpz_mul(result, result, b);
            mpz_mod(result, result, m);
        }
        mpz_mul(b, b, b);
        mpz_mod(b, b, m);
        power >>= 1;
    }
    
    uint64_t res = mpz_get_ui(result);
    mpz_clears(result, b, m, NULL);
    return res;
}

int main(int argc, char *argv[]){    // input: an odd integer p

    std::thread t(printer);
    progress = 0;
    printing = true;
    int sign = -1;
    for (uint64_t i = 2147483647ULL; i < 4294967295ULL; i += (5 + sign)){
        
        // Test Fermat primality first
        if (bin_exp(2, i-1, i) == 1) {
            
            // Test Fibonacci condition
            if (fast_fib(i+1, i) == 0) {

                try {
                    verify(i);
                    progress = i; // send to printing queue
                }
                catch (std::invalid_argument& e){
                    printing = false;
                    t.join(); // stop printing progress and kill thread
                    std::cout << static_cast<uint64_t>(i) << " failed verification, not a prime." << std::endl;
                    return 0;
                }
            }
        }
        sign *= -1;
    }
    printing = false;
    t.join();
    std::cout << "All possible integers up to 64-bit limit checked." << std::endl;
    return 0;

}

// MAC COMPILE:
// clang++ main_single.cpp -o main_single -I /opt/homebrew/include -L/opt/homebrew/lib -lgmp -lncurses -O3 -ffast-math -march=native

// LINUX COMPILE:
// g++ main_single.cpp -o main_single -lgmp -lncurses -O3 -ffast-math -march=native

// current progress: 9223372036854775807 / 18446744073709551615

// largest prime in 64 bits = 9223372036854775783

// whats happening with 3174114907?

// known prime for testing: 22855967 