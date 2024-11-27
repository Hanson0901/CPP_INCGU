#include<iostream> 
#include <string.h>
using namespace std;
int main(){
    int x;
    cin>> x;
    cin.ignore(100,'\n');
    string arr[x];//string 為char的陣列
    //string *arr = new string[x];
    string **arr_output = new string*[x];
     for (int i = 0; i < x; i++) {
        arr_output[i] = new string;
    }
    for(int i=0;i<x;i++){
        getline(cin,arr[i]);//讀完一整行
    }
    int max_space=0;
    for(int i=0;i<x;i++){
        int compare_space;
        if(arr[i].find(" ")!=string::npos){
            compare_space =arr[i].find_first_of(" ");

            if(max_space<compare_space){
                max_space=compare_space;
            }
        }
        else {
            compare_space=arr[i].length();
            if(max_space<compare_space){
                max_space=compare_space;
            }
        }
        }
        
    
    for(int i=0;i<x;i++){
        if(arr[i].find(" ")!=string::npos){
            string spacecount;
            for(int j=0;j<(max_space-arr[i].find_first_of(" "));j++){
                cout<<" ";
                spacecount=" "+spacecount;
            }
            *(arr_output[i])=spacecount+*(arr_output[i]);
            cout<<arr[i]<<endl;
            *(arr_output[i])=*(arr_output[i])+arr[i];
        }

        else{
            string spacecount;
            for(int j=0;j<max_space-arr[i].length();j++){
                cout<<" ";
                spacecount=" "+spacecount;
            }
            *(arr_output[i])=spacecount+*(arr_output[i]);
            cout<<arr[i]<<endl;
            *(arr_output[i])=*(arr_output[i])+arr[i];

        }
        
    }
    cout<<"the final list is:"<<endl;
    for(int i=0;i<x;i++){
        cout<<*(arr_output[i])<<endl;
    }
    for(int i=0;i<x;i++){
        delete arr_output[i];
    }
delete []arr_output;

}