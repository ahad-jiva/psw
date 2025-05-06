#include <stdlib.h>
#include <stdio.h>

// input: an integer k and an odd number p, where p is congruent to ±2 mod 5
// output: the (k+1)th Fibonacci number mod p

int main(int argc, char* argv[]){

    int fib_l = atoi(argv[1]) + 1;
    int mod = atoi(argv[2]);

    int fib[fib_l];

    fib[0] = 0;
    fib[1] = 1;

    for (int i = 2; i <= fib_l; i++){
        fib[i] = (fib[i-1] + fib[i-2]) % mod;
    }

    printf("%d\n", fib[fib_l]);
    

}