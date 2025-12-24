#include <iostream>
#include <cstdlib>     // rand, srand
#include <ctime>       // time
#include <chrono>      // замер времени

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;

// Заполняем массив случайными числами
void fillArray(int* a, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = rand() % 100000 + 1;
    }
}

// Копируем массив, чтобы seq и par сортировали одинаковые данные
void copyArray(const int* src, int* dst, int n) {
    for (int i = 0; i < n; i++) dst[i] = src[i];
}

// Проверяем, что массив отсортирован (для контроля)
bool isSorted(const int* a, int n) {
    for (int i = 1; i < n; i++) {
        if (a[i - 1] > a[i]) return false;
    }
    return true;
}
struct MinPair {
    int val;
    int idx;
};

// reduction для MinPair: выбираем пару с меньшим val
#pragma omp declare reduction( \
    minpair : MinPair : \
    omp_out = (omp_in.val < omp_out.val ? omp_in : omp_out) \
) initializer(omp_priv = omp_orig)


// Удобно считать разницу времени в миллисекундах
long long diffMs(chrono::high_resolution_clock::time_point start,
                 chrono::high_resolution_clock::time_point end) {
    return chrono::duration_cast<chrono::milliseconds>(end - start).count();
}

// SEQ sorting

// Bubble Sort (пузырьком)
void bubbleSeq(int* a, int n) {
    for (int i = 0; i < n - 1; i++) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
                swapped = true;
            }
        }
        // если нет — массив уже отсортирован
        if (!swapped) break;
    }
}

// Selection Sort (выбором)
void selectionSeq(int* a, int n) {
    for (int i = 0; i < n - 1; i++) {
        int minIdx = i;
        for (int j = i + 1; j < n; j++) {
            if (a[j] < a[minIdx]) minIdx = j;
        }
        if (minIdx != i) {
            int tmp = a[i];
            a[i] = a[minIdx];
            a[minIdx] = tmp;
        }
    }
}

// Insertion Sort (вставками) 
void insertionSeq(int* a, int n) {
    for (int i = 1; i < n; i++) {
        int key = a[i];
        int j = i - 1;
        while (j >= 0 && a[j] > key) {
            a[j + 1] = a[j];
            j--;
        }
        a[j + 1] = key;
    }
}

// PAR sorting

// Параллельный пузырёк делаю через odd-even phases.
void bubblePar(int* a, int n) {
#ifdef _OPENMP
    for (int phase = 0; phase < n; phase++) {
        int start = phase % 2; // 0: (0,1)(2,3)... 1: (1,2)(3,4)...
        #pragma omp parallel for
        for (int j = start; j < n - 1; j += 2) {
            if (a[j] > a[j + 1]) {
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }
#else
    bubbleSeq(a, n);
#endif
}

// В selection сортировке внешний цикл i зависит от результата,но поиск минимума на отрезке [i+1..n) можно распараллелить.
void selectionPar(int* a, int n) {
#ifdef _OPENMP
    for (int i = 0; i < n - 1; i++) {
        MinPair best{a[i], i};

        #pragma omp parallel for reduction(minpair: best)
        for (int j = i + 1; j < n; j++) {
            if (a[j] < best.val) {
                best = MinPair{a[j], j};
            }
        }

        if (best.idx != i) {
            int tmp = a[i];
            a[i] = a[best.idx];
            a[best.idx] = tmp;
        }
    }
#else
    selectionSeq(a, n);
#endif
}


// Insertion Sort no parallel sorting.
// Причина: каждый шаг зависит от предыдущего (отсортированная часть слева),и если параллелить, то будут конфликты и результат будет неправильный.

//Cравнение времени 

void testOneSize(int n) {
    cout << "\n--- Размер массива: " << n << " ---\n";

    // создаём исходный массив
    int* original = new int[n];
    fillArray(original, n);

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

        cout << "Bubble    seq: " << diffMs(s1, e1) << " ms | par: " << diffMs(s2, e2) << " ms";
        cout << (isSorted(parArr, n) ? " | OK\n" : " | ERROR\n");

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

        cout << "Selection seq: " << diffMs(s1, e1) << " ms | par: " << diffMs(s2, e2) << " ms";
        cout << (isSorted(parArr, n) ? " | OK\n" : " | ERROR\n");

        delete[] seqArr;
        delete[] parArr;
    }

    // ---------- Insertion (только seq, no par) ----------
    {
        int* seqArr = new int[n];
        copyArray(original, seqArr, n);

        auto s1 = chrono::high_resolution_clock::now();
        insertionSeq(seqArr, n);
        auto e1 = chrono::high_resolution_clock::now();

        cout << "Insertion seq: " << diffMs(s1, e1) << " ms | par: N/A (not parallelized) ";
        cout << (isSorted(seqArr, n) ? "| OK\n" : "| ERROR\n");

        delete[] seqArr;
    }

    delete[] original;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    srand((unsigned)time(nullptr));

#ifdef _OPENMP
    cout << "OpenMP включён, потоков: " << omp_get_max_threads() << "\n";
#else
    cout << "OpenMP отключён (компиляция без -fopenmp)\n";
#endif

    int sizes[] = {1000, 10000, 100000};
    for (int n : sizes) {
        testOneSize(n);
    }

    return 0;
}
