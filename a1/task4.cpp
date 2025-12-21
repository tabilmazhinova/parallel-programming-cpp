#include <iostream>
#include <random>
#include <chrono>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace chrono;

int main() {
    const int N = 5000000;
    const int RUNS = 5; // сколько раз повторяем замер

    int* arr = new int[N];

    // Заполняем массив значениями 1..100
    mt19937 rng(123);
    uniform_int_distribution<int> dist(1, 100);

    for (int i = 0; i < N; i++) {
        arr[i] = dist(rng);
    }

#ifdef _OPENMP
    cout << "OpenMP ON, threads = " << omp_get_max_threads() << "\n";
#else
    cout << "OpenMP OFF\n";
#endif

    // Последовательный подсчёт среднего (RUNS прогонов)
    long long totalSeq = 0;
    double avgSeq = 0.0;

    for (int r = 0; r < RUNS; r++) {
        auto start = high_resolution_clock::now();

        long long sumSeq = 0;
        for (int i = 0; i < N; i++) {
            sumSeq += arr[i];
        }

        auto end = high_resolution_clock::now();
        totalSeq += duration_cast<microseconds>(end - start).count();

        avgSeq = static_cast<double>(sumSeq) / N;
    }

    // Параллельный подсчёт среднего через OpenMP reduction (RUNS прогонов)
    long long totalPar = 0;
    double avgPar = 0.0;

    for (int r = 0; r < RUNS; r++) {
        auto start = high_resolution_clock::now();

        long long sumPar = 0;

#ifdef _OPENMP
        #pragma omp parallel for reduction(+:sumPar)
#endif
        for (int i = 0; i < N; i++) {
            sumPar += arr[i];
        }

        auto end = high_resolution_clock::now();
        totalPar += duration_cast<microseconds>(end - start).count();

        avgPar = static_cast<double>(sumPar) / N;
    }

    // Среднее время по прогонам
    double avgTimeSeq = (double)totalSeq / RUNS;
    double avgTimePar = (double)totalPar / RUNS;

    cout << "Sequential average: " << avgSeq << "\n";
    cout << "Parallel average:   " << avgPar << "\n";
    cout << "Avg time (seq): " << avgTimeSeq << " us\n";
    cout << "Avg time (par): " << avgTimePar << " us\n";

    if (avgTimePar > 0.0) {
        cout << "Speedup: " << (avgTimeSeq / avgTimePar) << "x\n";
    }

    delete[] arr;
    return 0;
}
