#include <iostream>
#include <cstdlib>   // rand, srand
#include <ctime>     // time
#include <chrono>    // измерение времени

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// Последовательный подсчёт среднего
double averageSequential(int* arr, int n) {
    long long sum = 0;

    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }

    return static_cast<double>(sum) / n;
}

// Параллельный подсчёт среднего с OpenMP (reduction)
double averageParallel(int* arr, int n) {
    long long sum = 0;

#ifdef _OPENMP
    #pragma omp parallel for reduction(+:sum)
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
#else
    // Если OpenMP недоступен, считаем последовательно
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
#endif

    return static_cast<double>(sum) / n;
}

int main() {
    int n;
    cout << "Enter N (array size): ";
    if (!(cin >> n) || n <= 0) {
        cout << "Invalid size\n";
        return 1;
    }

    // 1. Динамический массив через указатель
    int* arr = new int[n];

    // 2. Заполняем массив случайными числами от 1 до 100
    srand(static_cast<unsigned int>(time(nullptr)));
    for (int i = 0; i < n; i++) {
        arr[i] = rand() % 100 + 1;
    }

    // При маленьком n выведем массив
    if (n <= 20) {
        cout << "Array: ";
        for (int i = 0; i < n; i++) {
            cout << arr[i] << " ";
        }
        cout << "\n";
    }

    // 3. Последовательное среднее + время (в миллисекундах)
    auto start_seq = chrono::high_resolution_clock::now();
    double avg_seq = averageSequential(arr, n);
    auto end_seq = chrono::high_resolution_clock::now();
    double time_seq_ms =
        chrono::duration<double, std::milli>(end_seq - start_seq).count();

    // 4. Параллельное среднее + время (в миллисекундах)
    auto start_par = chrono::high_resolution_clock::now();
    double avg_par = averageParallel(arr, n);
    auto end_par = chrono::high_resolution_clock::now();
    double time_par_ms =
        chrono::duration<double, std::milli>(end_par - start_par).count();

    // 5. Вывод в нужном формате
    cout << "Results:\n";
    cout << "Sequential average: " << avg_seq
         << ", time = " << time_seq_ms << " ms\n";
    cout << "Parallel average:   " << avg_par
         << ", time = " << time_par_ms << " ms\n";

#ifdef _OPENMP
    cout << "OpenMP is enabled, max threads: " << omp_get_max_threads() << "\n";
#else
    cout << "OpenMP not available (compiled without -fopenmp)\n";
#endif

    // 6. Освобождение памяти
    delete[] arr;

    return 0;
}
