#include <iostream>
#include <cstdlib>
#include <cmath>
#include <time.h>
using namespace std;

// 節點結構 (表示多項式中的一個項)，包含係數 (coef) 和指數 (pow)
struct term {
    int coef, pow;
    term* next;
};

// 插入新的節點到多項式中
term* insert(term* first, int coef, int pow) {
    // 如果多項式為空，直接建立並返回新節點
    if (first == NULL) {
        term* newterm = new term;
        newterm->coef = coef;
        newterm->pow = pow;
        newterm->next = NULL;
        return newterm;
    }

    // 建立新節點
    term* newterm = new term;
    newterm->coef = coef;
    newterm->pow = pow;
    newterm->next = NULL;

    // 如果新節點的指數大於等於頭節點的指數
    if (pow > first->pow) {
        newterm->next = first;
        return newterm;
    } else if (pow == first->pow) {
        // 如果指數相同，更新係數
        first->coef += coef;
        delete newterm; // 刪除未使用的新節點
        return first;
    }

    // 遍歷多項式，找到適當的位置插入
    term* current = first;
    while (current->next != NULL && current->next->pow > pow) {
        current = current->next;
    }

    if (current->next != NULL && current->next->pow == pow) {
        // 如果找到指數相同的節點，更新係數
        current->next->coef += coef;
        delete newterm; // 刪除未使用的新節點
    } else {
        // 否則插入新節點
        newterm->next = current->next;
        current->next = newterm;
    }

    return first;
}

// 輸出多項式
void showlist(struct term* pointer) {
    while (pointer->next != NULL) {
        cout << pointer->coef << "x^" << pointer->pow; // 輸出目前項目
        if (pointer->next != NULL && pointer->next->coef >= 0)
            cout << "+"; // 若下一項係數為正，輸出 "+"
        pointer = pointer->next; // 移至下一節點
    }
    cout << pointer->coef << "x^" << pointer->pow << endl; // 輸出最後一項
}



// 將兩個多項式相乘
term* multiply(term* p1, term* p2, term* p3) {
    term* ptr1, * ptr2;
    ptr1 = p1;
    ptr2 = p2;

    // 遍歷第一個多項式的每一項
    while (ptr1 != NULL) {
        // 將每一項與第二個多項式的每一項相乘
        while (ptr2 != NULL) {
            int coefftion, power;

            // 計算係數和指數
            coefftion = ptr1->coef * ptr2->coef;
            power = ptr1->pow + ptr2->pow;

            // 將新項插入結果多項式
            p3 = insert(p3, coefftion, power);
            ptr2 = ptr2->next;
        }
        ptr2 = p2; // 重置第二個多項式的指標
        ptr1 = ptr1->next; // 移至下一項
    }

    return p3;
}
// 將多項式按照指數降冪排序



int main() {
	srand(time(NULL));
    int choice,d,terms;
	term* poly1 = NULL;
	term* poly2 = NULL;
	term* poly3 = NULL;
	cout<< "請輸入1為手動輸入，2為自動輸入"<<endl;
	cin >> choice;

    if (choice == 2) { // 當選2時，自動生成隨機多項式
    	int e = 0, c = 0;
    	cout<< "請輸入1為dence，2為non dence"<<endl;
    	cin >> d;
    	cout<< "m=";
    	cin >> terms;
    	if(d==2){
        	for (int i = 0; i < terms; i++) {
           		e = rand() % terms;
            	c = 1;
            	poly1 = insert(poly1, c, e);
        	}
    	}
    	else {
    		for (int i = 0; i < terms; i++){
    			e += 1;
    			c = 1;
    			poly1 = insert(poly1, c, e);
			}
		}
    } 
	else { // 手動輸入多項式
        cout << "input coef and exp.(m=4)";
		for (int i = 0; i < 4; i++) {
            int e, c;
            cin >> c;
            cin >> e;
            poly1 = insert(poly1, c, e);
        }
    } 
    
    if (choice == 2) { // 當選2時，自動生成隨機多項式
		int e = 0, c = 0;
    	cout<< "n=";
    	cin >> terms;
    	if(d==2){
        	for (int i = 0; i < terms; i++) {
           		e = rand() % terms;
            	c = 1;
            	poly2 = insert(poly2, c, e);
        	}
    	}
    	else {
    		for (int i = 0; i < terms; i++){
    			e += 1;
    			c = 1;
    			poly2 = insert(poly2, c, e);
			}
		}
    }
	else { // 手動輸入多項式
	    cout << "input coef and exp.(n=2)";
        for (int i = 0; i < 2; i++) {
            int e, c;
            cin >> c;
            cin >> e;
            poly2 = insert(poly2, c, e);
        }
    }
     
    
	cout << "1st Polynomial: ";
    showlist(poly1);
    cout << "2nd Polynomial: ";
    showlist(poly2); 
    cout << endl;

    // 將兩個多項式相乘
    
    cout << "after multiply :";
	double start_time,end_time;
    start_time = clock(); // 計算開始時間
    poly3 = multiply(poly1, poly2, poly3);

    end_time = clock(); // 計算結束時間

    showlist(poly3);
    cout << "run time=" << (end_time - start_time) <<" ms"<< endl;

    return 0;
}
