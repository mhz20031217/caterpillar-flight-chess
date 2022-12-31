#include<bits/stdc++.h>
#include<unistd.h>
#include<ao/ao.h>
#include<mpg123.h>
using namespace std;
class Mp3Player{
#define SIZE 4096
private:
    mpg123_handle *mpg123;
    int mpg123_error;
    long rate;
    int channel,encoding;
    ao_sample_format format;
    ao_device* device;
    int status;
public:
    Mp3Player(){
        status=0;
    }
    int open(string s){
        if(status!=0){
            return 1;
        }
        if(MPG123_OK!=mpg123_init()){
            return 2;
        }
        mpg123=mpg123_new(mpg123_decoders()[0],&mpg123_error);
        if(MPG123_OK!=mpg123_open(mpg123,s.c_str())){
            return 3;
        }
        mpg123_getformat(mpg123,&rate,&channel,&encoding);
        ao_initialize();
        int default_driver=ao_default_driver_id();
        memset(&format,0,sizeof(format));
        format.bits=16;
        format.channels=channel;
        format.rate=rate;
        format.byte_format=AO_FMT_LITTLE;
        device=ao_open_live(default_driver,&format,NULL);
        if(!device){
            return 4;
        }
        status=1;
        return 0;
    }
    void play(){
        if(status!=1)return;
        status=2;
        char buffer[SIZE*2];
        while(true){
            if(status!=2)break;
            size_t read=0;
            mpg123_read(mpg123,buffer,SIZE,&read);
            if(read==0||status!=2){
                break;
            }
            ao_play(device,buffer,read);
        }
        status=1;
    }
    void pause(){
        if(status!=2)return;
        status=1;
    }
    void close(){
        if(status!=1)return;
        mpg123_close(mpg123);
        ao_close(device);
        ao_shutdown();
        status=0;
    }
    ~Mp3Player(){
        if(status!=0)close();
    }
};
int main(){
    Mp3Player player;
    int x=player.open("a.mp3");
    if(x){
        cout<<x<<endl;
        return 0;
    }
    player.play();
    player.close();
    return 0;
}