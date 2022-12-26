#include<bits/stdc++.h>
using namespace std;
int main(){
    ofstream fout("newmap.txt");
    for(int i=0;i<80;++i){
        int id,v[4],o[5];
        cin>>id>>v[0]>>v[1]>>v[2]>>v[3];
        int x1=v[0],x2=v[0],pos=-1;
        
        for(int pt=0;pt<4;++pt){
            int cnt=0;
            x1=v[pt];
            for(int t=0;t<4;++t){
                if(x1==v[t])++cnt;
                else x2=v[t];
            }
            if(cnt==1){
                pos=pt;
                break;
            }
            if(cnt==4){
                fout<<id<<' '<<id<<' '<<id<<' '<<id<<' '<<id<<' '<<
            }
            
        }
        fout<<id<<' ';
        for(int j=0;j<4;++j){
            if(j==pos){
                fout<<x1<<' ';
            }else fout<<id<<' ';
        }
        fout<<x2<<endl;
    }
    return 0;
}