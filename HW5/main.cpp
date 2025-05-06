#include <iostream>
#include <vector>
#include <set>
#include <ctime>
#include <cstdlib>
using namespace std;

// 隨機產生無向圖
void generate_random_graph(int n, int e, vector<vector<int>>& adj_matrix, vector<vector<int>>& adj_list) {
    // 初始化鄰接矩陣
    adj_matrix.assign(n, vector<int>(n, 0));
    // 初始化鄰接串列
    adj_list.assign(n, vector<int>());

    set<pair<int, int>> edge_set;
    srand(time(0));
    while (edge_set.size() < e) {
        int u = rand() % n;
        int v = rand() % n;
        if (u == v) continue; // 不允許自環
        // 保證無向圖且不重複
        int a = min(u, v), b = max(u, v);
        if (edge_set.count({a, b}) == 0) {
            edge_set.insert({a, b});
            adj_matrix[a][b] = 1;
            adj_matrix[b][a] = 1;
            adj_list[a].push_back(b);
            adj_list[b].push_back(a);
        }
    }
}

// 輸出鄰接矩陣
void print_adj_matrix(const vector<vector<int>>& adj_matrix) {
    cout << "鄰接矩陣：" << endl;
    for (const auto& row : adj_matrix) {
        for (int val : row) {
            cout << val << " ";
        }
        cout << endl;
    }
}

// 輸出鄰接串列
void print_adj_list(const vector<vector<int>>& adj_list) {
    cout << "鄰接串列：" << endl;
    for (int i = 0; i < adj_list.size(); ++i) {
        cout << i << ": ";
        for (int v : adj_list[i]) {
            cout << v << " ";
        }
        cout << endl;
    }
}

int main() {
    int n, e;
    cout << "請輸入節點數 n：";
    cin >> n;
    cout << "請輸入邊數 e：";
    cin >> e;
    // 最大邊數檢查
    if (e > n * (n - 1) / 2) {
        cout << "邊數超過無向圖最大可能數量！" << endl;
        return 1;
    }
    vector<vector<int>> adj_matrix;
    vector<vector<int>> adj_list;
    generate_random_graph(n, e, adj_matrix, adj_list);
    print_adj_matrix(adj_matrix);
    print_adj_list(adj_list);
    return 0;
}
