#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <chrono>
#include <vector>
#include "Chain.h" 
using namespace std;
using namespace chrono;

void random_large_output(int m,int n,int mode1);
void random_output(int m,int n,int mode1);

struct Term {
    int coef;
    /*double coef_double;//reversed for division*/ 
    int expo; 

    Term& set(int c, int e) {
        coef = c;
        expo = e;
        return *this;
    }
    /*Term& set(double c, int e) {
        coef_double = c;
        expo = e;
        return *this;
    }       reserved for division*/

    bool operator<(const Term& other) const {
        return expo > other.expo;
    }
};

class Polynomial {
public:
    void add(int coef, int expo) {
        while (coef==0)
        {
            return;
        }
        
        auto it = poly.begin();
        auto prev = it;

        while (it != poly.end() && (*it).expo > expo) {
            prev = it;
            ++it;
        }

        if (it != poly.end() && (*it).expo == expo) {
            (*it).coef += coef;
            if ((*it).coef == 0) {
                poly.erase(it);
            }
        } else {
            Term newTerm;
            newTerm.set(coef, expo);
            if (it == poly.begin()) {
                poly.pushfront(newTerm);
            } else if (it == poly.end()) {
                poly.pushback(newTerm);
            } else {
                poly.pushspec(prev, newTerm);
            }
        }
    }
    /*void add(double coef, int expo) {
        auto it = poly.begin();
        auto prev = it;

        while (it != poly.end() && (*it).expo > expo) {
            prev = it;
            ++it;
        }

        if (it != poly.end() && (*it).expo == expo) {
            (*it).coef += coef;
            if ((*it).coef == 0) {
                poly.erase(it);
            }
        } else {
            Term newTerm;
            newTerm.set(coef, expo);
            if (it == poly.begin()) {
                poly.pushfront(newTerm);
            } else if (it == poly.end()) {
                poly.pushback(newTerm);
            } else {
                poly.pushspec(prev, newTerm);
            }
        }
    }       reserved for division*/
    Polynomial operator+(const Polynomial& b) const {
        Polynomial result;

        auto ai = poly.begin();
        auto bi = b.poly.begin();

        while (ai != poly.end() && bi != b.poly.end()) {
            if ((*ai).expo == (*bi).expo) {
                int sum = (*ai).coef + (*bi).coef;
                if (sum != 0) {
                    result.add(sum, (*ai).expo);
                }
                ++ai;
                ++bi;
            } else if ((*ai).expo > (*bi).expo) {
                result.add((*ai).coef, (*ai).expo);
                ++ai;
            } else {
                result.add((*bi).coef, (*bi).expo);
                ++bi;
            }
        }

        while (ai != poly.end()) {
            result.add((*ai).coef, (*ai).expo);
            ++ai;
        }

        while (bi != b.poly.end()) {
            result.add((*bi).coef, (*bi).expo);
            ++bi;
        }

        return result;
    }
    Polynomial operator-(const Polynomial& b) const {
        Polynomial result;

        auto ai = poly.begin();
        auto bi = b.poly.begin();

        while (ai != poly.end() && bi != b.poly.end()) {
            if ((*ai).expo == (*bi).expo) {
                int sum = (*ai).coef - (*bi).coef;
                if (sum != 0) {
                    result.add(sum, (*ai).expo);
                }
                ++ai;
                ++bi;
            } else if ((*ai).expo > (*bi).expo) {
                result.add((*ai).coef, (*ai).expo);
                ++ai;
            } else {
                result.add(-(*bi).coef, (*bi).expo);
                ++bi;
            }
        }

        while (ai != poly.end()) {
            result.add((*ai).coef, (*ai).expo);
            ++ai;
        }

        while (bi != b.poly.end()) {
            result.add(-(*bi).coef, (*bi).expo);
            ++bi;
        }

        return result;
    }

    Polynomial operator*(const Polynomial& b) const {
        Polynomial result;

        auto ai = poly.begin();
        while (ai != poly.end()) {
            auto bi = b.poly.begin();
            while (bi != b.poly.end()) {
                int newCoef = (*ai).coef * (*bi).coef;
                int newExpo = (*ai).expo + (*bi).expo;
                result.add(newCoef, newExpo);
                ++bi;
            }
            ++ai;
        }

        return result;
    }
    /*Polynomial operator/(const Polynomial& b) const {
        Polynomial result;

        auto ai = poly.begin();
        while (ai != poly.end()) {
            auto bi = b.poly.begin();
            while (bi != b.poly.end()) {
                double newCoef = (*ai).coef / (*bi).coef;
                int newExpo = (*ai).expo - (*bi).expo;
                result.add(newCoef, newExpo);
                ++bi;
            }
            ++ai;
        }

        return result;
    }               this part is not finished yet by adding some roles*/

    void display() const {
        auto it = poly.begin();
        bool isFirst = true;

        while (it != poly.end()) {
            if (!isFirst && (*it).coef > 0) {
                cout << "+";
            }

            if ((*it).expo == 0) {
                cout << (*it).coef;
            } else {
                cout << (*it).coef << "x^" << (*it).expo;
            }

            isFirst = false;
            ++it;
        }

        if (isFirst) {
            cout << "0";
        }

        cout << endl;
    }

    /*void randomize(int terms, int coefRange, int expoRange,int mode) {      //generate by AI for Check
        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> coefDist(-coefRange, coefRange);
        uniform_int_distribution<> expoDist(0, expoRange);

        if(mode==2){
            for (int i = 0; i < terms; ++i) {
                int coef = coefDist(gen);
                int expo = expoDist(gen);
                if (coef != 0) {
                    add(coef, expo);
                }
            }
        }
        else {
            
            int expo = expoDist(gen);
            for (int i = 0; i < terms; ++i) {
            int coef = coefDist(gen);
            if (coef != 0) {
                    add(coef, expo--);
                }
            }
        }
    }*/

    void randomize(int terms, int coefrange, int exporange, int mode) {      
        vector<pair<int, int>> termsList; 

        if (mode == 2) { 
            for (int i = 0; i < terms; ++i) {
                int coef = (rand() % (2 * coefrange + 1)) - coefrange; 
                int expo = rand() % (exporange + 1);                   
                if (coef != 0) {
                    termsList.emplace_back(coef, expo);
                }
            }
        } else { 
            int expo = rand() % (exporange + 1); 
            for (int i = 0; i < terms; ++i) {
                int coef = (rand() % (2 * coefrange + 1)) - coefrange; 
                if (coef != 0) {
                    termsList.emplace_back(coef, expo--);
                }
            }
        }

        
        for (const auto& term : termsList) {
            add(term.first, term.second); 
        }
    };

private:
    Chain<Term> poly;
};

int main() {
    srand(time(0)); 
    int m, n;


    int mode1;
    while(cout<<"\nEnter the poly are dense(mode 1) , nondense(mode 2) or Escape(type EOF): " && cin>>mode1){
    cout<<"Enter the number of m & n :";
    cin>>m>>n;
    cout<<"Enter Yes if you want display polyn0mial: ";
    string mode2;
    cin>>mode2;
           if (mode2 == "yes" || mode2 == "YES" || mode2 == "Y") {
            random_output(m, n, mode1);
        } else {
            random_large_output(m, n, mode1);
        }
    }
    return 0;
}
void random_large_output(int m,int n,int mode1){

    int M = m, N = n;
    while(M!=0){
        
        while(N!=0){
            Polynomial randomP1, randomP2;

            randomP1.randomize(M, rand()%10000, rand()%10000 ,mode1);   
            randomP2.randomize(N, rand()%10000, rand()%10000 ,mode1);


            Polynomial Result= randomP1 * randomP2;
            int count;
            if(mode1==2){
                count=1000;
            }
            else count=10000;
            auto start = high_resolution_clock::now();
            for(int i=0;i<count;i++){
            
                Polynomial Result_time= randomP1 * randomP2;
            
            }
            auto end = high_resolution_clock::now();


            auto duration = duration_cast<milliseconds>(end - start);
            cout << duration.count()<<endl;
            N--;
        }
        N=n;
        M--;
    }

}
void random_output(int m,int n,int mode1){
    int M = m, N = n;
    Polynomial randomP1, randomP2;

    randomP1.randomize(M, rand(), rand(),mode1);
    cout<<"The polyn0minal of P1=";
    randomP1.display();
    
    randomP2.randomize(N, rand(), rand(),mode1);
    cout<<"The polyn0minal of P2=";
    randomP2.display();

    cout << "\nTesting with polyn0mials: " << M << " terms and " << N << " terms." << endl;
    Polynomial Result= randomP1 * randomP2;

    auto start = high_resolution_clock::now();
    
    for(int i=0;i<10000;i++){
    
        Polynomial Result_time= randomP1 * randomP2;
    
    }
    auto end = high_resolution_clock::now();
    cout<<"The polyn0minal of P1 & P2 are:";
    Result.display();

    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Time taken for multiplication about 10000 times: " << duration.count() << " milliseconds" << endl;

}