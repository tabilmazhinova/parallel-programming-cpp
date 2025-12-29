#include <iostream>   // ввод/вывод: cout, endl
#include <random>     // генератор случайных чисел: mt19937, distribution
#include <chrono>     // замер времени: high_resolution_clock, duration_cast

#ifdef _OPENMP
#include <omp.h>      // OpenMP функции (threads и т.п.)
#endif

using namespace std;    // чтобы не писать std::
using namespace chrono; // чтобы не писать chrono::

int main() {
    const int N = 5000000;     // размер массива (5 миллионов элементов)
    const int RUNS = 5;        // сколько раз повторяем замер (для более стабильного времени)

    int* arr = new int[N];     // выделяем память под массив в куче (heap)

    // ==================== ЗАПОЛНЕНИЕ МАССИВА ====================

    // Заполняем массив значениями 1..100
    mt19937 rng(123);                          // генератор (фиксированный seed = одинаковые данные каждый запуск)
    uniform_int_distribution<int> dist(1, 100); // равномерное распределение чисел в диапазоне [1; 100]

    for (int i = 0; i < N; i++) {              // цикл по всем элементам массива
        arr[i] = dist(rng);                    // генерируем случайное число и сохраняем в массив
    }

#ifdef _OPENMP
    cout << "OpenMP ON, threads = "            // если OpenMP включён
         << omp_get_max_threads() << "\n";     // выводим максимальное число потоков
#else
    cout << "OpenMP OFF\n";                    // если OpenMP выключен (компиляция без -fopenmp)
#endif

    // ==================== SEQUENTIAL (последовательно) ====================

    // Последовательный подсчёт среднего (RUNS прогонов)
    long long totalSeq = 0;    // суммарное время всех прогонов (в микросекундах)
    double avgSeq = 0.0;       // последнее вычисленное среднее (по факту одинаковое каждый прогон)

    for (int r = 0; r < RUNS; r++) {           // повторяем RUNS раз для усреднения времени
        auto start = high_resolution_clock::now(); // старт таймера

        long long sumSeq = 0;                  // сумма элементов (последовательная)
        for (int i = 0; i < N; i++) {          // обычный цикл по массиву
            sumSeq += arr[i];                  // прибавляем текущий элемент к сумме
        }

        auto end = high_resolution_clock::now(); // конец таймера
        totalSeq += duration_cast<microseconds>(end - start).count(); // добавляем время этого прогона в totalSeq

        avgSeq = static_cast<double>(sumSeq) / N; // считаем среднее (double, чтобы деление было не целочисленным)
    }

    // ==================== PARALLEL (параллельно) ====================

    // Параллельный подсчёт среднего через OpenMP reduction (RUNS прогонов)
    long long totalPar = 0;    // суммарное время параллельных прогонов
    double avgPar = 0.0;       // среднее значение (параллельная версия)

    for (int r = 0; r < RUNS; r++) {           // повторяем RUNS раз
        auto start = high_resolution_clock::now(); // старт таймера

        long long sumPar = 0;                  // общая сумма для reduction

#ifdef _OPENMP
        #pragma omp parallel for reduction(+:sumPar) // параллельный цикл + безопасная сумма через reduction
#endif
        for (int i = 0; i < N; i++) {          // каждый поток берёт свой кусок индексов
            sumPar += arr[i];                  // локально суммирует, а потом OpenMP объединит всё в sumPar
        }

        auto end = high_resolution_clock::now(); // конец таймера
        totalPar += duration_cast<microseconds>(end - start).count(); // добавляем время прогона

        avgPar = static_cast<double>(sumPar) / N; // считаем среднее значение
    }

    // ==================== ИТОГИ (усреднение времени) ====================

    // Среднее время по прогонам (чтобы было меньше влияния случайных колебаний)
    double avgTimeSeq = (double)totalSeq / RUNS; // среднее время seq (в микросекундах)
    double avgTimePar = (double)totalPar / RUNS; // среднее время par (в микросекундах)

    // Вывод результатов
    cout << "Sequential average: " << avgSeq << "\n"; // среднее значение (seq)
    cout << "Parallel average:   " << avgPar << "\n"; // среднее значение (par)
    cout << "Avg time (seq): " << avgTimeSeq << " us\n"; // среднее время seq
    cout << "Avg time (par): " << avgTimePar << " us\n"; // среднее время par

    // Считаем ускорение (speedup)
    if (avgTimePar > 0.0) {                        // защита от деления на 0
        cout << "Speedup: " << (avgTimeSeq / avgTimePar) << "x\n"; // во сколько раз быстрее
    }

    delete[] arr;       // освобождаем память массива (обязательно для new[])
    return 0;           // успешное завершение программы
}
