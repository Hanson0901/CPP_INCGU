#include <iostream>
#include <string>
using namespace std;
#pragma GCC optimize("Ofast,unroll-loops,no-stack-protector,fast-math")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,tune=native")
#pragma comment(linker, "/stack:200000000")
int main() {
    int x;
    cin >> x;
    cin.ignore(100, '\n');

    string* arr = new string[x]; // 动态分配内存给字符串数组
    string** arr_output = new string*[x]; // 动态分配内存给二维字符串数组

    // 初始化二维数组的每一行
    for (int i = 0; i < x; i++) {
        arr_output[i] = new string;
    }

    // 读取所有输入行
    for (int i = 0; i < x; i++) {
        getline(cin, arr[i]);
    }

    int max_space = 0;

    // 计算最大空格位置或最大字符串长度
    for (int i = 0; i < x; i++) {
        int compare_space;
        size_t space_pos = arr[i].find(" ");

        if (space_pos != string::npos) {
            compare_space = space_pos;
        } else {
            compare_space = arr[i].length();
        }

        if (max_space < compare_space) {
            max_space = compare_space;
        }
    }

    // 根据最大长度进行对齐并存储结果
    for (int i = 0; i < x; i++) {
        size_t space_pos = arr[i].find(" ");
        string padding(max_space - (space_pos != string::npos ? space_pos : arr[i].length()), ' ');

        if (space_pos != string::npos) {
            *arr_output[i] = padding + arr[i];
        } else {
            *arr_output[i] = padding + arr[i];
        }
    }

    // 输出结果
    cout << "The final list is:" << endl;
    for (int i = 0; i < x; i++) {
        cout << *(arr_output[i]) << endl;
    }

    // 释放动态分配的内存
    for (int i = 0; i < x; i++) {
        delete arr_output[i];
    }
    delete[] arr_output;
    delete[] arr;

    return 0;
}
