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

int main(int argc, char* argv[]){
    x = 11;
    std::thread t(func, 18);

    t.join();
    std::cout << "main thread finished" << std::endl;
    return 0;
}