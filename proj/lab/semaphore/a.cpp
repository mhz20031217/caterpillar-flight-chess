#include<bits/stdc++.h>
#include<semaphore.h>
using namespace std;
sem_t sem;
int x;
bool flag;
void func(){
    cin>>x;
    x+=10;
    sem_post(&sem);
}
int main(){
    sem_init(&sem,0,0);
    x=-1;
    thread t(func);
    t.detach();
    sem_wait(&sem);
    cout<<x<<endl;
}