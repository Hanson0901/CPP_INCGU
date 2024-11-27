#pragma GCC optimize("Ofast,unroll-loops,no-stack-protector,fast-math")
#pragma GCC target("sse,sse2,sse3,ssse3,sse4,popcnt,abm,mmx,avx,tune=native")
#include<iostream>
#include<string.h>
#include<string>
#include<vector>
#include<algorithm>
using namespace std;

class String{
    private:
        char *str;
        int length;
    public:

        String():str(),length(){}
        String(const char* init,int m){
            length=m;
            str = new char[m+1];
            strncpy(str,init,m);
            str[m]='\0';
        };
       /* ~String(){
            delete[] str;
        }*/
        bool operator==(String t){
            return strcmp(this->str,t.str)==0;
        };
        /*bool operator!();*/
        int Length(){
            return length;
        }
        
        String Concat(String t){
            string a=this->str;
            string b=t.str;
            return String((a+b).c_str(),(a+b).length());
        };
        
       /* String substr(int i,int j){
            char newchar[j-i+1];
        };*/
        int Find(String pat){
            for(int i=0;i<=Length()-pat.Length();i++){
                int j;
                for(j=0;j<pat.Length()&&str[i+j]==pat.str[j];j++){
                    if(j==pat.Length()-1){//題目給錯，舉例:pat.length為3，可J從0屬到2剛好三個數，對比大小時，pat.length應減一
                        return i;
                    }
                }
            }
            return -1;
        };
        string show_str(){
            return this->str;
        }
        string Frequency(){
            string compare=str;            
            string ans;
            while(compare.length()!=0){
                ans=ans+compare[0]+" "+"have :";
                int count=0;
                char com=compare[0];
                for(int i=0;i<compare.length();i++){
                    if(com==compare[i]){
                        count++;
                    }
                }
                compare.erase(std::remove(compare.begin(), compare.end(), com), compare.end());
                ans=ans+to_string(count)+"\n";
                
            }
            return ans;
        };
        string CharDelete(char c){
            string ans =str;
            ans.erase(std::remove(ans.begin(), ans.end(), c), ans.end());

           return  ans;
        }
        friend istream& operator>>(istream& input,String &Str){
            string temp;
            input>>temp>>Str.length;
            Str.str=new char[Str.length+1];
            strcpy(Str.str,temp.c_str());
            return input;
        }

};

int main(){
    //String s;
    //cin>>s;
   // cout<<"s's str="<<s.show_str()<<endl;
    String a("again and again", 15);
    /*
    String b("Hel",3);
    String cmp("Hello",5);
    String ab=a.Concat(b);
    cout<<"a's str="<<a.show_str()<<endl;
    cout<<"b's str="<<b.show_str()<<endl;
    cout<<"cmp's str="<<cmp.show_str()<<endl;
    cout<<"a+b="<<ab.show_str()<<endl;

    cout << "a == s? \n" << (a == s ? "Yes\n" : "No\n");
    cout << "a == cmp? \n" << (a == cmp ? "Yes\n" : "No\n");
    cout<<"a have b?\n"<<(a.Find(b)!=(-1)?"have b in "+to_string(a.Find(b)):"no b")<<"\n";*/
    //cout<<s.Frequency();
    cout<<a.CharDelete('n');
}