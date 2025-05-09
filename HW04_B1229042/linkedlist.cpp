#include <iostream>
#include <string>
#include<sstream>
#include "Chain.h"
using namespace std;

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
    friend istream& operator>>(istream& input, Polynomial& p) {
        string line;
        getline(input, line);

        stringstream ss(line);
        int coef, expo;
        char useless;

        while (ss >> coef) {
            if (ss.peek() == 'x') {
                ss >> useless >> useless >> expo;
            } else {
                expo = 0;
            }
            p.add(coef, expo);

            if (ss.peek() == '+' || ss.peek() == ' ') {
                ss>>useless;
            }
        }
        return input;
    };

private:
    Chain<Term> poly;
};

int main() {

    Polynomial p1, p2;

    cout << "Enter Polyn0mial 1:";
    cin >> p1;

    cout << "Enter Polyn0mial 2:";
    cin >> p2;
    Polynomial result = p1 + p2;


    cout << "Polyn0mial 1: ";
    p1.display();

    cout << "Polyn0mial 2: ";
    p2.display();

    cout << "Result: ";
    result.display(); 

    return 0;
}
