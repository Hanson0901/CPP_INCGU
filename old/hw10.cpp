#include<iostream>
#include<vector>
#include<time.h>
#include<iomanip>
#include<fstream>
using namespace std;
int binary_search(vector<int> &nums, int target){
    int left = 0;
    int right = nums.size() - 1;
    while (left <= right) {
        int mid = (left + right) / 2; 
        if (nums[mid] > target) {
            right = mid - 1;
        } else if (nums[mid] < target) {
            left = mid + 1;
        } else {
            return mid; 
        }
    }
    return -1;
}
int main() {
    ofstream out;
    out.open("timess.txt");
    vector<int> nums;
    srand(time(NULL));
    
    for(int i=0;i<=3000;i++){
        nums.push_back(i);
        int x;
        int min = 0;
        int max = i;
        x = rand() % (max - min + 1) + min;
        double current_clock= (double)clock();
        
        for(int j=0;j<=10000000;j++){
        binary_search(nums,x);
        }
        out<<((double)clock() - current_clock) / (CLOCKS_PER_SEC )<<endl;
        //cout <<fixed<<setprecision(6)<<((double)clock()-current_clock) / (CLOCKS_PER_SEC) <<endl;
    }

}