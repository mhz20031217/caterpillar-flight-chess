#include<bits/stdc++.h>
#include<unistd.h>
#include<ao/ao.h>
#include<mad.h>
using namespace std;
class Mp3Player{
#define SIZE 4096
private:
    mad_stream stream;
    mad_frame frame;
    mad_synth synth;
    mad_header header;
    mad_timer_t timer;
    ifstream file;
    ao_device* device;
    ao_sample_format format;
    int ao_default_driver;
    int status;
    // 0: uninitialized; 1: initialized, not playing;
    // 2: initialized, playing;
    unsigned char input_buffer[SIZE+MAD_BUFFER_GUARD];
    char time_label[100];
    signed short to_short(mad_fixed_t fixed){
        if(fixed>=MAD_F_ONE)
            return(SHRT_MAX);
        if(fixed<=-MAD_F_ONE)
            return(-SHRT_MAX);
        fixed=fixed>>(MAD_F_FRACBITS-15);
        return((signed short)fixed);
    }
    bool setup(){
        // if(!file)return 0;
        size_t remaining;
        size_t readsize;
        unsigned char* head;
        
        if(stream.next_frame!=NULL){
            remaining=stream.bufend-stream.next_frame;
            memmove(input_buffer,stream.next_frame,remaining);
            head=input_buffer+remaining;
            readsize=SIZE-remaining;
        }else{
            readsize=SIZE;
            head=input_buffer;
            remaining=0;
        }
        file.read((char*)input_buffer,readsize);
        if(!file)return 0;
        mad_stream_buffer(&stream,input_buffer,readsize+remaining);
        // mad_timer_string(timer,time_label,"%02lu:%02u:%02u",MAD_UNITS_HOURS, MAD_UNITS_MILLISECONDS, 0);
        stream.error=mad_error(0);
        return 1;
    }
    bool next_frame(unsigned char* pcm,long& length){
        if(stream.buffer==NULL){
            if(!setup())return 0;
        }
        while(mad_frame_decode(&frame,&stream)!=0){
            if(MAD_RECOVERABLE(stream.error)){
                if(stream.error!=MAD_ERROR_LOSTSYNC){}
                continue;
            }else{
                if(stream.error==MAD_ERROR_BUFLEN){
                    if(!setup())return 0;
                    continue;
                }
                return 0;
            }
        }
        mad_synth_frame(&synth,&frame);
        format.rate=synth.pcm.samplerate;
        format.channels=(frame.header.mode==MAD_MODE_SINGLE_CHANNEL)?1:2;
        // mad_timer_add(&timer,frame.header.duration);
        long j=0;
        for(int i=0;i<synth.pcm.length;++i){
            signed short sample;
            sample=to_short(synth.pcm.samples[0][i]);
            pcm[j++] = sample&0xff;
			pcm[j++] = sample>>8;
			if(MAD_NCHANNELS(&frame.header)==2)
				sample=to_short(synth.pcm.samples[1][i]);
			pcm[j++] = sample&0xff;
			pcm[j++] = sample>>8;
        }
        // mad_timer_string(timer,time_label, "%02lu:%02u:%02u.%03u", MAD_UNITS_HOURS, MAD_UNITS_MILLISECONDS, 0);
        length=j;
		return 1;
    }
public:
    Mp3Player(){
        status=0;
    }
    bool open(string filename){
        if(status==2){
            if(!close())return 1;
        }
        mad_stream_init(&stream);
        mad_frame_init(&frame);
        mad_synth_init(&synth);
        mad_timer_reset(&timer);
    
        file.open(filename.c_str(),ios::binary);
        if(!file){
            return 0;
        }
        memset(&format,0,sizeof(format));
        unsigned char pcm[SIZE*2];
        long length=0;
        next_frame(pcm,length);
        format.rate=synth.pcm.samplerate;
        format.channels=(frame.header.mode==MAD_MODE_SINGLE_CHANNEL)?1:2;
        format.bits=16;
        format.byte_format=AO_FMT_LITTLE;
        ao_initialize();
        ao_default_driver=ao_default_driver_id();
        device=ao_open_live(ao_default_driver,&format,NULL);
        if(!device){
            return 0;
        }
        file.close();
        file.open(filename.c_str(),ios::binary);
        mad_stream_init(&stream);
        mad_frame_init(&frame);
        mad_synth_init(&synth);
        mad_timer_reset(&timer);
        if(!file){
            return 0;
        }
        status=1;
        return 1;
    }
    void play(){
        if(status!=1)return;
        status=2;
        unsigned char pcm[SIZE*2];
        long length=0;
        while(true){
            if(!next_frame(pcm,length))break;
            if(!length)break;
            if(status!=2)break;
            ao_play(device,(char*)pcm,length);
        }
        status=1;
    }
    void pause(){
        if(status==2)status=1;
    }
    bool close(){
        ao_close(device);
        ao_shutdown();
        mad_synth_finish(&synth);
        mad_frame_finish(&frame);
		mad_stream_finish(&stream);
        file.close();
        status=0;
        return 1;
    }
    ~Mp3Player(){
        if(status!=0)close();
    }
};
int main(){
    Mp3Player player;
    if(!player.open("a.mp3")){
        cout<<"Error00"<<endl;
        return 0;
    }
    player.play();
    player.close();
    return 0;
}