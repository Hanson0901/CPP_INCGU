#include<iostream>
#include<vector>
#include<iomanip>
#include<algorithm>
#include<fstream>
#include<ctime> 
using namespace std;

void selection_sort(vector<int>& nums) {
    int n = nums.size();
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (nums[j] < nums[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            swap(nums[i], nums[min_idx]);
        }
    }
}

int main() {
    
    ofstream out;
    out.open("times.txt");
    // 設定隨機數種子
    srand(time(NULL));

    // 重複 100 次進行測試
    for (int i = 0; i < 3000; i++) {
        
        vector<int> nums;
        // 生成 N 個不重複的隨機數字
        double current_clock = (double)clock();
        for(int j=0;j<100;j++){
            while (nums.size() < i) {
                int min = 0;
                int max = i;
                int x = rand() % (max - min + 1) + min;
                if (find(nums.begin(), nums.end(), x) == nums.end()) {
                    nums.push_back(x);
                }
            }
            

            // 使用 selection sort 進行排序
            selection_sort(nums);
        }
    //cout << fixed << setprecision(6) << ((double)clock() - current_clock) / (CLOCKS_PER_SEC ) << endl;
    out<<((double)clock() - current_clock) / (CLOCKS_PER_SEC )<<endl;
    }

    // 計算平均時間
    out.close();
    return 0;
}