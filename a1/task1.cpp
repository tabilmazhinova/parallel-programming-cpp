#include <iostream>
#include <cstdlib>   // rand, srand
#include <ctime>     // time

using namespace std;

int main() {
    const int N = 50000;

    // динамическое выделение памяти
    int* arr = new int[N];

    // инициализация генератора случайных чисел
    srand(time(nullptr));

    // заполнение массива значениями от 1 до 100
    for (int i = 0; i < N; i++) {
        arr[i] = rand() % 100 + 1;
    }

    // вычисление среднего значения
    long long sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }

    double average = static_cast<double>(sum) / N;
    cout << "Average value: " << average << endl;

    // освобождение памяти
    delete[] arr;

    return 0;
}
