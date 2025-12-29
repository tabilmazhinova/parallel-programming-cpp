#include <iostream>    // ввод и вывод (cout, cin)
#include <cstdlib>     // rand(), srand()
#include <ctime>       // time() для генерации seed
#include <chrono>      // высокоточный замер времени

#ifdef _OPENMP
#include <omp.h>       // библиотека OpenMP (потоки, reduction и т.д.)
#endif

using namespace std;   // чтобы не писать std::

// ==================== ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ====================

// Заполняем массив случайными числами
void fillArray(int* a, int n) {          // a — указатель на массив, n — размер
    for (int i = 0; i < n; i++) {        // проходим по всем элементам
        a[i] = rand() % 100000 + 1;      // случайное число от 1 до 100000
    }
}

// Копируем массив (чтобы seq и par сортировали одинаковые данные)
void copyArray(const int* src, int* dst, int n) {
    for (int i = 0; i < n; i++)           // идём по всем индексам
        dst[i] = src[i];                  // копируем элемент
}

// Проверяем, что массив отсортирован по возрастанию
bool isSorted(const int* a, int n) {
    for (int i = 1; i < n; i++) {         // начинаем со второго элемента
        if (a[i - 1] > a[i])              // если предыдущий больше текущего
            return false;                 // массив не отсортирован
    }
    return true;                          // если нарушений нет — OK
}

// Структура для хранения минимума и его индекса
struct MinPair {
    int val;                              // значение элемента
    int idx;                              // индекс элемента
};

// reduction для MinPair: выбираем пару с минимальным val
#pragma omp declare reduction( \
    minpair : MinPair : \
    omp_out = (omp_in.val < omp_out.val ? omp_in : omp_out) \
) initializer(omp_priv = omp_orig)

// Функция для удобного подсчёта времени в миллисекундах
long long diffMs(chrono::high_resolution_clock::time_point start,
                 chrono::high_resolution_clock::time_point end) {
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

// ==================== ПОСЛЕДОВАТЕЛЬНЫЕ СОРТИРОВКИ ====================

// Bubble Sort (пузырьковая сортировка)
void bubbleSeq(int* a, int n) {
    for (int i = 0; i < n - 1; i++) {     // количество проходов
        bool swapped = false;             // флаг обмена
        for (int j = 0; j < n - 1 - i; j++) { // сравнение соседних элементов
            if (a[j] > a[j + 1]) {        // если порядок неправильный
                int tmp = a[j];           // меняем местами
                a[j] = a[j + 1];
                a[j + 1] = tmp;
                swapped = true;           // был обмен
            }
        }
        if (!swapped) break;              // если обменов не было — массив отсортирован
    }
}

// Selection Sort (сортировка выбором)
void selectionSeq(int* a, int n) {
    for (int i = 0; i < n - 1; i++) {     // позиция, куда ставим минимум
        int minIdx = i;                   // предполагаемый минимум
        for (int j = i + 1; j < n; j++) { // ищем минимум справа
            if (a[j] < a[minIdx])
                minIdx = j;
        }
        if (minIdx != i) {                // если нашли другой минимум
            int tmp = a[i];               // меняем местами
            a[i] = a[minIdx];
            a[minIdx] = tmp;
        }
    }
}

// Insertion Sort (сортировка вставками)
void insertionSeq(int* a, int n) {
    for (int i = 1; i < n; i++) {         // начинаем со второго элемента
        int key = a[i];                   // элемент, который вставляем
        int j = i - 1;
        while (j >= 0 && a[j] > key) {    // сдвигаем элементы вправо
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;                   // вставляем элемент на место
    }
}

// ==================== ПАРАЛЛЕЛЬНЫЕ СОРТИРОВКИ ====================

// Параллельный пузырёк (odd-even phases)
void bubblePar(int* a, int n) {
#ifdef _OPENMP
    for (int phase = 0; phase < n; phase++) { // количество фаз
        int start = phase % 2;                // 0 — чётные пары, 1 — нечётные
        #pragma omp parallel for
        for (int j = start; j < n - 1; j += 2) { // независимые пары
            if (a[j] > a[j + 1]) {              // сравнение соседей
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }
#else
    bubbleSeq(a, n);                            // если OpenMP нет — обычный пузырёк
#endif
}

// Параллельная сортировка выбором
void selectionPar(int* a, int n) {
#ifdef _OPENMP
    for (int i = 0; i < n - 1; i++) {           // позиция минимума
        MinPair best{a[i], i};                  // текущий минимум

        #pragma omp parallel for reduction(minpair: best)
        for (int j = i + 1; j < n; j++) {       // поиск минимума параллельно
            if (a[j] < best.val) {
                best = MinPair{a[j], j};
            }
        }

        if (best.idx != i) {                    // меняем найденный минимум
            int tmp = a[i];
            a[i] = a[best.idx];
            a[best.idx] = tmp;
        }
    }
#else
    selectionSeq(a, n);                         // fallback без OpenMP
#endif
}

// Insertion Sort не распараллеливается,
// потому что каждый шаг зависит от предыдущего
// (левая часть массива должна быть уже отсортирована).

// ==================== ТЕСТИРОВАНИЕ И СРАВНЕНИЕ ====================

void testOneSize(int n) {
    cout << "\n--- Размер массива: " << n << " ---\n";

    int* original = new int[n];                 // исходный массив
    fillArray(original, n);                     // заполняем случайными числами

    // ---------- Bubble ----------
    {
        int* seqArr = new int[n];
        int* parArr = new int[n];
        copyArray(original, seqArr, n);
        copyArray(original, parArr, n);

        auto s1 = chrono::high_resolution_clock::now();
        bubbleSeq(seqArr, n);
        auto e1 = chrono::high_resolution_clock::now();

        auto s2 = chrono::high_resolution_clock::now();
        bubblePar(parArr, n);
        auto e2 = chrono::high_resolution_clock::now();

        cout << "Bubble    seq: " << diffMs(s1, e1)
             << " ms | par: " << diffMs(s2, e2)
             << (isSorted(parArr, n) ? " | OK\n" : " | ERROR\n");

        delete[] seqArr;
        delete[] parArr;
    }

    // ---------- Selection ----------
    {
        int* seqArr = new int[n];
        int* parArr = new int[n];
        copyArray(original, seqArr, n);
        copyArray(original, parArr, n);

        auto s1 = chrono::high_resolution_clock::now();
        selectionSeq(seqArr, n);
        auto e1 = chrono::high_resolution_clock::now();

        auto s2 = chrono::high_resolution_clock::now();
        selectionPar(parArr, n);
        auto e2 = chrono::high_resolution_clock::now();

        cout << "Selection seq: " << diffMs(s1, e1)
             << " ms | par: " << diffMs(s2, e2)
             << (isSorted(parArr, n) ? " | OK\n" : " | ERROR\n");

        delete[] seqArr;
        delete[] parArr;
    }

    // ---------- Insertion ----------
    {
        int* seqArr = new int[n];
        copyArray(original, seqArr, n);

        auto s1 = chrono::high_resolution_clock::now();
        insertionSeq(seqArr, n);
        auto e1 = chrono::high_resolution_clock::now();

        cout << "Insertion seq: " << diffMs(s1, e1)
             << " ms | par: N/A (not parallelized) "
             << (isSorted(seqArr, n) ? "| OK\n" : "| ERROR\n");

        delete[] seqArr;
    }

    delete[] original;                          // освобождаем память
}

int main() {
    ios::sync_with_stdio(false);               // ускоряем ввод/вывод
    cin.tie(nullptr);                          // отключаем привязку cin к cout

    srand((unsigned)time(nullptr));            // seed для rand()

#ifdef _OPENMP
    cout << "OpenMP включён, потоков: "
         << omp_get_max_threads() << "\n";
#else
    cout << "OpenMP отключён (компиляция без -fopenmp)\n";
#endif

    int sizes[] = {1000, 10000, 100000};       // размеры массивов для тестов
    for (int n : sizes) {
        testOneSize(n);                        // запускаем тесты
    }

    return 0;                                  // успешное завершение
}
