#include<bits/stdc++.h>
#include<gtk/gtk.h>
#include"mp3player.hpp"
#include"adt.hpp"
#ifdef WINDOWS
#include<windows.h>
#endif
using namespace std;
const string CONFIG_FILE_NAME="ctpchess.conf";
int number_of_players,control_mode;
vector<int>v;
string map_path,audio_path;
void save_config(GtkWidget* widget=NULL,gpointer data=NULL){
    ofstream fout(CONFIG_FILE_NAME);
    fout<<number_of_players<<endl;
    for(int i=0;i<number_of_players;++i){
        fout<<v[i]<<' ';
    }
    fout<<endl;
    fout<<control_mode<<endl<<map_path<<endl<<audio_path<<endl;
}
void factory(){
    number_of_players=4;
    control_mode=0;
    v={1,0,0,0};
    map_path="map/classic/classic.conf";
    audio_path="audio/ingame.conf";
}
void save_and_start_game(GtkWidget* widget=NULL,gpointer data=NULL){
    save_config();
    #ifndef WINDOWS
    system("./game &");
    #else
    WinExec("game.exe",SW_SHOW);
    #endif
    exit(0);
}
void quit_without_saving(GtkWidget* widget=NULL,gpointer data=NULL){
    exit(0);
}
void load_config(GtkWidget* widget=NULL,gpointer data=NULL){
    ifstream conf(CONFIG_FILE_NAME),mapin;
    if(!conf){
        factory();
        return;
    }
    conf>>number_of_players;
    v.resize(number_of_players);
    for(int i=0;i<number_of_players;++i){
        if(!conf){
            factory();
            return;
        }
        conf>>v[i];
    }
    conf>>control_mode;
    conf>>map_path;
    if(number_of_players<2||number_of_players>100){
        factory();
        return;
    }
    mapin.open(map_path.c_str());
    if(!mapin){
        factory();
        return;
    }
    mapin.close();
    conf>>audio_path;
    if(map_path==""||audio_path==""){
        factory();
        return;
    }
}
void change_map(GtkFileChooserButton* button,gpointer data){
    map_path = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(button));
    ifstream testin(map_path.c_str());
    if(!testin){
        factory();
        return;
    }
    string junk;
    int nop=0;
    testin>>junk>>nop;
    if(nop!=number_of_players){
        factory();
        return;
    }
}
void change_nop(GtkSpinButton *spin_button,gpointer data){
    number_of_players=gtk_spin_button_get_value(spin_button);
    v.resize(gtk_spin_button_get_value(spin_button));
}
void music(){
    ifstream fin("audio/launcher.conf");
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
gboolean
change_type(GtkSwitch* widget,gboolean state,gpointer data){
    *(int*)data=((state)?0:1);
    return TRUE;
}
static void create_change_player_type_window(GtkButton* button,gpointer data){
    GtkWidget* window;
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"Is the player a computer?");
    GtkWidget* list;
    list = gtk_list_box_new();
    for(int i=number_of_players-1;i>=0;--i){
        GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
        GtkWidget *label,*sw;
        label = gtk_label_new(("Player "+itos(i)).c_str());
        sw = gtk_switch_new();
        gtk_switch_set_state(GTK_SWITCH(sw),gboolean(!v[i]));
        g_signal_connect(sw,"state-set",G_CALLBACK(change_type),&v[i]);
        gtk_box_pack_start(GTK_BOX(box),label,FALSE,FALSE,0);
        gtk_box_pack_end(GTK_BOX(box),sw,FALSE,FALSE,0);
        gtk_list_box_prepend(GTK_LIST_BOX(list),box);
    }
    gtk_container_add(GTK_CONTAINER(window),list);
    gtk_widget_show_all(window);
}
void init(GtkApplication* app,gpointer app_data){
    thread run_music(music);
    run_music.detach();
    load_config();
    GtkWidget* main_window;
    main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(main_window),"FlightChess Launcher");
    gtk_window_set_default_size(GTK_WINDOW(main_window),800,600);
    gtk_window_set_resizable(GTK_WINDOW(main_window),FALSE);
    gtk_window_set_gravity(GTK_WINDOW(main_window),GDK_GRAVITY_SOUTH_WEST);

    GtkWidget* main_overlay;
    GtkWidget* main_background;
    main_overlay = gtk_overlay_new();
    main_background = gtk_image_new_from_file("img/background.png");
    gtk_widget_set_halign(main_background,GTK_ALIGN_START);
    gtk_widget_set_valign(main_background,GTK_ALIGN_START);
    

    gtk_container_add(GTK_CONTAINER(main_overlay),main_background);

    GtkWidget* main_box;
    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    gtk_widget_set_size_request(main_box,800,600);

    GtkWidget* toolbar_overlay;
    toolbar_overlay = gtk_overlay_new();
    GtkWidget* toolbar_background;
    toolbar_background = gtk_image_new_from_file("img/toolbar.png");
    gtk_container_add(GTK_CONTAINER(toolbar_overlay),toolbar_background);

    GtkWidget* toolbar;
    toolbar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    gtk_overlay_add_overlay(GTK_OVERLAY(toolbar_overlay),toolbar);
    gtk_box_pack_end(GTK_BOX(main_box),toolbar_overlay,FALSE,FALSE,0);
    gtk_widget_set_opacity(toolbar_background,0.8);
    gtk_widget_set_size_request(toolbar,800,32);


    GtkAdjustment* nop_adjust;
    GtkWidget* nop_spin_button;
    GtkWidget* nop_label = gtk_label_new("Player(s):");
    nop_adjust = gtk_adjustment_new(number_of_players,2,100,1,1,0);
    nop_spin_button = gtk_spin_button_new(nop_adjust,0.5,0);
    gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(nop_spin_button),TRUE);
    g_signal_connect(nop_spin_button,"value-changed",G_CALLBACK(change_nop),NULL);

    GtkWidget* change_type_button;
    change_type_button = gtk_button_new_from_stock("gtk-properties");
    g_signal_connect(change_type_button,"released",G_CALLBACK(create_change_player_type_window),NULL);

    gtk_box_pack_start(GTK_BOX(toolbar),nop_label,FALSE,FALSE,4);
    gtk_box_pack_start(GTK_BOX(toolbar),nop_spin_button,FALSE,FALSE,4);
    gtk_box_pack_start(GTK_BOX(toolbar),change_type_button,FALSE,FALSE,0);

    GFile* cur_file = g_file_new_for_path(map_path.c_str());
    GtkWidget* map_button;
    GtkFileChooserAction map_action = GTK_FILE_CHOOSER_ACTION_OPEN;
    map_button = gtk_file_chooser_button_new("Open a map file",map_action);
    gtk_file_chooser_set_file(GTK_FILE_CHOOSER(map_button),cur_file,NULL);
    gtk_file_chooser_set_current_folder_file(GTK_FILE_CHOOSER (map_button),cur_file,NULL);
    g_signal_connect(map_button,"file-set",G_CALLBACK(change_map),NULL);
    g_object_unref(cur_file);

    GtkWidget* map_label;
    map_label = gtk_label_new("Map file:");

    gtk_box_pack_start(GTK_BOX(toolbar),map_label,FALSE,FALSE,4);
    gtk_box_pack_start(GTK_BOX(toolbar),map_button,FALSE,FALSE,4);

    GtkWidget* start_button;
    start_button = gtk_button_new_with_label("Start");
    g_signal_connect(start_button,"released",G_CALLBACK(save_and_start_game),NULL);
    gtk_box_pack_end(GTK_BOX(toolbar),start_button,FALSE,FALSE,0);

    gtk_overlay_add_overlay(GTK_OVERLAY(main_overlay),main_box);
    gtk_container_add(GTK_CONTAINER(main_window),main_overlay);
    gtk_widget_show_all(main_window);

    return;
}
int main(int argc,char** argv){
    GtkApplication* app;
    int gui_return;

    app = gtk_application_new("org.caterpillar.FlightChess.launcher",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(init),NULL);
    gui_return = g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return gui_return;
}