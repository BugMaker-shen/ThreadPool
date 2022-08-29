#include <iostream>
#include <functional>
#include <thread>
#include <future>
#include "threadpool.h"

using namespace std;

void sum1(int a, int b) {
    a + b;
    // return a + b;
}

void sum2(int a, int b) {
    int sum = 0;
    for (int i = a; i <= b; ++i) {
        sum += i;
    }
    // return sum;
}

int main(){
    ThreadPool pool;
    pool.start(2);
    
    future<void> res1 = pool.submitTask(sum1, 1, 2);
    future<void> res2 = pool.submitTask(sum2, 1, 100);
    future<void> res3 = pool.submitTask(sum2, 1, 1000);

    // cout << res1.get() << endl;
    // cout << res2.get() << endl;
    // cout << res3.get() << endl;
}
