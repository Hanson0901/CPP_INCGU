#include <iostream>
#include<string>
#include<cmath> 
using namespace std;
string n="Fisrt" ;
class Quadratic {
private:
    double a, b, c; 

public:

    Quadratic() : a(0), b(0), c(0) {}//overload 空值
    Quadratic(double a, double b, double c) : a(a), b(b), c(c) {}//繼承a,b,c之值


    Quadratic operator+(Quadratic &num){ //member operater
        return Quadratic(a + num.a, b + num.b, c + num.c);
    }
    friend ostream& operator<<(ostream &output, Quadratic &num) {
        output << num.a << "x^2 + " << num.b << "x + " << num.c;
        return output;
    }

    friend istream& operator>>(istream &input_num, Quadratic &num) {//operater >> and << should be friend
        cout << "The "<<n<<" polyn0mial a, b, c 's value: ";
        input_num >> num.a >> num.b >> num.c;
        n="second";
        return input_num;

    }
    double eval(double x);
};

double Quadratic::eval(double x){

    return a*pow(x,2)+b*x+c;
};

int main() {
    Quadratic q1, q2, q3;
    double x;
    // input 
    cout<<q1<<endl;
    cin >> q1;
    cin >> q2;
    cout<<"X equal to :";
    cin>>x;
    //add
    q3 = q1 + q2;

    cout << "Sum of polyn0mials: " << q3 << endl;
    
    cout<<"the eval of x with polyn0mial is :"<<q3.eval(x)<<"\n";
    return 0;
}
