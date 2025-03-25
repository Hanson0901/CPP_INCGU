#ifndef QUICK_SORT_H
#define QUICK_SORT_H

#include<iostream>

template <class T>
void Quick_Sort(T *a, int left, int right) {
    if (left < right) {
        int i = left, j = right; 
        T pivot = a[left]; 

        while (i <= j) { 
            while (a[i] < pivot) i++;
            while (a[j] > pivot) j--;
            if (i <= j) {
                std::swap(a[i], a[j]);
                i++;
                j--;
            }
        }

        if (left < j) Quick_Sort(a, left, j);
        if (i < right) Quick_Sort(a, i, right);
    }
}
#endif 