#include"mp3player.hpp"
#include<bits/stdc++.h>
using namespace std;
void play(string s){
	Mp3Player player;
	if(player.open(s))cout<<"ERROR\n";
	player.play();
}
int main(){
	thread t1(play,"a.mp3"),t2(play,"b.mp3");
	t1.join();
	t2.join();
}
