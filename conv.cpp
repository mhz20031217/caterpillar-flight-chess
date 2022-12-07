#include<bits/stdc++.h>
using namespace std;
int main(){
    ofstream conv("converted.txt");
    for(int i=0;i<80;++i){
        int id,x,y;
        cin>>id>>x>>y;
        conv<<id<<' '<<x-2<<' '<<y-7<<endl;
    }
    return 0;
}