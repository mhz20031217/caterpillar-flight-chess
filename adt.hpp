#include<string>
#include<iostream>
#include<fstream>
#include<unistd.h>
#include<climits>
#include<cctype>
#include<sstream>
/* BEGIN ADT */
std::ofstream logout("log.txt");
std::string junk;
void Error(std::string msg){
    std::cerr<<"[ERROR] "<<msg<<std::endl;
    logout<<"[ERROR] "<<msg<<std::endl;
    exit(0);
}
void Print(std::string msg){
    logout<<msg<<std::endl;
    std::cout<<msg<<std::endl;
}
void Message(std::string msg){
    logout<<"[MESSAGE] "<<msg<<std::endl;
    std::cout<<"[MESSAGE] "<<msg<<std::endl;
}
void uniusleep(int us){
    usleep(us);
}
void unisleep(int s){
    sleep(s);
}
void voidPrompt(std::string msg){
    std::cout<<msg;
    getchar();
}
int intPrompt(std::string msg, int min=INT_MIN,int max=INT_MAX){
    int ret;
    do{
        std::cout<<msg;
        fflush(stdout);
        std::cin>>ret;
    }while(ret<min||ret>=max);
    return ret;
}
double doublePrompt(std::string msg, double min=-1e18,double max=1e18){
    std::cout<<msg;
    fflush(stdout);
    double ret;
    do{
        std::cin>>ret;
    }while(ret<min||ret>=max);
    return ret;
}
std::string itos(int x){
    std::stringstream ss;
    std::string ret;
    ss<<x;ss>>ret;
    return ret;
}
/* END ADT */
