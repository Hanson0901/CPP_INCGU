#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <fstream>
#include <chrono>  //更精準的時間，clock(ms),chrono(ns)
#include <iomanip> //格式化輸出工具
using namespace std;
using namespace chrono;

// HeapSort
void Adjust(int *a, int root, int n)
{
    int e = a[root];
    for (int j = 2 * root + 1; j < n; j = 2 * j + 1)
    {
        if (j + 1 < n && a[j] < a[j + 1])
        {
            j++;
        }
        if (e >= a[j])
        {
            break;
        }
        a[root] = a[j];
        root = j;
    }
    a[root] = e;
}

void HeapSort(int *a, int n)
{
    for (int i = (n / 2) - 1; i >= 0; i--) // 堆積
    {
        Adjust(a, i, n);
    }
    for (int i = n - 1; i > 0; i--) // 排序
    {
        swap(a[0], a[i]);
        Adjust(a, 0, i);
    }
}

// InsertionSort
void Insert(int e, int *a, int i)
{
    while (i >= 0 && e < a[i])
    {
        a[i + 1] = a[i];
        i--;
    }
    a[i + 1] = e;
}

void InsertionSort(int *a, int n)
{
    for (int j = 1; j < n; j++)
    {
        int temp = a[j];
        Insert(temp, a, j - 1);
    }
}

// QuickSort
void QuickSort(int *a, int left, int right)
{
    if (left < right)
    {
        int i = left;
        int j = right + 1;
        int pivot = a[left];
        do
        {
            do
            {
                i++;
            } while (i <= right && a[i] < pivot);
            do
            {
                j--;
            } while (a[j] > pivot);
            if (i < j)
            {
                swap(a[i], a[j]);
            }
        } while (i < j);
        swap(a[left], a[j]);
        QuickSort(a, left, j - 1);
        QuickSort(a, j + 1, right);
    }
}

void RandomNum(int *a, int n)
{
    for (int i = 0; i < n; i++)
    {
        a[i] = rand() % 10000 + 1;
    }
}

void PrintFirst10(int *a, int n, string data)
{
    cout << data;
    for (int i = 0; i < 10 && i < n; i++)
    {
        cout << a[i] << " ";
    }
    if (n > 10)
    {
        cout << "...";
    }
    cout << endl;
}

double RunSortAverage(string method, int *data, int n, int repeat = 100)
{
    double total = 0;
    for (int r = 0; r < repeat; r++)
    {
        int *a = new int[n]; // 這個a陣列是要來排序並保留原本的
        for (int i = 0; i < n; i++)
        {
            a[i] = data[i]; // 將data內容一個個複製到a裡面
        }

        auto start = high_resolution_clock::now(); // 排序開始的time(ns)

        if (method == "heapsort")
        {
            HeapSort(a, n);
        }
        else if (method == "insertionsort")
        {
            InsertionSort(a, n);
        }
        else if (method == "quicksort")
        {
            QuickSort(a, 0, n - 1);
        } // 對a排序，not data

        auto end = high_resolution_clock::now();
        total = total + duration<double>(end - start).count(); // 精確換算成秒

        delete[] a;
    }

    return total / repeat;
}

void UserMode()
{
    int sizes[] = {10,
                   20,
                   40,
                   80,
                   160,
                   320,
                   640,
                   1280,
                   2560,
                   5120,
                   10240,
                   20480,
                   40960,
                   81920,
                   163840};
    string methods[] = {"heapsort", "insertionsort", "quicksort"};

    for (int i = 0; i < sizeof(sizes); i++)
    {
        int n = sizes[i];
        int *data = new int[n];
        RandomNum(data, n);

        cout << "\n== data amounts: " << n << " ==\n";

        for (string method : methods)
        {
            cout << "[" << method << "] Sorting...\n";
            double t = RunSortAverage(method, data, n);
            cout << "Soring Time: " << fixed << setprecision(9) << t << " sec\n"; // 精確到小數後第9位
        }

        delete[] data;
    }
}

int main()
{
    srand(time(0));

    // UserMode();

    int sizes[] = {
        10,
        20,
        40,
        80,
        160,
        320,
        640,
        1280,
        2560,
        5120,
        10240,
        20480,
        40960,
        81920,
        163840};
    string methods[] = {"heapsort", "insertionsort", "quicksort"};

    ofstream fout("Sorting_Results.csv");
    fout << "Method,Size,Time\n";

    for (string method : methods)
    {
        for (int i = 0; i < sizeof(sizes); i++)
        {
            int n = sizes[i];
            int *data = new int[n];
            RandomNum(data, n);

            double t = RunSortAverage(method, data, n); // 再跑一次，因為寫入 CSV
            fout << method << "," << n << "," << t << endl;

            delete[] data;
        }
    }

    fout.close();
    cout << "Sorting Finish";
}