#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <fstream>
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

// 匯出邊列表CSV
void export_edge_list_csv(const vector<vector<int>>& adj_matrix, const string& filename) {
    ofstream outfile(filename);
    outfile << "Source,Target,Weight\n";
    for (int i = 0; i < adj_matrix.size(); ++i)
        for (int j = i+1; j < adj_matrix.size(); ++j)
            if (adj_matrix[i][j])
                outfile << i << "," << j << ",1\n";
    outfile.close();
}

// DFS 生成樹
void dfs_tree(int u, const vector<vector<int>>& adj_list, vector<bool>& visited, vector<pair<int,int>>& tree_edges, int parent = -1) {
    visited[u] = true;
    if (parent != -1) tree_edges.push_back({parent, u});
    for (int v : adj_list[u]) {
        if (!visited[v])
            dfs_tree(v, adj_list, visited, tree_edges, u);
    }
}

// BFS 生成樹
void bfs_tree(int start, const vector<vector<int>>& adj_list, vector<bool>& visited, vector<pair<int,int>>& tree_edges) {
    queue<int> q;
    visited[start] = true;
    q.push(start);
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj_list[u]) {
            if (!visited[v]) {
                visited[v] = true;
                tree_edges.push_back({u, v});
                q.push(v);
            }
        }
    }
}

// 匯出生成樹邊列表CSV
void export_tree_csv(const vector<pair<int,int>>& tree_edges, const string& filename) {
    ofstream outfile(filename);
    outfile << "Source,Target,Weight\n";
    for (auto& edge : tree_edges)
        outfile << edge.first << "," << edge.second << ",1\n";
    outfile.close();
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
    vector<vector<int>> adj_matrix, adj_list;
    generate_random_graph(n, e, adj_matrix, adj_list);
    print_adj_matrix(adj_matrix);
    print_adj_list(adj_list);
    export_edge_list_csv(adj_matrix, "graph_edges.csv");
    cout << "匯出graph_edges.csv\n";

    // DFS tree
    vector<bool> visited(n, false);
    vector<pair<int,int>> dfs_edges;
    for (int i = 0; i < n; ++i) // forest
        if (!visited[i])
            dfs_tree(i, adj_list, visited, dfs_edges);
    export_tree_csv(dfs_edges, "dfs_tree.csv");
    cout << "匯出dfs_tree.csv\n";

    // BFS tree
    fill(visited.begin(), visited.end(), false);
    vector<pair<int,int>> bfs_edges;
    for (int i = 0; i < n; ++i)
        if (!visited[i])
            bfs_tree(i, adj_list, visited, bfs_edges);
    export_tree_csv(bfs_edges, "bfs_tree.csv");
    cout << "匯出bfs_tree.csv\n";

    return 0;
}
