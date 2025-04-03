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
void first_string();
void SetColorAndBackground(int ForgC, int BackC);
int init_arr(vector<int>&initial_arr,int &quantity);
void sort_switch(int &mode,vector<int> &arr,int &size,bool &ascend);
void second_string();
void print_sorted_arr(const vector<int>&arr);
void ask_ascend();
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
    /*first_string();
    size= init_arr(initial_arr,quantity);

    second_string();
    sort_switch(sort_mode,initial_arr,size,ascend);
    print_sorted_arr(initial_arr);*/
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

void first_string(){

    cout<<"choice ur quantity for the size of array:"<<endl;
    SetColorAndBackground(15,8);
    cout<<"(1):10 , (2):100 , (3):1000 , (4):10000 , (5):100000 "<<endl;
    SetColorAndBackground(15,0);
};

void second_string(){
    cout<<"choose ur sort mode:"<<endl;
    SetColorAndBackground(15,8);
    cout<<"(1):quick sort , (2):Merge_sort , (3):Heap_sort "<<endl;
    SetColorAndBackground(15,0);
};



int init_arr(vector<int>&initial_arr,int &quantity){
    while(cout<<"quantity:" && cin>>quantity){

        if(quantity<1||quantity>5){
            SetColorAndBackground(7,4);
            cout<<"Waring:False input,type quantity again!!\n"<<endl;
            SetColorAndBackground(15,0);
            first_string();
        }
        else{ 
            generate_random_array(initial_arr,static_cast<int>(pow(10,quantity))) ;
            break;
        }
    }
    return static_cast<int>(pow(10,quantity));
};



void sort_switch(int &mode,vector<int> &arr,int &size,bool &ascend){
    while(cout<<"mode:" && cin>>mode){
        
        if(mode==1){
            ask_ascend();
            Quick_Sort(arr.data(),0,size-1,ascend);
            break;
        }
        else if(mode ==2){
            ask_ascend();
            vector<int>temp_arr(arr.size());
            Merge_Sort(arr.data(),temp_arr.data(),0,arr.size()-1,ascend);
            break;
        }
        else if(mode == 3){
            ask_ascend();
            Heap_Sort(arr,ascend);
            break;
        }
        else{
            SetColorAndBackground(7,4);
            cout<<"Waring:False input,choose mode again!!\n"<<endl;
            SetColorAndBackground(15,0);
            first_string();
        }
        
        
    }
};


void print_sorted_arr(const vector<int>&arr){
    for(auto a:arr){
        cout<<a<<" ";
    }
    cout<<endl;
};

void ask_ascend(){
    cout<<"ascend(1) or descend(0)(defualt is ascend if no input):";
    cin>>ascend;
};
void SetColorAndBackground(int ForgC, int BackC) {
    #ifdef _WIN32
        // Windows: 使用 SetConsoleTextAttribute
        WORD wColor = ((BackC & 0x0F) << 4) + (ForgC & 0x0F);
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColor);
    #else
        // Linux/macOS: 使用 ANSI Escape Codes
        int textColor = ForgC % 8;  // 0~7 對應標準顏色
        int backgroundColor = BackC % 8 + 40; // 背景色碼範圍 40~47
    
        cout << "\033[" << backgroundColor << ";3" << textColor << "m";
    #endif
    }