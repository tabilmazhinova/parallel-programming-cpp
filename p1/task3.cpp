#include <iostream>   // ввод/вывод: cout, cin
#include <cstdlib>    // rand(), srand()
#include <ctime>      // time() для зерна генератора случайных чисел
#include <chrono>     // точное измерение времени выполнения

#ifdef _OPENMP         // если компилируем с OpenMP (например, -fopenmp)
#include <omp.h>       // функции OpenMP (omp_get_max_threads и т.д.)
#endif

using namespace std;   // чтобы не писать std::cout, std::cin, std::chrono и т.п.

// Последовательный подсчёт среднего значения массива
double averageSequential(int* arr, int n) {     // функция получает указатель на массив и его размер
    long long sum = 0;                          // сумма (long long, чтобы не было переполнения на больших n)

    for (int i = 0; i < n; i++) {               // идём по всем элементам
        sum += arr[i];                          // добавляем текущий элемент к сумме
    }

    return static_cast<double>(sum) / n;        // переводим sum в double и делим на n -> среднее
}

// Параллельный подсчёт среднего (OpenMP + reduction)
double averageParallel(int* arr, int n) {       // то же самое, но с распараллеливанием
    long long sum = 0;                          // общая сумма (будет собираться через reduction)

#ifdef _OPENMP                                     // если OpenMP доступен
    #pragma omp parallel for reduction(+:sum)      // параллельный for + суммирование sum со всех потоков
    for (int i = 0; i < n; i++) {                  // каждый поток берёт часть индексов i
        sum += arr[i];                             // суммируем, OpenMP потом аккуратно объединит частичные суммы
    }
#else                                              // если OpenMP не подключен
    // Если OpenMP недоступен, считаем последовательно (чтобы программа всё равно работала)
    for (int i = 0; i < n; i++) {                  // обычный цикл
        sum += arr[i];                             // обычная сумма
    }
#endif

    return static_cast<double>(sum) / n;           // возвращаем среднее значение
}

int main() {                                       // точка входа программы
    int n;                                         // размер массива
    cout << "Enter N (array size): ";              // просим ввести N

    if (!(cin >> n) || n <= 0) {                   // проверяем ввод: число ли ввели и > 0 ли оно
        cout << "Invalid size\n";                  // сообщение об ошибке
        return 1;                                  // выходим с кодом ошибки
    }

    // 1) Динамический массив (вручную, через new)
    int* arr = new int[n];                         // выделяем память под n int'ов (в куче/heap)

    // 2) Заполняем массив случайными числами от 1 до 100
    srand(static_cast<unsigned int>(time(nullptr))); // задаём зерно (seed) = текущее время, чтобы rand был "разный"
    for (int i = 0; i < n; i++) {                    // пробегаем массив
        arr[i] = rand() % 100 + 1;                   // число 1..100 (остаток 0..99 + 1)
    }

    // Если массив маленький, показываем его (чтобы можно было проверить глазами)
    if (n <= 20) {                                // условие для вывода
        cout << "Array: ";                        // подпись
        for (int i = 0; i < n; i++) {             // печатаем каждый элемент
            cout << arr[i] << " ";                // вывод элемента + пробел
        }
        cout << "\n";                             // перевод строки
    }

    // 3) Последовательное среднее + измерение времени (мс)
    auto start_seq = chrono::high_resolution_clock::now(); // старт таймера (последовательно)
    double avg_seq = averageSequential(arr, n);             // считаем среднее последовательно
    auto end_seq = chrono::high_resolution_clock::now();    // конец таймера
    double time_seq_ms =                                   // переменная под время в миллисекундах
        chrono::duration<double, std::milli>(end_seq - start_seq).count(); // разница времени -> мс -> число

    // 4) Параллельное среднее + измерение времени (мс)
    auto start_par = chrono::high_resolution_clock::now();  // старт таймера (параллельно)
    double avg_par = averageParallel(arr, n);               // считаем среднее с OpenMP (или fallback)
    auto end_par = chrono::high_resolution_clock::now();    // конец таймера
    double time_par_ms =                                   // время параллельного варианта
        chrono::duration<double, std::milli>(end_par - start_par).count(); // разница -> мс

    // 5) Вывод результатов
    cout << "Results:\n";                                   // заголовок вывода
    cout << "Sequential average: " << avg_seq               // среднее (последовательное)
         << ", time = " << time_seq_ms << " ms\n";          // время в миллисекундах
    cout << "Parallel average:   " << avg_par               // среднее (параллельное)
         << ", time = " << time_par_ms << " ms\n";          // время в миллисекундах

#ifdef _OPENMP
    cout << "OpenMP is enabled, max threads: "              // сообщение что OpenMP включен
         << omp_get_max_threads() << "\n";                  // сколько потоков максимум может использоваться
#else
    cout << "OpenMP not available (compiled without -fopenmp)\n"; // если не компилировали с OpenMP
#endif

    // 6) Освобождение памяти
    delete[] arr;                                           // освобождаем массив, который делали через new[]

    return 0;                                               // успешное завершение программы
}
