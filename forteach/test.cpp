#include <iostream>
  class ListNode{
      public:
        int coef;
        int exp;
        ListNode* next;
        ListNode(int c, int e): coef(c),exp(e),next (nullptr){}
    };
int main()
{
    int m;
    std::cin>> m;
    int n;
    std::cin>> n;
  
    int arr[m];
    int brr[n];
    
    for(int i=0; i<m; i++){//poly1輸入
        std::cin>> arr[i];
    }
    for( int i=0; i<m-1; i++){//poly1降冪排序 。使用selection sort
        for(int j=i+1; j< m;j++){
            if(arr[i]%10<arr[j]%10){
                std::swap(arr[i],arr[j]);
            }
        }
    }//到這邊都是poly1的輸入，並降冪排序完畢
    
    for(int i=0; i<n; i++){//poly2輸入
        std::cin>> brr[i];
    }
    for( int i=0; i<n-1; i++){//poly2降冪排序 。使用selection sort
        for(int j=i+1; j< n;j++){
            if(brr[i]%10<brr[j]%10){
                std::swap(brr[i],brr[j]);
            }
        }
    }//到這邊都是poly2的輸入，並降冪排序完畢
    
    //for(int v=0; v< m; v++){
    //std::cout<< arr[v];
    // }
    
    ListNode* dummy =nullptr;//poly1的linked list
    ListNode* tt =nullptr;
    ListNode* head= nullptr;
    for(int e =0; e<m; e++){
        if(e==0){
          head= new ListNode(arr[e]/10,arr[e]%10);
          dummy =head;
          tt=head;
        }else{
          head->next= new ListNode(arr[e]/10,arr[e]%10);

          head=head->next;
            
        }
   
    }
    
    ListNode* dummyy =nullptr;//poly2的linked list
    ListNode*ee=nullptr;
    ListNode* headd= nullptr;
    for(int e =0; e<n; e++){
        if(e==0){
          headd= new ListNode(brr[e]/10,brr[e]%10);
          dummyy =headd;
          ee=headd;
        }else{
          headd->next= new ListNode(brr[e]/10,brr[e]%10);

          headd=headd->next;
            
        }
   
    }
    std::cout<<"poly1= ";
    ListNode* current=dummy;
    while(current->next!=nullptr){
    std::cout<< current->coef<<"x^"<< current->exp<<"+";
    current=current->next;
    }
    std::cout<< current->coef<<"x^"<<current->exp;
    
    current=dummyy;
    std::cout<<"\n"<<"poly2= ";
    while(current->next!=nullptr){
    std::cout<< current->coef<<"x^"<< current->exp<<"+";
    current=current->next;
    }
    std::cout<< current->coef<<"x^"<<current->exp;
    
    

/*ListNode*result = nullptr;
bool first1=1;
for(int i =0; i<m; i++){
    ListNode* plus=nullptr;
    ListNode*hd=nullptr;
    for( int r= 0; r< n; r++){
        if(dummy!=nullptr &&dummyy!=nullptr)
        plus= new ListNode((dummy->coef)*(dummyy->coef),(dummy->exp)+(dummyy->exp));    
        while(first1){
            current=plus;
            first1=false;
        }
        
        if( r==0){
            hd= plus;
        }
        if(dummy->next!=nullptr &&dummyy->next!=nullptr)
        dummyy= dummyy->next;
        plus=plus->next;
        
    } 
    if(dummy->next!=nullptr &&dummyy->next!=nullptr)
    dummy=dummy->next;
    ListNode*answer= new ListNode(0,0);
    ListNode*start = answer;
    int go=1;
    while(go){
    if( hd->coef==0){
        answer->next= result;
    }else if(result==0){
        answer->next= hd;
    }
    else if(hd->exp == result->exp){
        answer->next= new ListNode((hd->coef)+(result->coef),hd->exp);
        answer= answer->next;
        hd=hd->next;
        result=result->next;

    }
    
    else if( hd->exp > result-> exp){
        answer->next = plus;
        hd=hd->next;
        answer= answer->next;
    }
    else if(result->exp> hd->exp){
        answer->next= result;
        result=result->next;
        answer= answer->next;
    }
    if(result==nullptr){
            go--;
        }
    }result=start->next;
    
    
}
bool first = true;
std::cout << "\npoly1*poly2= ";

while (current != nullptr) {
    if (!first) {
        std::cout << "+";
    }
    std::cout << current->coef << "x^" << current->exp;
    first = false;
    current = current->next;
}*/
ListNode* result = nullptr;

for (ListNode* p1 = dummy; p1 != nullptr; p1 = p1->next) {
    for (ListNode* p2 = dummyy; p2 != nullptr; p2 = p2->next) {
        int newCoef = p1->coef * p2->coef;
        int newExp = p1->exp + p2->exp;
        
        // Create the new term
        ListNode* newNode = new ListNode(newCoef, newExp);

        // Insert it into the result linked list
        if (result == nullptr) {
            result = newNode;
        } else {
            ListNode* temp = result;
            ListNode* prev = nullptr;
            while (temp != nullptr && temp->exp > newExp) {
                prev = temp;
                temp = temp->next;
            }
            if (temp != nullptr && temp->exp == newExp) {
                temp->coef += newCoef;
                if (temp->coef == 0) { // Remove zero coefficient terms
                    if (prev) prev->next = temp->next;
                    else result = temp->next;
                    delete temp;
                }
                delete newNode;
            } else {
                newNode->next = temp;
                if (prev) prev->next = newNode;
                else result = newNode;
            }
        }
    }
}

// Print the result
bool first = true;
std::cout << "\npoly1*poly2= ";
current = result;
while (current != nullptr) {
    if (!first) {
        std::cout << "+";
    }
    std::cout << current->coef << "x^" << current->exp;
    first = false;
    current = current->next;
}

return 0;
}