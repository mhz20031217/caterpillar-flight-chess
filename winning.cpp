#include<bits/stdc++.h>
#include<gtk/gtk.h>
#include"mp3player.hpp"
#include"adt.hpp"
#ifdef WINDOWS
#include<windows.h>
#endif
using namespace std;
int x;
void music(){
    ifstream fin("audio/winning.conf");
    if(!fin){
        Error("No music config file!");
    }
    vector<string>filename;
    while(fin){
        string tmp;
        fin>>tmp;
        if(tmp=="")continue;;
        filename.push_back(tmp);
    }
    for(int i=0;;++i){
        Mp3Player player;
        if(player.open(filename[i%filename.size()])){
            unisleep(5);
            continue;
        }
        player.play();
        player.close();
        unisleep(5);
    }
}
void init(GtkApplication* app,gpointer app_data){
    thread run_music(music);
    run_music.detach();
    GtkWidget* main_window;
    main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(main_window),"You win!");
    gtk_window_set_resizable(GTK_WINDOW(main_window),FALSE);
    GtkWidget* label, *image;

    label = gtk_label_new(("Player "+itos(x)+" wins!").c_str());
    gtk_label_set_markup(GTK_LABEL(label),("<big>Player <b>"+itos(x)+"</b> wins!</big>").c_str());
    
    image = gtk_image_new_from_file("img/winning.jpg");
    
    GtkWidget* box;

    box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    gtk_box_pack_start(GTK_BOX(box),image,FALSE,FALSE,4);
    gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);

    gtk_container_add(GTK_CONTAINER(main_window),box);

    gtk_widget_show_all(main_window);
    return;
}
int main(int argc,char** argv){
    if(argc<2)return 0;
    x=stoi(argv[1]);
    argc=1;
    GtkApplication* app;
    int gui_return;
    app = gtk_application_new("org.caterpillar.FlightChess.winning",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(init),NULL);
    gui_return = g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    #ifndef WINDOWS
    system("./launcher &");
    #else
    WinExec("launcher.exe",SW_SHOW);
    #endif
    return gui_return;
}