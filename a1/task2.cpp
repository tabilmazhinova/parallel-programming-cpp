#include <iostream>
#include <random>      // генерация случайных чисел
#include <chrono>      // измерение времени

using namespace std;
using namespace chrono;

int main() {
    const int N = 1000000;

    // Динамическое выделение памяти для массива
    int* arr = new int[N];

    // Инициализация генератора случайных чисел
    // Значения будут в диапазоне от 1 до 100
    mt19937 rng(123);
    uniform_int_distribution<int> dist(1, 100);

    // Заполнение массива случайными числами
    for (int i = 0; i < N; i++) {
        arr[i] = dist(rng);
    }

    // Засекаем время начала выполнения алгоритма
    auto start = high_resolution_clock::now();

    // Инициализация минимального и максимального значений
    int minVal = arr[0];
    int maxVal = arr[0];

    // Последовательный поиск минимума и максимума
    for (int i = 1; i < N; i++) {
        if (arr[i] < minVal) {
            minVal = arr[i];
        }
        if (arr[i] > maxVal) {
            maxVal = arr[i];
        }
    }

    // Засекаем время окончания выполнения алгоритма
    auto end = high_resolution_clock::now();

    // Вычисляем время выполнения в микросекундах
    auto duration = duration_cast<microseconds>(end - start).count();

    // Вывод результатов
    cout << "Sequential Min: " << minVal << endl;
    cout << "Sequential Max: " << maxVal << endl;
    cout << "Time: " << duration << " us" << endl;

    // Освобождение выделенной памяти
    delete[] arr;

    return 0;
}
