#include<bits/stdc++.h>
#include<semaphore.h>
#include<gtk/gtk.h>
using namespace std;
/* BEGIN DEBUG */
//#define DEBUG
ofstream logout("log.txt");
/* END DEBUG */

/* BEGIN CONSTANT */
const string CONFIG_FILE_NAME="ctpchess.conf";
const string APP_PACKAGE_NAME="org.caterpillar.flightchess";
const string IMG_DICE="img/dice.png";
const string IMG_USER="img/user.png";
const int NUMBER_OF_CHESSES=4;
const int INTERVAL_USECOND=50000;
/* END CONSTANT */

/* BEGIN ADT */
string junk;
static void Error(string msg){
    cerr<<"[ERROR] "<<msg<<endl;
    logout<<"[ERROR] "<<msg<<endl;
    exit(0);
}
static void Print(string msg){
    logout<<msg<<endl;
    cout<<msg<<endl;
}
static void Message(string msg){
    logout<<"[MESSAGE] "<<msg<<endl;
    cout<<"[MESSAGE] "<<msg<<endl;
}
static void uniusleep(int us){
    usleep(us);
}
static void unisleep(int s){
    sleep(s);
}
static void voidPrompt(string msg){
    cout<<msg;
    getchar();
}
static int intPrompt(string msg, int min=INT_MIN,int max=INT_MAX){
    int ret;
    do{
        cout<<msg;
        fflush(stdout);
        cin>>ret;
    }while(ret<min||ret>=max);
    return ret;
}
static double doublePrompt(string msg, double min=-INFINITY,double max=INFINITY){
    cout<<msg;
    fflush(stdout);
    double ret;
    do{
        cin>>ret;
    }while(ret<min||ret>=max);
    return ret;
}
string itos(int x){
    stringstream ss;
    string ret;
    ss<<x;ss>>ret;
    return ret;
}
/* END ADT */

/* BEGIN CLASS DEF */
class Chess;
class Place;
class Player;
class GameState;
class Place{
public:
    int id;
    int x,y;
    Place *entrance;
    vector<Place*>exit;
    vector<Place*>special_exit;
    set<Chess*>chesses;
    Place();
    void set_coordinate(int x,int y);
    void set_exit(vector<Place*>exit,vector<Place*>special_exit);
    void set_entrance(Place* entrance);
    Place* get_exit(Chess* chess);
    Place* get_special_exit(Chess* chess);
    Place* get_entrance(Chess* chess);
    void set_id(int id);
    void add_chess(Chess* chess);
    void remove_chess(Chess* chess);
    void remove_opponent(Chess* chess);
};
class Chess{
public:
    Player* player;
    Place* place;
    string color;
    GtkWidget* widget;
    Chess(Player* player);
    Place* get_dest(int step);
    void move_to(Place* place);
    void move(int step);
    bool conflict(int step);
    void set_place(Place* place);
    void gui_follow_dest(int step);
    void set_color(string color);
    void gui_show();
    void gui_clicked();
    ~Chess();
};
class Player{
public:
    int id;
    int number_of_chess;
    int type;
    string color;
    vector<Chess*>chesses;
    Place* airport;
    Player();
    void set_id(int id);
    void init_chesses(int n);
    void set_airport(Place* airport);
    void set_color(string color);
    void set_type(int type);
    void action();
    void take_back(Chess* chess);
    ~Player();
};
class GameState{
public:
    vector<Player*>players;
    vector<Place*>places;
    GtkWidget* board;
    int number_of_players;
    int number_of_places;
    Place* exit;
    GtkWidget* dice_label;
    GtkWidget* user_label;
    int dice_value;
    bool control_mode;
    GameState();
    void gui_set_elements(GtkWidget* board,GtkWidget* dice_label,GtkWidget* user_label);
    void read_map(istream& in);
    void set_player_type(vector<int>v);
    int main_loop();
    int dice();
    int gui_dice();
    int roll();
    ~GameState();
}gamestate;
/* END CLASS DEF */

/* BEGIN SIMPLE SIGNALS */
const int C_SIGNAL_COUNT=2;
// 0: dice, 1: chesses
sem_t c_traffic_light[C_SIGNAL_COUNT];
bool c_flag[C_SIGNAL_COUNT];
union CSiganlMessage{
    int integer;
    double real;
    char ch;
    string s;
    Place* place;
    Chess* chess;
    Player* player;
}c_data[C_SIGNAL_COUNT];
void c_signal_init(){
    for(int i=0;i<C_SIGNAL_COUNT;++i){
        sem_init(&c_traffic_light[i],NULL,0);
        c_flag[i]=0;
        memset(&c_data[i],0,sizeof(c_data[i]));
    }
}
void c_signal_allow(int i){
    assert(i>=0&&i<C_SIGNAL_COUNT);
    c_flag[i]=1;
    sem_wait(&c_traffic_light[i]);
}
bool c_signal_request(int i){
    assert(i>=0&&i<C_SIGNAL_COUNT);
    if(c_flag[i]==0)return 0;
    c_flag[i]=0;
    return 1;
}
void c_signal_finish(int i){
    assert(i>=0&&i<C_SIGNAL_COUNT);
    sem_post(&c_traffic_light[i]);
}
/* END SIMPLE SIGNALS */

/* BEGIN CHESS */
Chess::Chess(Player* player){
    this->player = player;
    this->place = NULL;
    this->widget = NULL;
}
Place* Chess::get_dest(int step){
    Place* cur = this->place;
    int cnt = 0;
    for(;cnt<step;++cnt){
        if(cur->get_exit(this)==gamestate.exit){
            if(step==cnt+1){
                ++cnt;
                cur = cur->get_exit(this);
                break;
            }else{
                break;
            }
        }
        cur = cur->get_exit(this);
    }
    for(;cnt<step;++cnt){
        cur = cur->get_entrance(this);
    }
    cur = cur->get_special_exit(this);
    return cur;
}
void Chess::move(int step){
    this->gui_follow_dest(step);
    Place* dest = this->get_dest(step);
    this->move_to(dest);
}
void Chess::gui_follow_dest(int step){
    Place* cur = this->place;
    int cnt = 0;
    for(;cnt<step;++cnt){
        if(cur->get_exit(this)==gamestate.exit){
            if(step==cnt+1){
                ++cnt;
                cur = cur->get_exit(this);
                gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,cur->x,cur->y);
                uniusleep(INTERVAL_USECOND);
                break;
            }else{
                break;
            }
        }
        cur = cur->get_exit(this);
        gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,cur->x,cur->y);
        uniusleep(INTERVAL_USECOND);
    }
    for(;cnt<step;++cnt){
        cur = cur->get_entrance(this);
        gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,cur->x,cur->y);
        uniusleep(INTERVAL_USECOND);
    }
    cur = cur->get_special_exit(this);
    gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,cur->x,cur->y);
    uniusleep(INTERVAL_USECOND);
}
void Chess::move_to(Place* place){
    Place* cur = this->place;
    cur->remove_chess(this);
    place->add_chess(this);
    this->place = place;
    gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,place->x,place->y);
    uniusleep(INTERVAL_USECOND);
}
void Chess::set_place(Place* place){
    this->place = place;
    place->add_chess(this);
}
void Chess::gui_show(){
    this->widget = gtk_image_new_from_file(this->color.c_str());
    gtk_widget_show(this->widget);
    gtk_fixed_put(GTK_FIXED(gamestate.board),this->widget,this->place->x,this->place->y);
}
void Chess::gui_clicked(){
    if(!c_signal_request(1))return;
    c_data[1].chess=this;
    c_signal_finish(1);
}
void Chess::set_color(string color){
    this->color = color;
}
Chess::~Chess(){
    g_object_unref(this->widget);
}
/* END CHESS */

/* BEGIN PLACE */
Place::Place(){
    this->id = -1;
    this->x = -1;
    this->y = -1;
    this->entrance = NULL;
}
void Place::set_id(int id){
    this->id = id;
}
void Place::set_coordinate(int x,int y){
    this->x = x;
    this->y = y;
}
void Place::set_entrance(Place* entrance){
    this->entrance = entrance;
}
void Place::set_exit(vector<Place*>exit,vector<Place*>special_exit){
    this->exit = exit;
    this->special_exit = special_exit;
}
Place* Place::get_entrance(Chess* chess){
    return this->entrance;
}
Place* Place::get_exit(Chess* chess){
    return this->exit[chess->player->id];
}
Place* Place::get_special_exit(Chess* chess){
    return this->special_exit[chess->player->id];
}
void Place::add_chess(Chess* chess){
    Print("Request add of Chess (player: "+itos(chess->player->id)+" place: "+itos(chess->place->id)+") to "+itos(this->id)+".");
    if(this!=gamestate.exit)this->remove_opponent(chess);
    this->chesses.insert(chess);
}
void Place::remove_chess(Chess* chess){
    Print("Request removal of Chess (player: "+itos(chess->player->id)+" place: "+itos(chess->place->id)+") from "+itos(this->id)+".");
    assert(this->chesses.find(chess)!=this->chesses.end());
    this->chesses.erase(chess);
}
void Place::remove_opponent(Chess* chess){
    set<Chess*>tmp = this->chesses;
    for(set<Chess*>::iterator it=tmp.begin();it!=tmp.end();++it){
        if((*it)->player->id!=chess->player->id){
            // this->remove_chess(*it);
            Print("Removing opponent Chess (player: "+itos(chess->player->id)+" place: "+itos(chess->place->id)+") from "+itos(this->id)+".");
            (*it)->player->take_back((*it));
        }
    }
}
/* END PLACE */

/* BEGIN PLAYER */
Player::Player(){
    this->id = -1;
    this->number_of_chess = -1;
    this->type = -1;
    this->color = "ERROR";
    this->airport = NULL;
}
void Player::set_id(int id){
    this->id = id;
}
void Player::set_airport(Place* airport){
    this->airport = airport;
};
void Player::set_color(string color){
    this->color = color;
}
void Player::set_type(int type){
    this->type = type;
}
void Player::init_chesses(int n){
    assert(this->id!=-1);
    assert(this->airport!=NULL);
    assert(this->color!="ERROR");
    assert(this->id!=-1);
    this->number_of_chess=n;
    this->chesses.resize(n);
    for(int i=0;i<n;++i){
        this->chesses[i]=new Chess(this);
        this->chesses[i]->set_color(this->color);
        this->chesses[i]->set_place(this->airport);
        this->chesses[i]->gui_show();
    }
}
void Player::action(){
    vector<Chess*>&chesses = this->chesses;
    if(this->type == 1){ // RealUser
        bool has_chess_in_airport=false;
        Chess* first_in_airport_chess=NULL;
        for(int i=0;i<chesses.size();++i){
            if(chesses[i]->place==this->airport){
                has_chess_in_airport=true;
                first_in_airport_chess=chesses[i];
                break;
            }
        }
        if(!has_chess_in_airport){
            Print("User "+itos(this->id)+", you have no planes in your airport.\nPlease roll dice.");
            int dice_res=gamestate.roll();
            vector<Chess*>candidate;
            for(int i=0;i<chesses.size();++i){
                if(chesses[i]->place==gamestate.exit)continue;
                candidate.push_back(this->chesses[i]);
            }
            Print("Candidates:");
            for(int i=0;i<candidate.size();++i){
                Print("Chess "+itos(i)+": "+itos(candidate[i]->place->id));
            }
            int bestp=intPrompt("Choose a candidate to move: ",0,candidate.size());
            candidate[bestp]->move(dice_res);
        }else{
            Print("User "+itos(this->id)+" you still have plane(s) in your airport.\nPlease roll dice.");
            int dice_res1=gamestate.roll();
            if(dice_res1==6){
                first_in_airport_chess->move(1);
                Print("Great! You got 6 and please roll again to decide its step.");
                int dice_res2=gamestate.roll();
                first_in_airport_chess->move(dice_res2);
            }
        }
    }else if(this->type == 0){ // Computer
        bool has_chess_in_airport=false;
        Chess* first_in_airport_chess=NULL;
        for(int i=0;i<chesses.size();++i){
            if(chesses[i]->place==this->airport){
                has_chess_in_airport=true;
                first_in_airport_chess=chesses[i];
                break;
            }
        }
        if(!has_chess_in_airport){
            Print("Computer "+itos(this->id)+" has no planes in its airport.");
            int dice_res=gamestate.dice();
            Print("It rolls the dice and gets the value of "+itos(dice_res));
            vector<Chess*>candidate;
            for(int i=0;i<this->chesses.size();++i){
                if(this->chesses[i]->place==this->airport||this->chesses[i]->place==gamestate.exit)continue;
                candidate.push_back(this->chesses[i]);
            }
            int bestn=0,bestp=0;
            if(candidate.size()==0)Error("Impossible situation!");
            // for(int i=0;i<candidate.size();++i){
            //     int value=(candidate[i]->get_dest(dice_res)->get_exit(candidate[i])!=candidate[i]->get_dest(dice_res)->get_special_exit(candidate[i]))?5:1;
            //     if(value>bestn){
            //         bestn=value;
            //         bestp=i;
            //     }
            // }
            Print("And chooses the chess at "+itos(candidate[bestp]->place->id)+" to move.");
            candidate[bestp]->move(dice_res);
        }else{
            Print("Computer still has plane(s) in its airport. It rolls the dice to decide whether a plane will take off.");
            int dice_res1=gamestate.dice();
            if(dice_res1==6){
                Print("The result is 6 and a plane takes off.");
                first_in_airport_chess->move(1);
                Print("It rolls the dice again to decide its step.");
                int dice_res2=gamestate.dice();
                first_in_airport_chess->move(dice_res2);
            }
        }
    }
}
void Player::take_back(Chess* chess){
    chess->move_to(this->airport);
}
Player::~Player(){
    for(int i=0;i<this->chesses.size();++i){
        delete this->chesses[i];
    }
}
/* END PLAYER */

/* BEGIN GAMESTATE */
GameState::GameState(){
    this->number_of_places = -1;
    this->number_of_players = -1;
    this->exit = NULL;
    this->board = this->dice_label = this->user_label = NULL;
}
void GameState::gui_set_elements(GtkWidget* board,GtkWidget* dice_label,GtkWidget* user_label){
    this->board = board;
    this->dice_label = dice_label;
    this->user_label = user_label;
}
void GameState::set_player_type(vector<int>v){
    assert(v.size()==this->number_of_players);
    for(int i=0;i<this->number_of_players;++i){
        this->players[i]->set_type(v[i]);
    }
}
void GameState::read_map(istream& in){
    int tmp;
    in >> tmp;
    if(tmp!=this->number_of_players){
        Error("The map is incapable for this number of player(s).");
    }
    in >> this->number_of_places;
    Print("Number of places: "+itos(number_of_places));

    this->places.resize(this->number_of_places);
    for(int i=0;i<this->number_of_places;++i){
        this->places[i]=new Place();
        this->places[i]->set_id(i);
    }
    for(int i=0;i<this->number_of_places;++i){
        int id,entrance;
        vector<Place*>exits(number_of_players),special_exits(number_of_players);
        in>>id>>entrance;
        this->places[i]->set_entrance(this->places[entrance]);
        for(int j=0;j<this->number_of_players;++j){
            int x;
            in>>x;
            if(x<0||x>=this->number_of_places){
                Error("Invalid map file! #1");
            }
            exits[j]=this->places[x];
        }
        for(int j=0;j<this->number_of_players;++j){
            int x;
            in>>x;
            if(x<0||x>=this->number_of_places){
                Error("Invalid map file! #2");
            }
            special_exits[j]=this->places[x];
        }
        this->places[i]->set_exit(exits,special_exits);
    }
    int number_of_visible_places;
    in>>number_of_visible_places;
    Print("Number of visible places: "+itos(number_of_visible_places));
    for(int i=0;i<number_of_visible_places;++i){
        int id,x,y;
        in>>id>>x>>y;
        this->places[i]->set_coordinate(x,y);
    }
    in>>tmp;
    if(tmp<0||tmp>=this->number_of_places){
        Error("Invalid map file! #3: "+itos(tmp));
    }

    this->exit=this->places[tmp];

    this->players.resize(number_of_players);
    for(int i=0;i<this->number_of_players;++i){
        string color;
        in>>color;
        this->players[i]=new Player();
        this->players[i]->set_id(i);
        this->players[i]->set_color(color);
    }
    for(int i=0;i<this->number_of_players;++i){
        int airport;
        in>>airport;
        this->players[i]->set_airport(this->places[airport]);
    }
    for(int i=0;i<this->number_of_players;++i){
        this->players[i]->init_chesses(NUMBER_OF_CHESSES);
    }
}
int GameState::main_loop(){
    for(int round=0;;++round){
        Print("\n\nRound "+itos(round));
        for(int i=0;i<this->number_of_players;++i){
            uniusleep(INTERVAL_USECOND);
            Print("Player "+itos(i)+":");
            gtk_label_set_label(GTK_LABEL(this->user_label),("Player: "+itos(i+1)+"/"+itos(this->number_of_players)).c_str());
            this->players[i]->action();
            vector<Chess*>&chesses=this->players[i]->chesses;
            for(int j=0;j<this->players[i]->number_of_chess;++j){
                Print("Player "+itos(i)+": Chess "+itos(j)+": "+itos(chesses[j]->place->id));
            }
            bool flag=true;
            for(int j=0;j<this->players[i]->number_of_chess;++j){
                if(chesses[j]->place!=this->exit){
                    flag=false;
                    break;
                }
            }
            if(flag){
                return i;
            }
        }
    }
    Error("Main loop returns\n");
    return -1;
}
int GameState::dice(){
    # ifdef DEBUG
    return intPrompt("Dice value: ",1,7);
    # endif
    int x=rand()%6+1;
    Print("Dice value: "+string(1,x+'0'));
    gtk_label_set_label(GTK_LABEL(this->dice_label),("Dice: "+itos(x)).c_str());
    return x;
}
void dice_clicked(GtkImage* button,gpointer dice_popup){
    
}
int GameState::gui_dice(){
    c_signal_allow(0);
    int x=rand()%6+1;
    Print("Dice value: "+string(1,x+'0'));
    gtk_label_set_label(GTK_LABEL(this->dice_label),("Dice: "+itos(x)).c_str());
    return x;
}
int GameState::roll(){
    # ifdef DEBUG
    return intPrompt("Dice value: ",1,7);
    # endif
    uniusleep(INTERVAL_USECOND);
    int dice_res;
    if(this->control_mode){
        voidPrompt("Roll dice? ");
        dice_res=this->dice();
    }else{
        dice_res=this->gui_dice();
    }
    return dice_res;
}
GameState::~GameState(){
    for(int i=0;i<this->places.size();++i){
        delete this->places[i];
    }
    for(int i=0;i<this->places.size();++i){
        delete this->places[i];
    }
}
/* END GAMESTATE */

static void running(){
    int result = gamestate.main_loop();
    GtkWidget* win_prompt;
    win_prompt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win_prompt),"YOU WIN!");
    gtk_container_set_border_width(GTK_CONTAINER(win_prompt),16);
    //gtk_window_set_position(GTK_WINDOW(win_prompt),GTK_WIN_POS_CENTER);
    GtkWidget* label;
    label = gtk_label_new(("Player "+itos(result+1)+" WINS!").c_str());
    gtk_container_add(GTK_CONTAINER(win_prompt),label);
    gtk_widget_show_all(win_prompt);
    // gtk_window_activate_focus(GTK_WINDOW(win_prompt));
    unisleep(1000);
}

static void init(GtkApplication* app,gpointer* app_data){
    srand(time(NULL));
    Print("Caterpillar Flight Chess V1.3.0\n");
    ifstream conf(CONFIG_FILE_NAME),mapin;
    if(!conf){
        Error("No config file!");
    }
    string map_path;
    conf>>gamestate.number_of_players;
    vector<int>type_of_players(gamestate.number_of_players);
    for(int i=0;i<gamestate.number_of_players;++i){
        if(!conf)Error("Invalid users definition!");
        conf>>(type_of_players[i]);
    }
    conf>>map_path;
    if(gamestate.number_of_players<2||gamestate.number_of_players>100){
        Error("Invalid number_of_players!");
    }
    mapin.open(map_path.c_str());
    if(!mapin){
        Error("Invalid map filename!");
    }
    Print("Reading from Map description file: "+map_path);
    GtkWidget* main_window;
    main_window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(main_window),"Flight Chess");
    gtk_window_set_default_size(GTK_WINDOW(main_window),1080,840);

    GtkWidget* main_box;
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);

    GtkWidget *board_overlay,*board_background,*real_board;
    board_overlay = gtk_overlay_new();

    string board_background_filename;
    mapin>>board_background_filename;

    board_background = gtk_image_new_from_file(board_background_filename.c_str());
    gtk_widget_set_halign(board_background,GTK_ALIGN_START);
    gtk_widget_set_valign(board_background,GTK_ALIGN_START);

    gtk_container_add(GTK_CONTAINER(board_overlay),board_background);

    real_board = gtk_fixed_new();
    gtk_widget_set_halign(real_board,GTK_ALIGN_START);
    gtk_widget_set_valign(real_board,GTK_ALIGN_START);
    gtk_overlay_add_overlay(GTK_OVERLAY(board_overlay),real_board);
    gtk_overlay_reorder_overlay(GTK_OVERLAY(board_overlay),real_board,10000);
    gtk_widget_show_all(board_overlay);
    
    GtkWidget* separator;
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    GtkWidget* panel;
    panel = gtk_list_box_new();

    GtkWidget* panel_box;
    panel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);

    GtkWidget *panel_user_box,*panel_dice_box;
    panel_user_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    panel_dice_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    GtkWidget *user_image, *user_label, *dice_image, *dice_label;
    GtkWidget *dice_event_box;
    user_image = gtk_image_new_from_file(IMG_USER.c_str());
    dice_image = gtk_image_new_from_file(IMG_DICE.c_str());
    dice_event_box = gtk_event_box_new();
    gtk_container_add(GTK_CONTAINER(dice_event_box), dice_image);
    g_signal_connect(G_OBJECT(dice_event_box),"button_press_event",G_CALLBACK(dice_clicked),dice_image);

    user_label = gtk_label_new("Player: 0/0");
    dice_label = gtk_label_new("Dice: NULL");

    gtk_box_pack_end(GTK_BOX(panel_user_box),user_label,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(panel_user_box),user_image,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(panel_dice_box),dice_label,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(panel_dice_box),dice_event_box,FALSE,FALSE,FALSE);
    
    gtk_box_pack_start(GTK_BOX(panel_box),panel_user_box,FALSE,FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(panel_box),panel_dice_box,FALSE,FALSE,FALSE);

    

    GtkWidget *log_info, *list_of_state;
    GtkTextBuffer *log_info_buffer;
    log_info = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_info), FALSE);
    log_info_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_info));
    

    gtk_list_box_prepend(GTK_LIST_BOX(panel),log_info);
    gtk_list_box_prepend(GTK_LIST_BOX(panel),panel_box);



    gtk_box_pack_start(GTK_BOX(main_box),board_overlay,TRUE,TRUE,FALSE);
    gtk_box_pack_start(GTK_BOX(main_box),separator,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(main_box),panel,FALSE,FALSE,FALSE);

    gtk_container_add(GTK_CONTAINER(main_window),main_box);
    gtk_widget_show_all(main_window);


    GtkWidget* dice_popup;
    dice_popup = gtk_window_new(GTK_WINDOW_POPUP);
    GtkWidget* box;
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    GtkWidget* image;
    image = gtk_image_new_from_file(IMG_DICE.c_str());
    GtkWidget* button;
    button = gtk_button_new_with_label("ROLL");
    gtk_box_pack_start(GTK_BOX(box),image,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(box),button,FALSE,FALSE,0);

    gamestate.gui_set_elements(real_board,dice_label,user_label);
    gamestate.read_map(mapin);
    gamestate.set_player_type(type_of_players);

    c_signal_init();

    thread run_game(running);
    run_game.detach();
}

int main(int argc,char **argv){
    GtkApplication* app;
    int gui_return;

    app = gtk_application_new(APP_PACKAGE_NAME.c_str(),G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(init),NULL);
    gui_return = g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return gui_return;
}