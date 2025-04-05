#include<iostream>
#include<cmath>
#include<ctime>
#include<vector>
#include <fstream>
#include<chrono>
#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

#include"quick_sort.h"
#include"merge_sort.h"
#include"heap_sort.h"

using namespace std;
void generate_random_array(vector<int> &arr, int size);
void counting_time(int &size,vector<int>&initial_arr,int &quantity,int &sort_mode);
bool ascend=true,timer;//全域變數for降冪or升冪 and 計時
clock_t start, tend;//全域計算時間
double duration;
//main function
int main(){
    srand(time(NULL));

    int quantity=0;
    vector<int>initial_arr;
    int sort_mode=0;
    int size=0;
    counting_time(size,initial_arr,quantity,sort_mode);
    return 0;
}



void generate_random_array(vector<int> &arr, int size) {
    arr.clear();
    for (int i = 0; i < size; i++) {
        arr.push_back((rand() % 50000)+1); 
    }
};
void counting_time(int &size,vector<int>&initial_arr,int &quantity,int &sort_mode){
    ofstream ofs("timer.txt");
    while(cout<<"<counting time?"&&cin>>timer>>quantity>>sort_mode){
        if(!timer){
            break;
        }
        else{
            size= static_cast<int>(pow(10,quantity));
            
            if (sort_mode == 1) {
                for (int j=0; j<100; j++) {
                    auto start = chrono::high_resolution_clock::now(); // 修改點1
                    
                    for (int i=0; i<1000; i++) {
                        generate_random_array(initial_arr, size);
                        Quick_Sort(initial_arr.data(), 0, size-1, ascend);
                    }
                    
                    auto end = chrono::high_resolution_clock::now(); // 修改點2
                    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count(); // 修改點3
                    ofs << duration << endl;
                }
            }
            else if(sort_mode==2){
                for(int j=0;j<100;j++){
                auto start = chrono::high_resolution_clock::now(); // 修改點1
                for(int i=0;i<1000;i++){
                    generate_random_array(initial_arr,static_cast<int>(pow(10,quantity))) ;
                    vector<int>temp_arr(size);
                    
                    Merge_Sort(initial_arr.data(),temp_arr.data(),0,size-1,ascend);
                    
                }
                auto end = chrono::high_resolution_clock::now(); // 修改點2
                auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count(); // 修改點3
                ofs<<duration<<endl;
                }
            }
            else if(sort_mode==3){
                for(int j=0;j<100;j++){
                auto start = chrono::high_resolution_clock::now(); // 修改點1
                for(int i=0;i<1000;i++){
                    generate_random_array(initial_arr,static_cast<int>(pow(10,quantity))) ;
                    
                    Heap_Sort(initial_arr,ascend);
                }
                auto end = chrono::high_resolution_clock::now(); // 修改點2
                auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count(); // 修改點3
                    ofs<<duration<<endl;
                }
            }
            break;
        }
    }

};

