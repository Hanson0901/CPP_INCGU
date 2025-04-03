#ifndef HEAP_SORT_H
#define HEAP_SORT_H
#include<iostream>
#include<vector>
template <class T>
void Adjust(std::vector<T> &a, int root, int n, bool ascend) {
    T e = a[root]; // 暫存根節點
    int j;
    
    for (j = 2 * root + 1; j < n; j = 2 * j + 1) { // 找左子節點
        if(ascend){
            if (j + 1 < n && a[j] < a[j + 1]) // 若右子節點較大，則 j 指向右子節點
                j++;
            if (e >= a[j]) // 若根節點已大於子節點，則結束調整
                break;
            }
        else{
            if (j + 1 < n && a[j] > a[j + 1]) // 若右子節點較大，則 j 指向右子節點
                j++;
            if (e <= a[j]) // 若根節點已大於子節點，則結束調整
                break;
        }
        a[root] = a[j]; // 把較大子節點上移
        root = j;
    }
    a[root] = e; // 放回 e
}

// 堆積排序
template <class T>
void Heap_Sort(std::vector<T> &a,bool ascend) {
    int n = a.size();

    // 建立堆 (Heap)
    for (int i = n / 2 - 1; i >= 0; i--)
        Adjust(a, i, n,ascend);

    // 依序取出最大、小值並調整堆積
    for (int i = n - 1; i > 0; i--) {
        std::swap(a[0], a[i]); // 將堆頂  與最後一個元素交換
        Adjust(a, 0, i,ascend); // 重新調整堆
    }
}

#endif