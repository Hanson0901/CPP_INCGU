#ifndef MERGE_SORT_H
#define MERGE_SORT_H
#include<iostream>
    template <class T>
    void Merge(T* initList, T*merged_list,const int l,const int m, const int n,bool ascend){
        int a=l,b=m+1,result =l;

        while (a<=m && b<=n){// 合併兩個已排序的arr
            if(ascend){
                if(initList[a]<=initList[b]){

                    merged_list[result++] = initList[a++];
                }

                else{

                    merged_list[result++]=initList[b++];
                }
            }
            else{
                if(initList[a]>=initList[b]){

                    merged_list[result++] = initList[a++];
                }

                else{

                    merged_list[result++]=initList[b++];
                }
            }
        }
        
        while (a <= m){
            merged_list[result++] = initList[a++];
        }

        while(b<=n){
            merged_list[result++]=initList[b++];
        }
        
        for(int i=l;i<n;i++){
            initList[i]=merged_list[i];
        }
    }
    template <class T>
    void Merge_Sort(T *arr, T *tempArr, int left, int right,bool ascend) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            
            // 遞迴排序左右兩部分
            Merge_Sort(arr, tempArr, left, mid,ascend);
            Merge_Sort(arr, tempArr, mid + 1, right,ascend);
            
            // 合併排序後的兩部分
            Merge(arr, tempArr, left, mid, right,ascend);
        }
    }
    
#endif