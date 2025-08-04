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
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

std::atomic_uint64_t progress;
std::atomic_uint64_t numbers_processed;
std::atomic_bool printing;
std::atomic_bool done;
std::atomic_uint64_t thread_count;

// Thread-safe work queue
class WorkQueue {
private:
    std::queue<uint64_t> queue;
    std::mutex mutex;
    std::condition_variable cv;
    bool shutdown = false;

public:
    void push(uint64_t value) {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push(value);
        cv.notify_one();
    }
    
    bool pop(uint64_t& value) {
        std::unique_lock<std::mutex> lock(mutex);
        cv.wait(lock, [this] { return !queue.empty() || shutdown; });
        if (shutdown && queue.empty()) return false;
        value = queue.front();
        queue.pop();
        return true;
    }
    
    void shutdown_queue() {
        std::lock_guard<std::mutex> lock(mutex);
        shutdown = true;
        cv.notify_all();
    }
    
    size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
};

WorkQueue work_queue;

// Forward declarations
uint64_t bin_exp(uint64_t base, uint64_t power, uint64_t mod);
int fast_fib(uint64_t n, uint64_t p);
int verify(uint64_t candidate);

void worker_thread() {
    uint64_t candidate;
    while (work_queue.pop(candidate)) {
        numbers_processed++;
        
        // Test Fermat primality first
        if (bin_exp(2, candidate-1, candidate) == 1) {
            // Test Fibonacci condition
            if (fast_fib(candidate+1, candidate) == 0) {
                try {
                    verify(candidate);
                    progress = candidate; // send to printing queue
                }
                catch (std::invalid_argument& e){
                    done = true;
                    work_queue.shutdown_queue();
                    std::cout << static_cast<uint64_t>(candidate) << " failed verification, not a prime." << std::endl;
                    return;
                }
            }
        }
    }
}

void printer(){
    initscr();
    uint64_t last_progress = 0;
    size_t last_queue_size = 0;
    uint64_t last_processed = 0;
    auto last_time = std::chrono::steady_clock::now();
    
    // For smoothed rate calculation
    uint64_t rate_start_processed = 0;
    auto rate_start_time = std::chrono::steady_clock::now();
    double smoothed_rate = 0.0;
    int rate_update_counter = 0;
    
    while (printing){
        uint64_t current_progress = progress.load();
        size_t current_queue_size = work_queue.size();
        uint64_t current_processed = numbers_processed.load();
        uint64_t current_threads = thread_count.load();
        auto current_time = std::chrono::steady_clock::now();
        
        // Update smoothed rate every 10 iterations (1 second)
        rate_update_counter++;
        if (rate_update_counter >= 10) {
            auto rate_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - rate_start_time).count();
            if (rate_elapsed > 0) {
                uint64_t rate_processed_diff = current_processed - rate_start_processed;
                smoothed_rate = (rate_processed_diff * 1000.0) / rate_elapsed;
            }
            rate_start_processed = current_processed;
            rate_start_time = current_time;
            rate_update_counter = 0;
        }
        
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_time).count();
        if (elapsed > 0) {
            uint64_t processed_diff = current_processed - last_processed;
            
            if (current_progress != last_progress || current_queue_size != last_queue_size || 
                processed_diff > 0) {
                clear();
                printw("Testing candidate primes...\n");
                printw("Threads: %lu\n", current_threads);
                printw("Latest candidate: %lu\n", current_progress);
                printw("Candidates in queue: %zu\n", current_queue_size);
                printw("Processing rate: %.1f numbers/sec\n", smoothed_rate);
                printw("Total processed: %lu\n", current_processed);
                refresh();
                last_progress = current_progress;
                last_queue_size = current_queue_size;
                last_processed = current_processed;
                last_time = current_time;
            }
        }
        usleep(250000); // Sleep for 0.25 seconds
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

    // Default to 1 thread, allow command line override
    unsigned int num_threads = 1;
    
    if (argc > 1) {
        num_threads = std::stoi(argv[1]);
        if (num_threads < 1) num_threads = 1;
        if (num_threads > 16) num_threads = 16; // Allow up to 16 threads via command line
    }
    
    std::cout << "Using " << num_threads << " computation threads" << std::endl;
    std::cout << "Starting PSW conjecture testing..." << std::endl;

    std::thread printer_thread(printer);
    progress = 0;
    numbers_processed = 0;
    thread_count = num_threads;
    printing = true;
    done = false;
    
    // Start worker threads
    std::vector<std::thread> workers;
    for (unsigned int i = 0; i < num_threads; ++i) {
        workers.emplace_back(worker_thread);
    }
    
    // Generate numbers and maintain target queue size
    int sign = -1;
    const size_t target_queue_size = 10000; // Target number of candidates in queue
    
    for (uint64_t i = 4294967295ULL; i < 18446744073709551615ULL && !done; i += (5 + sign)){
        // Wait if queue is too full
        while (work_queue.size() > target_queue_size * 2 && !done) {
            usleep(100); // Short wait
        }
        
        work_queue.push(i);
        sign *= -1;
        
        // Small delay to let workers process
        if (work_queue.size() > target_queue_size) {
            usleep(50);
        }
    }
    
    // Shutdown work queue and wait for workers
    work_queue.shutdown_queue();
    for (auto& worker : workers) {
        worker.join();
    }
    
    printing = false;
    printer_thread.join();
    
    if (!done) {
        std::cout << "All possible integers up to 64-bit limit checked." << std::endl;
    }
    
    return 0;

}

// MAC COMPILE:
// clang++ main.cpp -o main -I /opt/homebrew/include -L/opt/homebrew/lib -lgmp -lncurses -O3 -ffast-math -march=native

// LINUX COMPILE:
// g++ main.cpp -o main -lgmp -lncurses -O3 -ffast-math -march=native

// current progress: 9223372036854775807 / 18446744073709551615

// largest prime in 64 bits = 9223372036854775783

// whats happening with 3174114907?

// known prime for testing: 22855967

