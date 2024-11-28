#include <iostream>
//用來算時間的
#include <chrono>
// 生亂數用
#include <ctime>
#include <cstdlib>
using namespace std;

//定義Node
struct Node {
    // 係數
    int coef;
    // 指數
    int expo;
    //指向下一個節點的指標
    Node* link;

    Node(int c, int e) : coef(c), expo(e), link(nullptr) {}
};

// 插入節點
// 插入節點
Node* insert(Node* head, int coef, int expo) {
    //如果係數是零，直接返回原本的head，這樣才不會輸出多餘的0
    if (coef == 0) return head;

    // temp用來找要插入在哪裡
    Node* temp;
    // 用new跟前面的Node(15行)加一個新節點
    Node* newp = new Node(coef, expo);

    // 如果最前面是空的，或是新指數大於舊的head指數
    if (head == nullptr || expo > head->expo) {
        newp->link = head;
        head = newp;
    } else if (expo == head->expo) { // 如果新指數與head指數相同
        head->coef += coef;
        delete newp;
    } else {
        temp = head;
        // 用temp來輔助運算找出插入位置
        while (temp->link != nullptr && temp->link->expo > expo) {
            temp = temp->link;
        }
        if (temp->link != nullptr && temp->link->expo == expo) {
            // 如果找到相同指數，合併係數
            temp->link->coef += coef;
            delete newp;
        } else {
            // 否則插入新節點
            newp->link = temp->link;
            temp->link = newp;
        }
    }
    return head;
}




// 建立多項式
Node* create(Node* head, string termType) {
    int termCount, coef, expo;

    cout << "輸入Terms(" << termType << "): ";
    cin >> termCount;
    
    int tempExpo = rand()%1000;

    for (int i = 0; i < termCount; ++i) {
        //係數直接給0
        coef=1;
        
        int expo = tempExpo+i;
        
        head = insert(head, coef, expo);
    }
    return head;
}

// 印出多項式
void print(Node* head) {
    if (head == nullptr) {
        cout << "0" << endl;
        return;
    }
    Node* temp = head;
    bool noZero = false;
    
    while (temp != nullptr) {
        if (temp->coef != 0){
            noZero = true;
        
            if (temp->expo == 1) {
                cout << temp->coef << "x";
            } else if (temp->expo == 0) {
                cout << temp->coef;
            } else {
                cout << "(" << temp->coef << "x^" << temp->expo << ")";
            }
            
            if (temp->link != nullptr && temp->link->coef != 0) {
                cout << " + ";
            }
        }
        temp = temp->link;
    }
    
    if(!noZero){
        cout << "0";
    }
    cout << endl;
}

// 多項式相乘
void polyMult(Node* head1, Node* head2) {
    Node* ptr1 = head1;
    Node* ptr2 = head2;
    Node* head3 = nullptr;

    if (head1 == nullptr || head2 == nullptr) {
        cout << "零多項式" << endl;
        return;
    }
    int first=1;
    while (ptr1 != nullptr) {
        while (ptr2 != nullptr) {
            head3 = insert(head3, ptr1->coef * ptr2->coef, ptr1->expo + ptr2->expo);
            ptr2 = ptr2->link;
        }
        ptr1 = ptr1->link;
        ptr2 = head2;
    }
      
    cout << "poly1 : ";
    print(head1);
    cout << "poly2 : ";
    print(head2);
    cout << "poly1 x poly2 : ";
    print(head3);
}

int main() {
    auto start = chrono::high_resolution_clock::now();
    
    Node* head1 = nullptr;
    Node* head2 = nullptr;

    cout << "輸入Poly1 : " << endl;
    head1 = create(head1, "m");
    cout << "輸入Poly2 : " << endl;
    head2 = create(head2, "n");

    polyMult(head1, head2);
    
    auto end = chrono::high_resolution_clock::now(); // 結束計時
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start);
    cout << "程式運行時間: " << duration.count() << " 微秒" << endl;

    return 0;
}




















