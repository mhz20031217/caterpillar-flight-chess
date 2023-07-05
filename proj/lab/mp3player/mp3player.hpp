/* mp3player.hpp */
#include<string>
#include<cstring>
#include<ao/ao.h>
#ifndef WINDOWS
#include<ao/ao.h>
#include<mpg123.h>
#else
#include<mad.h>
/*
    class libMP3 Copied and modified from http://www.cppblog.com/gaimor/archive/2013/06/09/200905.html
    http://www.cppblog.com/gaimor/
    This class is used to support mp3 playing on Windows.
    The original licence is unknown.
*/
signed short to_short(mad_fixed_t fixed){
    if(fixed>=MAD_F_ONE)
        return(SHRT_MAX);
    if(fixed<=-MAD_F_ONE)
        return(-SHRT_MAX);
    fixed=fixed>>(MAD_F_FRACBITS-15);
    return((signed short)fixed);
}
unsigned char inputBuffer[4096 + MAD_BUFFER_GUARD];
class libMP3{
public:
    libMP3(const char* filename){
        file = fopen(filename,"rb");
        mad_stream_init(&stream);
        mad_frame_init(&frame);
        mad_synth_init(&synth);
        mad_timer_reset(&timer);
    }
    ~libMP3(){
        mad_synth_finish(&synth);
        mad_frame_finish(&frame);
        mad_stream_finish(&stream);
        fclose(file);
    }
    int setup(){
        size_t    remaining;
        unsigned char* read;

        if(stream.next_frame!=NULL){
            remaining=stream.bufend-stream.next_frame;
            memmove(inputBuffer,stream.next_frame,remaining);
            read = inputBuffer+remaining;
            readsize = 4096-remaining;
        }else{
            readsize = 4096,
            read = inputBuffer,
            remaining=0;
        }
        readsize = fread(read,1,readsize,file);
        if(readsize<=0){
            return -1;
        }
        mad_stream_buffer(&stream,inputBuffer,readsize+remaining);
        char dest[100];
        mad_timer_string(timer, dest, "%02lu:%02u:%02u", MAD_UNITS_HOURS, MAD_UNITS_MILLISECONDS, 0);
        stream.error=(mad_error)0;
        return 0;
    }
    long readFrame(unsigned char* pcm){
        if(stream.buffer == NULL){
            if(setup() == -1)
                return -1;
        }
        while(mad_frame_decode(&frame,&stream) != 0){
            if(MAD_RECOVERABLE(stream.error)){
                if(stream.error!=MAD_ERROR_LOSTSYNC){}
                continue;
            }else{
                if(stream.error==MAD_ERROR_BUFLEN){
                    if (setup() == -1) return -1;
                    continue;
                }else{
                    return -1;
                }
            }
        }
        mad_synth_frame(&synth,&frame);
        //! 获取频率
        //freq = synth.pcm.samplerate;
        //bitrate = frame.header.bitrate;
        bitrate = synth.pcm.samplerate;
        channels = (frame.header.mode == MAD_MODE_SINGLE_CHANNEL) ? 1 : 2;
        mad_timer_add(&timer,frame.header.duration);
        int j = 0;
        for(int i=0;i<synth.pcm.length;++i){
            signed short    sample;
            sample=to_short(synth.pcm.samples[0][i]);
            pcm[j++] = sample&0xff;
            pcm[j++] = sample>>8;
            if(MAD_NCHANNELS(&frame.header)==2)
                sample=to_short(synth.pcm.samples[1][i]);
            pcm[j++] = sample&0xff;
            pcm[j++] = sample>>8;
        }
        char dest[120];
        mad_timer_string(timer,dest, "%lu:%02lu.%03u", MAD_UNITS_MINUTES, MAD_UNITS_MILLISECONDS, 0);
        return j;
    }
    unsigned long bitrate;
    int channels;
private:
    struct mad_stream   stream;
    struct mad_frame    frame;
    struct mad_synth    synth;
    struct mad_header   header;
    mad_timer_t         timer;
    FILE* file;
    long readsize;
};
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
    std::string filename;
    unsigned char buffer[4096*2];
    unsigned char data[4096];
    ao_device *device;
    ao_sample_format format;
    int default_driver;
    int rate,channels;
    libMP3* mp3;
public:
    Mp3Player(){
        status=0;
        mp3=NULL;
        device=NULL;
    }
    int open(std::string s){
        if(status!=0){
            return 1;
        }
        filename=s;
        {
            libMP3 mp3(filename.c_str());
            mp3.readFrame(buffer);
            rate=mp3.bitrate;
            channels=mp3.channels;
        }
        mp3=new libMP3(filename.c_str());
        ao_initialize();

        default_driver = ao_default_driver_id();
        format.bits = 16;
        format.channels = channels;
        format.rate = rate;
        format.byte_format = AO_FMT_LITTLE;
        device = ao_open_live(default_driver,&format,NULL);
        if(device == NULL){
            return 1;
        }
        status=1;
        return 0;
    }
    void play(){
        if(status!=1)return;
        status=2;
        long len = 0;
        while(len=mp3->readFrame(buffer)){
            if(status!=2){
                break;
            }
            if(len>0)ao_play(device,(char*)buffer,len);
            else{
                break;
            }
        }
        status=1;
    }
    void close(){
        if(status!=1)return;
        ao_close(device);
        ao_shutdown();
        status=0;
    }
    ~Mp3Player(){
        if(status!=0)close();
    }
#endif
};
