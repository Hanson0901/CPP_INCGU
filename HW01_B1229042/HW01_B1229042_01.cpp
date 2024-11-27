#include <iostream>
using namespace std;
int main(){
  int x,y;
  (cin>>x>>y).get();  
  char *arr = new char[x*y];
  for(int i=0;i<x;i++){
      int count=0;
    for(int j=0;j<y+1;j++){
      *(arr+i*y+j)=cin.get();
      count++;
      
      if(*(arr+i*y+j)=='\n'){
        j=y+1;  
        count=0;
      }
      if(j==y&&*(arr+i*y+j)!='\n'){
        cin.ignore(100,'\n');
        j=-1;
        cout<<"this is the wrong name length，please enter again:"<<endl;
        count=0;
        
      }
    }

  }

     for (int i = 0; i < x - 1; i++) {
        for (int j = 0; j < x - i - 1; j++) {
            if (*(arr + j * y) < *(arr + (j + 1) * y)) {
                char temp[y];
                for (int k = 0; k < y; k++) {
                    temp[k] = *(arr + j * y + k);
                }
                for (int k = 0; k < y; k++) {
                    *(arr + j * y + k) = *(arr + (j + 1) * y + k);
                }
                for (int k = 0; k < y; k++) {
                    *(arr + (j + 1) * y + k) = temp[k];
                }
            }
        }
    }

  cout<<"The result of string bubble sort is:"<<endl;
  for(int i=0;i<x;i++){
    for(int j=0;j<y;j++){
      if(*(arr+i*y+j)=='\n'){
        j=y;
        
      }
      else    cout<<*(arr+i*y+j);

    }
      cout<<endl;

  }
  delete[] arr;
}