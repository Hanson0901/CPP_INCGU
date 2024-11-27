#include<algorithm> 使用sort函數
class numberChecker{
    public:
        int x,y;
};
class Solution {
public:
    vector<int> kWeakestRows(vector<vector<int>>& mat, int k) {
        vector<numberChecker.x,numberChecker.y>compare;
        vector<int>answer;   
        for(int i=0;i<mat.size();i++){
            int count=0;
            for(int j=0;j<mat[0].size();i++){
                if(mat[i][j]==1){
                    count+=1;
                }
            }
            compare.push_back({count,i});
        }
        sort(compare.begin(),compare.end());
        for(int i=0;){
            
        };
    }
};