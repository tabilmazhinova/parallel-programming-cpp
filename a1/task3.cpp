#include <iostream>    // ввод и вывод (cout, endl)
#include <random>      // генерация случайных чисел (mt19937, distribution)
#include <chrono>      // измерение времени выполнения
#include <limits>      // numeric_limits<T>::min / max

#ifdef _OPENMP
#include <omp.h>       // библиотека OpenMP
#endif

using namespace std;    // упрощаем доступ к стандартной библиотеке
using namespace chrono; // упрощаем работу с таймерами

int main() {
    const int N = 1000000;   // размер массива (1 миллион элементов)

    // ==================== СОЗДАНИЕ И ЗАПОЛНЕНИЕ МАССИВА ====================

    // Выделяем память под массив в куче
    int* arr = new int[N];

    // Инициализируем генератор случайных чисел
    mt19937 rng(123);        // фиксированный seed (результаты воспроизводимы)

    // Равномерное распределение чисел в диапазоне [1; 100]
    uniform_int_distribution<int> dist(1, 100);

    // Заполняем массив случайными числами
    for (int i = 0; i < N; i++) {
        arr[i] = dist(rng);  // генерация очередного случайного значения
    }

    // ==================== ПОСЛЕДОВАТЕЛЬНЫЙ ПОИСК ====================

    // Засекаем начало последовательного алгоритма
    auto startSeq = high_resolution_clock::now();

    // Начальные значения минимума и максимума
    int minSeq = arr[0];     // предполагаем, что первый элемент — минимум
    int maxSeq = arr[0];     // и максимум

    // Последовательный проход по массиву
    for (int i = 1; i < N; i++) {
        if (arr[i] < minSeq) minSeq = arr[i];  // обновляем минимум
        if (arr[i] > maxSeq) maxSeq = arr[i];  // обновляем максимум
    }

    // Засекаем конец последовательного алгоритма
    auto endSeq = high_resolution_clock::now();

    // Считаем время выполнения в микросекундах
    auto timeSeq =
        duration_cast<microseconds>(endSeq - startSeq).count();

    // ==================== ПАРАЛЛЕЛЬНЫЙ ПОИСК (OpenMP) ====================

    // Глобальные минимум и максимум
    int globalMin = numeric_limits<int>::max(); // начальное значение — максимум int
    int globalMax = numeric_limits<int>::min(); // начальное значение — минимум int

    // Засекаем начало параллельного алгоритма
    auto startPar = high_resolution_clock::now();

#ifdef _OPENMP
    #pragma omp parallel                     // создаём команду потоков
    {
        // Локальные значения минимума и максимума для каждого потока
        int localMin = numeric_limits<int>::max();
        int localMax = numeric_limits<int>::min();

        #pragma omp for nowait                // делим цикл по потокам, без ожидания в конце
        for (int i = 0; i < N; i++) {
            if (arr[i] < localMin) localMin = arr[i]; // локальный минимум
            if (arr[i] > localMax) localMax = arr[i]; // локальный максимум
        }

        #pragma omp critical                  // критическая секция
        {
            // безопасно обновляем глобальные значения
            if (localMin < globalMin) globalMin = localMin;
            if (localMax > globalMax) globalMax = localMax;
        }
    }
#else
    // Если OpenMP не поддерживается — обычный последовательный проход
    for (int i = 0; i < N; i++) {
        if (arr[i] < globalMin) globalMin = arr[i];
        if (arr[i] > globalMax) globalMax = arr[i];
    }
#endif

    // Засекаем конец параллельного алгоритма
    auto endPar = high_resolution_clock::now();

    // Считаем время выполнения параллельного алгоритма
    auto timePar =
        duration_cast<microseconds>(endPar - startPar).count();

    // ==================== ВЫВОД РЕЗУЛЬТАТОВ ====================

    cout << "Sequential Min: " << minSeq << endl;      // минимум (seq)
    cout << "Sequential Max: " << maxSeq << endl;      // максимум (seq)
    cout << "Sequential Time: " << timeSeq << " us\n\n"; // время (seq)

    cout << "Parallel Min: " << globalMin << endl;     // минимум (par)
    cout << "Parallel Max: " << globalMax << endl;     // максимум (par)
    cout << "Parallel Time: " << timePar << " us" << endl; // время (par)

#ifdef _OPENMP
    cout << "Threads used: " << omp_get_max_threads() << endl; // число потоков
#endif

    // Подсчёт ускорения (speedup)
    if (timePar > 0) {
        cout << "Speedup: "
             << (double)timeSeq / timePar << "x" << endl;
    }

    // ==================== ОСВОБОЖДЕНИЕ ПАМЯТИ ====================

    delete[] arr;            // освобождаем динамически выделенную память

    return 0;                // успешное завершение программы
}
