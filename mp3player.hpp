#include<string>
#include<cstring>
#ifndef WINDOWS
#include<ao/ao.h>
#include<mpg123.h>
#else
#include<iostream>
#include<sstream>
#include<vector>
#include<windows.h>
#include<mmsystem.h>
#pragma comment(lib,"WinMM.Lib")
extern int mp3_player_count = 0;
#endif
class Mp3Player{
#ifndef WINDOWS
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
    int open(std::string s){
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
#else
private:
    int status;
    int id;
    std::string name;
    inline std::string itos(int x){
        std::stringstream ss;
        std::string ret;
        ss<<x;ss>>ret;
        return ret;
    }
public:
    Mp3Player(){
        status=0;
    }
    int open(std::string s){
        if(status!=0){
            return 1;
        }
        id=mp3_player_count++;
        name=s;
        TCHAR filename[MAX_PATH];
        wsprintf(filename,s.c_str());
        TCHAR short_name[MAX_PATH];
        GetShortPathName(filename,short_name,sizeof(short_name)/sizeof(TCHAR));
        TCHAR cmd[MAX_PATH+10];
        wsprintf(cmd,("open %s alias music"+itos(id)).c_str(),short_name);
        mciSendString(cmd,NULL,0,NULL); 
        status=1;
        return 0;
    }
    void play(){
        if(status!=1)return;
        status=2;
        mciSendString(("play music"+itos(id)).c_str(),NULL,0,NULL);
        status=1;
    }
    void pause(){
        if(status!=2)return;
        status=1;
    }
    void close(){
        if(status!=1)return;
        mciSendString(("close music"+itos(id)).c_str(),NULL,0,NULL);
        status=0;
    }
    ~Mp3Player(){
        if(status!=0)close();
    }
#endif
};