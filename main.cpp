#include <stdexcept>
#include <cmath>
#include <iostream>
#include <gmp.h>
#include <thread>
#include <queue>
#include <ncurses.h>
#include <atomic>
#include <unistd.h>

std::atomic_uint progress;
std::atomic_bool printing;

void printer(){
    initscr();
    while (printing){
        printw("Testing candidate primes...\n");
        printw("%u passed tests and verified.", progress.load());
        refresh();
        clear();
        usleep(10000);
    }
    endwin();
}

// input: an integer n and a modulus p
// output: the nth Fibonacci number mod p
// int fast_fib(int n, int p){
    
//     mpz_t fib;
//     mpz_t r;
//     mpz_t mod;

//     mpz_init(fib);
//     mpz_init(r);
//     mpz_init(mod);

//     mpz_set_ui(mod, p);

//     mpz_fib_ui(fib, n);
//     mpz_fdiv_r(r, fib, mod);

//     int return_val = mpz_get_ui(r);

//     mpz_clear(fib);
//     mpz_clear(r);
//     mpz_clear(mod);

//     return return_val;
// }

// void fib_mod(mpz_t Fn, int n, const mpz_t mod) {
//     mpz_t a, b, t1, t2, temp;
//     mpz_inits(a, b, t1, t2, temp, NULL);

//     mpz_set_ui(a, 0);
//     mpz_set_ui(b, 1);

//     for (int i = 31 - __builtin_clz(n); i >= 0; --i) {
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

int fast_fib(unsigned int n, unsigned int p) {
    mpz_t Fn, mod;
    mpz_inits(Fn, mod, NULL);
    mpz_set_ui(mod, p);

    mpz_t a, b, t1, t2, temp;
    mpz_inits(a, b, t1, t2, temp, NULL);

    mpz_set_ui(a, 0);
    mpz_set_ui(b, 1);

    for (int i = 31 - __builtin_clz(n); i >= 0; --i) {
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

unsigned int isqrt(unsigned int n){
    unsigned int x = n;
    unsigned int y = (x + 1) / 2;
    while (y < x){
        x = y;
        y = (x + n / x) / 2;
    }
    return x;
}

int verify(unsigned int candidate){  // using wheel factorization mod 30 with unrolled loop for speed

    if (candidate % 3 == 0) {
        std::cout << candidate << std::endl;
        throw std::invalid_argument(" not a prime\n");
    }

    int sup = isqrt(candidate);

    for (int base = 0; base <= sup; base += 30){ // TODO: invert conditions for fewer d<=sup checks
        int d = 0;

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

// computes base^power % mod using binary exponentiation
// essentially Fermat primality test with base 2
int bin_exp(int base, unsigned int power, unsigned int mod){
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


// // input: an odd integer p
// // run Fermat primality test with base 2
// int flt(int p){

//     // check first condition (congruent to ±2 mod 5)
//     int test = p % 5;
//     if ((test == 2) || (test == 3)){
//         return bin_exp(2, p-1, p);
//     }
//     else
//     {
//         return 0; // not congruent to ±2 mod 5
//     }
// }


int main(int argc, char *argv[]){    // input: an odd integer p

    std::thread t(printer);
    progress = 0;
    printing = true;
    int sign = -1;
    for (unsigned int i = 2147483647; i < 4294967295; i += (5 + sign)){
        if (bin_exp(2, i-1, i) == 1 && fast_fib(i+1, i) == 0){
            try {
                verify(i);
                progress = i; // send to printing queue
            }
            catch (std::invalid_argument& e){
                printing = false;
                t.join(); // stop printing progress and kill thread
                std::cout << i << " failed verification, not a prime. ❌" << std::endl;
                return 0;
            }
        }
        sign *= -1;
    }
    printing = false;
    t.join();
    std::cout << "All possible integers up to 32-bit limit checked." << std::endl;
    return 0;

}

// MAC COMPILE:
// clang++ main.cpp -o main -I /opt/homebrew/include -L/opt/homebrew/lib -lgmp -lncurses -O3 -ffast-math -march=native

// LINUX COMPILE:
// g++ main.cpp -o main -lgmp -lncurses -O3 -ffast-math -march=native

// current progress: 2147483647 / 4294967295