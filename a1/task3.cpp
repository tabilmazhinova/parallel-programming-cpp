#include <iostream>
#include <random>
#include <chrono>
#include <limits>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace chrono;

int main() {
    const int N = 1000000;

    // Динамическое выделение памяти
    int* arr = new int[N];

    // Генерация случайных чисел 1..100
    mt19937 rng(123);
    uniform_int_distribution<int> dist(1, 100);

    for (int i = 0; i < N; i++) {
        arr[i] = dist(rng);
    }

    // ==================== SEQUENTIAL ====================
    auto startSeq = high_resolution_clock::now();

    int minSeq = arr[0];
    int maxSeq = arr[0];

    for (int i = 1; i < N; i++) {
        if (arr[i] < minSeq) minSeq = arr[i];
        if (arr[i] > maxSeq) maxSeq = arr[i];
    }

    auto endSeq = high_resolution_clock::now();
    auto timeSeq = duration_cast<microseconds>(endSeq - startSeq).count();

    // ==================== PARALLEL (OpenMP) ====================
    int globalMin = numeric_limits<int>::max();
    int globalMax = numeric_limits<int>::min();

    auto startPar = high_resolution_clock::now();

#ifdef _OPENMP
    #pragma omp parallel
    {
        int localMin = numeric_limits<int>::max();
        int localMax = numeric_limits<int>::min();

        #pragma omp for nowait
        for (int i = 0; i < N; i++) {
            if (arr[i] < localMin) localMin = arr[i];
            if (arr[i] > localMax) localMax = arr[i];
        }

        #pragma omp critical
        {
            if (localMin < globalMin) globalMin = localMin;
            if (localMax > globalMax) globalMax = localMax;
        }
    }
#else
    for (int i = 0; i < N; i++) {
        if (arr[i] < globalMin) globalMin = arr[i];
        if (arr[i] > globalMax) globalMax = arr[i];
    }
#endif

    auto endPar = high_resolution_clock::now();
    auto timePar = duration_cast<microseconds>(endPar - startPar).count();

    // ==================== OUTPUT ====================
    cout << "Sequential Min: " << minSeq << endl;
    cout << "Sequential Max: " << maxSeq << endl;
    cout << "Sequential Time: " << timeSeq << " us\n\n";

    cout << "Parallel Min: " << globalMin << endl;
    cout << "Parallel Max: " << globalMax << endl;
    cout << "Parallel Time: " << timePar << " us" << endl;

#ifdef _OPENMP
    cout << "Threads used: " << omp_get_max_threads() << endl;
#endif

    if (timePar > 0) {
        cout << "Speedup: " << (double)timeSeq / timePar << "x" << endl;
    }

    delete[] arr;
    return 0;
}
