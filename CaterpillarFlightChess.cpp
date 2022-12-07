#include<bits/stdc++.h>
#include<pthread.h>
#include<gtk/gtk.h>
//#define DEBUG
using namespace std;
/* BEGIN CONSTANT */
const int player_delta = 1000;
/* END CONSTANT */
/* BEGIN ADT */
string junk;
static void Error(string msg){
    cerr<<"[ERROR] "<<msg<<endl;
    exit(0);
}
static void Print(string msg){
    cout<<msg<<endl;
}
static void Message(string msg){
    cout<<"[MESSAGE] "<<msg<<endl;
}
static void uniusleep(int ms){
    usleep(ms);
}
static void unisleep(int s){
    sleep(s);
}
static int intPrompt(string msg, int min=INT_MIN,int max=INT_MAX){
    cout<<msg;
    fflush(stdout);
    int ret;
    do{
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
    int color;
    int x,y;
    Place *entrance;
    vector<Place*>exit;
    vector<Place*>special_exit;
    set<Chess*>chesses;
    Place(int _id);
    void set_exit(vector<Place*>exit);
    void set_special_exit(vector<Place*>special_exit);
    void set_entrance(Place* entrance);
    Place* get_exit(Chess* chess);
    Place* get_special_exit(Chess* chess);
    Place* get_entrance(Chess* chess);
    void add_chess(Chess* chess);
    void remove_chess(Chess* chess);
    void set_coordinate(int x,int y);
    void set_id(int id);
};
class Chess{
public:
    Player* player;
    Place* place;
    string icon_filename;
    GtkWidget* widget;
    Chess(Player* _player,string icon_filename);
    Place* get_dest(int step);
    void move(int step);
    bool conflict(int step);
    void move_to(Place* place);
    void init_place(Place* place);
    void follow_dest(int step);
};
class Player{
public:
    int id;
    int number_of_chess;
    int type;
    string color;
    vector<Chess*>chesses;
    Place* airport;
    // Player();
    Player(int id,string color, Place* start_point, int type, int airportx, int airporty, int number_of_chess);
    void take_back(Chess* chess);
    void action();
};
class GameState{
public:
    vector<Player*>players;
    vector<Place*>places;
    vector<int>type_of_players;
    GtkWidget* board;
    int number_of_players;
    int number_of_places;
    Place* exit;
    GtkWidget* dice_label;
    GtkWidget* user_label;
    GameState(){};
    GameState(int n,vector<int>type_of_players);
    void read_map(istream& in,GtkWidget*board);
    int main_loop();
    int dice();
}gamestate;

/* END CLASS DEF */

/* BEGIN CHESS */
Chess::Chess(Player* _player,string icon_filename){
    this->player = _player;
    this->icon_filename=icon_filename;
    this->widget = gtk_image_new_from_file(icon_filename.c_str());
    gtk_widget_show(this->widget);
}

Place* Chess::get_dest(int step){
    Place* cur = this->place;
    int i=0;
    for(;i<step-1;++i){
        if(cur->get_exit(this)==gamestate.exit){
            if(step==i+1){
                ++i;
                cur=cur->get_exit(this);
                break;
            }else{
                break;
            }
        }
        cur=cur->get_exit(this);
    }
    for(;i<step;++i){
        cur=cur->get_entrance(this);
    }
    cur=cur->get_special_exit(this);
    return cur;
}
void Chess::follow_dest(int step){
    Place* cur = this->place;
    int i=0;
    for(;i<step-1;++i){
        int sx=cur->x,ex,sy=cur->y,ey;
        if(cur->get_exit(this)==gamestate.exit){
            if(step==i+1){
                ++i;
                cur=cur->get_exit(this);
                break;
            }else{
                break;
            }
            g_object_ref(this->widget);
            gtk_container_remove(GTK_CONTAINER(gamestate.board),this->widget);
        }
        cur=cur->get_exit(this);
        ex=cur->x;ey=cur->y;
        for(int cx=sx;cx<=ex;++cx){
            gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,cx,sy);
        }
        for(int cy=sy;cy<=ey;++cy){
            gtk_fixed_move(GTK_FIXED(gamestate.board),this->widget,ex,cy);
        }
    }
    for(;i<step;++i){
        cur=cur->get_entrance(this);
    }
    cur=cur->get_special_exit(this);
}
void Chess::move(int step){
    Place* dest=this->get_dest(step);
    this->move_to(dest);
    this->follow_dest(step);
}
bool Chess::conflict(int step){
    Place* dest=this->get_dest(step);
    set<Chess*>&chesses=place->chesses;
    for(set<Chess*>::iterator it=chesses.begin();it!=chesses.end();++it){
        if((*it)->player!=this->player){
            return true;
        }
    }
    return false;
}
void Chess::move_to(Place* place){
    set<Chess*>chesses=place->chesses;
    for(set<Chess*>::iterator it=chesses.begin();it!=chesses.end();++it){
        if((*it)->player!=this->player){
            (*it)->player->take_back((*it));
        }
    }
    this->place->remove_chess(this);
    place->add_chess(this);
    this->place=place;
}
void Chess::init_place(Place* place){
    this->place = place;
    place->add_chess(this);
    gtk_fixed_put(GTK_FIXED(gamestate.board),this->widget,this->place->x,this->place->y);
}
/* END CHESS */

/* BEGIN PLACE */
Place::Place(int _id){
    this->id=_id;
}
void Place::set_exit(vector<Place*>exit){
    this->exit=exit;
}
Place* Place::get_exit(Chess* chess){
    return this->exit[chess->player->id-player_delta];
}
Place* Place::get_special_exit(Chess* chess){
    return this->special_exit[chess->player->id-player_delta];
}
void Place::set_entrance(Place* entrance){
    this->entrance=entrance;
}
void Place::set_special_exit(vector<Place*>special_exit){
    this->special_exit=special_exit;
}
Place* Place::get_entrance(Chess* chess){
    return this->entrance;
}
void Place::add_chess(Chess* chess){
    this->chesses.insert(chess);
}
void Place::remove_chess(Chess* chess){
    this->chesses.erase(chess);
}
void Place::set_coordinate(int x,int y){
    this->x=x;
    this->y=y;
}
void Place::set_id(int id){
    this->id=id;
}
/* END PLACE */

/* BEGIN PLAYER */
Player::Player(int id,string color, Place* start_point, int type, int airportx, int airporty, int number_of_chess = 4){
    this->type=type;
    this->id=id;
    this->airport=new Place(this->id);
    this->airport->set_exit(vector<Place*>(gamestate.number_of_players,start_point));
    this->airport->set_special_exit(vector<Place*>(gamestate.number_of_players,start_point));
    this->airport->set_coordinate(airportx,airporty);
    this->number_of_chess=number_of_chess;
    this->color=color;
    this->chesses.resize(number_of_chess);
    for(int i=0;i<number_of_chess;++i){
        this->chesses[i]=new Chess(this,this->color);
        this->chesses[i]->init_place(this->airport);
    }
}
void Player::take_back(Chess* chess){
    chess->move_to(this->airport);
}
void Player::action(){
    if(this->type){
        bool has_chess_in_airport=false;
        Chess* first_in_airport_chess=NULL;
        for(int i=0;i<this->chesses.size();++i){
            if(this->chesses[i]->place==this->airport){
                has_chess_in_airport=true;
                first_in_airport_chess=this->chesses[i];
                break;
            }
        }
        if(!has_chess_in_airport){
            Print("User "+itos(this->id)+" has no planes in his/her airport,\nrolling dice for him/her.");
            int dice_res=gamestate.dice();
            vector<Chess*>candidate;
            for(int i=0;i<this->chesses.size();++i){
                if(this->chesses[i]->place==gamestate.exit)continue;
                candidate.push_back(this->chesses[i]);
            }
            Print("Candidates:");
            for(int i=0;i<candidate.size();++i){
                Print("Chess "+itos(i)+": "+itos(candidate[i]->place->id));
            }
            int bestp=intPrompt("Choose a candidate to move: ",0,candidate.size());
            candidate[bestp]->move(dice_res);
        }else{
            int dice_res1=gamestate.dice();
            if(dice_res1==6){
                first_in_airport_chess->move(1);
                int dice_res2=gamestate.dice();
                first_in_airport_chess->move(dice_res2);
            }
        }
    }else{
        bool has_chess_in_airport=false;
        Chess* first_in_airport_chess=NULL;
        for(int i=0;i<this->chesses.size();++i){
            if(this->chesses[i]->place==this->airport){
                has_chess_in_airport=true;
                first_in_airport_chess=this->chesses[i];
                break;
            }
        }
        if(!has_chess_in_airport){
            int dice_res=gamestate.dice();
            vector<Chess*>candidate;
            for(int i=0;i<this->chesses.size();++i){
                if(this->chesses[i]->place==this->airport||this->chesses[i]->place==gamestate.exit)continue;
                candidate.push_back(this->chesses[i]);
            }
            int bestn=0,bestp=0;
            if(candidate.size()==0)Error("Impossible situation!");
            for(int i=0;i<candidate.size();++i){
                int value=(candidate[i]->get_dest(dice_res)->color-player_delta==this->id)?5:1;
                if(value>bestn){
                    bestn=value;
                    bestp=i;
                }
            }
            candidate[bestp]->move(dice_res);
        }else{
            int dice_res1=gamestate.dice();
            if(dice_res1==6){
                first_in_airport_chess->move(1);
                int dice_res2=gamestate.dice();
                first_in_airport_chess->move(dice_res2);
            }
        }
    }
}
/* END PLAYER */

/* BEGIN GAMESTATE */
GameState::GameState(int n,vector<int>type_of_players){
    this->type_of_players=type_of_players;
    this->number_of_players = n;
    this->exit=NULL;
}
/*
void GameState::read_map(istream& in,GtkWidget* board){
    int tmp;
    in >> tmp;
    if(tmp!=this->number_of_players){
        Error("The map is incapable for this number of player(s).");
    }
    in >> this->number_of_places;
    if(this->number_of_places>player_delta-3){
        Error("The number_of_places is too large!");
    }
    this->places.resize(this->number_of_places+1);
    for(int i=0;i<this->number_of_places;++i){
        this->places[i]=new Place(i);
    }
    vector<int>entrances(this->number_of_players);
    vector<string>colors(this->number_of_players);
    int exit;
    for(int i=0;i<this->number_of_players;++i){
        in>>entrances[i];
        if(entrances[i]<0||entrances[i]>=this->number_of_places)Error("Invalid map file! #2");
    }
    in>>exit;
    this->exit=new Place(exit);
    for(int i=0;i<this->number_of_players;++i){
        in>>colors[i];
    }
    // vector<vector<Place*>>entertmp;
    // entertmp.resize(this->number_of_places);
    // for(int i=0;i<this->number_of_places;++i){
    //     entertmp[i].resize(number_of_players);
    // }
    // for(int i=0;i<this->number_of_places;++i){
    //     int id;
    //     in >> id;
    //     vector<Place*>exits(this->number_of_players+1);
    //     int pt;
    //     for(int j=0;j<this->number_of_players+1;++j){
            
    //         in>>pt;
    //         if(pt<0||pt>this->number_of_places)Error("Invalid map file! #1");
    //         if(pt==exit)exits[j]=this->exit;
    //         else{
    //             exits[j]=this->places[pt];
                
    //         }
    //     }
    //     for(int j=0;j<number_of_players;++j){
    //         entertmp[pt][j]=places[i];
    //     }
        
    //     this->places[i]->set_exit(exits);
    // }
    // for(int i=0;i<this->number_of_places;++i){
    //     this->places[i]->set_entrance(entertmp[i]);
    // }

    vector<pair<int,pair<vector<int>,int>>>table;
    for(int i=0;i<number_of_places;++i){
        in>>table[i].first;
        table[i].second.first.resize(number_of_players);
        for(int j=0;j<number_of_players;++j){
            in>>table[i].second.first[j];
        }
        in>>table[i].second.second;
    }

    for(int i=0;i<number_of_places;++i){
        this->places[i]->set_id(table[i].first);
        vector<Place*>exits(number_of_players+1);
        for(int j=0;j<number_of_players;++j){
            int exit_id=table[i].second.first[j];
            for(int k=0;k<number_of_places;++k){
                if(table[i].first==exit_id){
                    exits[j]=this->places[k];
                }
            }
        }
        int default_exit=table[i].second.second;
        for(int k=0;k<number_of_places;++k){
            if(table[i].first==default_exit){
                exits[number_of_players]=this->places[k];
            }
        }
        this->places[i]->set_exit(exits);
    }
    for(int i=0;i<number_of_places;++i){
        int x=table[i].first,y=table[i].second.second;
        for(int j=0;j<number_of_places;++j){
            if(table[j].first==y){
                this->places[j]->set_entrance(vector<Place*>(number_of_players,this->places[i]));
            }
        }
    }


    string board_filename;
    in >> board_filename;
    GtkWidget* board_background;
    GtkWidget* real_board = gtk_fixed_new();
    board_background = gtk_image_new_from_file(board_filename.c_str());
    gtk_widget_set_halign(board_background,GTK_ALIGN_START);
    gtk_widget_set_valign(board_background,GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(board),board_background);
    gtk_widget_set_halign(real_board,GTK_ALIGN_START);
    gtk_widget_set_valign(real_board,GTK_ALIGN_START);
    gtk_overlay_add_overlay(GTK_OVERLAY(board),real_board);
    gtk_overlay_reorder_overlay(GTK_OVERLAY(board),real_board,10000);
    gtk_widget_show_all(board);

    this->board = real_board;

    this->players.resize(0);
    for(int i=0;i<this->number_of_players;++i){
        vector<Place*>tmpv(this->number_of_players);
        for(int j=0;j<this->number_of_players;++j){
            tmpv[j]=this->places[entrances[i]];
        }
        this->players.push_back(new Player(i+player_delta,colors[i],tmpv,type_of_players[i]));
    }
    
    
    int number_of_places_onmap;
    in >> number_of_places_onmap;
    for(int i=0;i<number_of_places_onmap;++i){
        int id, x, y;
        in>>id>>x>>y;
        this->places[i]->set_coordinate(x,y);
    }
}
*/
void GameState::read_map(istream& in,GtkWidget* board){
    int tmp;
    in >> tmp;
    if(tmp!=this->number_of_players){
        Error("The map is incapable for this number of player(s).");
    }
    in >> this->number_of_places;
    if(this->number_of_places>player_delta-3){
        Error("The number_of_places is too large!");
    }
    this->places.resize(this->number_of_places);
    for(int i=0;i<this->number_of_places;++i){
        this->places[i]=new Place(i);
    }
    vector<Place*>start_ponits(this->number_of_players);
    vector<string>colors(this->number_of_players);
    int exit;
    for(int i=0;i<this->number_of_players;++i){
        in>>tmp;
        if(tmp<0||tmp>=this->number_of_places)Error("Invalid map file! #1");
        start_ponits[i]=this->places[tmp];
    }
    in>>exit;
    this->exit=new Place(exit);
    for(int i=0;i<this->number_of_players;++i){
        in>>colors[i];
    }

    vector<pair<int,int>>airport_coordinates(number_of_players);
    for(int i=0;i<number_of_players;++i){
        in>>airport_coordinates[i].first>>airport_coordinates[i].second;
    }

    vector<vector<Place*>>default_exit,special_exit;
    default_exit.resize(number_of_places);
    special_exit.resize(number_of_places);
    for(int i=0;i<number_of_places;++i){
        int id, enterance;
        default_exit[i].resize(number_of_players);
        special_exit[i].resize(number_of_players);
        in>>id>>enterance;
        for(int j=0;j<number_of_players;++j){
            in>>tmp;
            if(tmp>=0&&tmp<this->number_of_places){
                default_exit[i][j]=this->places[tmp];
            }else if(tmp==this->number_of_players){
                default_exit[i][j]=this->exit;
            }
        }
        for(int j=0;j<number_of_players;++j){
            in>>tmp;
            if(tmp>=0&&tmp<this->number_of_places){
                special_exit[i][j]=this->places[tmp];
            }else if(tmp==this->number_of_players){
                special_exit[i][j]=this->exit;
            }
        }
        this->places[i]->set_id(id);
        this->places[i]->set_entrance(this->places[enterance]);
        this->places[i]->set_exit(default_exit[i]);
        this->places[i]->set_special_exit(special_exit[i]);
    }


    string board_filename;
    in >> board_filename;
    GtkWidget* board_background;
    GtkWidget* real_board = gtk_fixed_new();
    board_background = gtk_image_new_from_file(board_filename.c_str());
    gtk_widget_set_halign(board_background,GTK_ALIGN_START);
    gtk_widget_set_valign(board_background,GTK_ALIGN_START);
    gtk_container_add(GTK_CONTAINER(board),board_background);
    gtk_widget_set_halign(real_board,GTK_ALIGN_START);
    gtk_widget_set_valign(real_board,GTK_ALIGN_START);
    gtk_overlay_add_overlay(GTK_OVERLAY(board),real_board);
    gtk_overlay_reorder_overlay(GTK_OVERLAY(board),real_board,10000);
    gtk_widget_show_all(board);

    this->board = real_board;

    for(int i=0;i<this->number_of_players;++i){
        this->players.push_back(new Player(i+player_delta,colors[i],start_ponits[i],type_of_players[i],airport_coordinates[i].first,airport_coordinates[i].second));
    }
    
    
    int number_of_places_onmap;
    in >> number_of_places_onmap;
    for(int i=0;i<number_of_places_onmap;++i){
        int id, x, y;
        in>>id>>x>>y;
        this->places[i]->set_coordinate(x,y);
    }
}
int GameState::main_loop(){
    for(int round=0;;++round){
        Print("\n\nRound "+itos(round));
        for(int i=0;i<this->number_of_players;++i){
            Print("Player "+itos(i)+":");
            gtk_label_set_label(GTK_LABEL(this->user_label),("Player: "+itos(i+1)+"/"+itos(number_of_players)).c_str());
            players[i]->action();
            vector<Chess*>&chesses=players[i]->chesses;
            for(int j=0;j<players[i]->number_of_chess;++j){
                cerr<<"Player "<<i<<": Chess "<<j<<": "<<chesses[j]->place->id<<endl;
            }
            bool flag=true;
            for(int j=0;j<players[i]->number_of_chess;++j){
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
    cerr<<"Main loop returns\n";
    return -1;
}
int GameState::dice(){
    # ifdef DEBUG
    return intPrompt("Dice value: ",1,7);
    # endif
    uniusleep(500000);
    int x=rand()%6+1;
    Print("Dice value: "+string(1,x+'0'));
    gtk_label_set_label(GTK_LABEL(this->dice_label),("Dice: "+itos(x)).c_str());
    return x;
}
/* END GAMESTATE */
static void CLI(GtkWidget* board,GtkWidget* user_label,GtkWidget* dice_label)
{
    srand(time(NULL));
    Print("Caterpillar Flight Chess V1.0.0\n");
    ifstream conf("ctpchess.conf"),mapin;
    if(!conf){
        Error("No config file!");
    }
    int number_of_players;
    string map_path;
    conf>>number_of_players;
    vector<int>type_of_players(number_of_players);
    for(int i=0;i<number_of_players;++i){
        if(!conf)Error("Invalid users definition!");
        conf>>(type_of_players[i]);
    }
    conf>>map_path;
    if(number_of_players<2||number_of_players>100){
        Error("Invalid number_of_players!");
    }
    mapin.open(map_path.c_str());
    if(!mapin){
        Error("Invalid map filename!");
    }
    gamestate=GameState(number_of_players,type_of_players);
    gamestate.read_map(mapin,board);
    gamestate.dice_label = dice_label;
    gamestate.user_label = user_label;
    int result = gamestate.main_loop();
    GtkWidget* win_prompt;
    win_prompt = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(win_prompt),"YOU WIN!");
    gtk_container_set_border_width(GTK_CONTAINER(win_prompt),16);
    GtkWidget* label;
    label = gtk_label_new(("Player "+itos(result+1)+" WINS!").c_str());
    gtk_container_add(GTK_CONTAINER(win_prompt),label);
    gtk_widget_show_all(win_prompt);
    
    cerr<<"\n\nWinner: "<<result<<endl;
}
static void
ctp_main(GtkApplication* app,gpointer app_data){
    GtkWidget* main_window;
    GtkWidget* main_box;
    GtkWidget *board;
    GtkWidget* panel;

    GtkWidget* separator;
    main_window = gtk_application_window_new(app);

    gtk_window_set_title(GTK_WINDOW(main_window),"Flight Chess");
    gtk_window_set_default_size(GTK_WINDOW(main_window),1080,840);
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    

    board = gtk_overlay_new();

    panel = gtk_list_box_new();
    
    
    separator = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

    GtkWidget* panel_box;
    panel_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
    // gtk_box_set_homogeneous(GTK_BOX(panel_box),TRUE);

    GtkWidget *panel_user_box,*panel_dice_box;
    panel_user_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);
    panel_dice_box = gtk_box_new(GTK_ORIENTATION_VERTICAL,0);

    GtkWidget *user_image, *user_label, *dice_image, *dice_label;
    user_image = gtk_image_new_from_file("user.png");
    dice_image = gtk_image_new_from_file("dice.png");
    user_label = gtk_label_new("Player: 0/0");
    dice_label = gtk_label_new("Dice: NULL");

    gtk_box_pack_end(GTK_BOX(panel_user_box),user_label,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(panel_user_box),user_image,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(panel_dice_box),dice_label,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(panel_dice_box),dice_image,FALSE,FALSE,FALSE);
    
    gtk_box_pack_start(GTK_BOX(panel_box),panel_user_box,FALSE,FALSE,FALSE);
    gtk_box_pack_start(GTK_BOX(panel_box),panel_dice_box,FALSE,FALSE,FALSE);

    gtk_list_box_prepend(GTK_LIST_BOX(panel),panel_box);

    GtkWidget *log_info, *list_of_state;
    GtkTextBuffer *log_info_buffer;
    log_info = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(log_info), FALSE);
    log_info_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(log_info));
    
    gtk_box_pack_start(GTK_BOX(main_box),board,TRUE,TRUE,FALSE);
    gtk_box_pack_start(GTK_BOX(main_box),separator,FALSE,FALSE,FALSE);
    gtk_box_pack_end(GTK_BOX(main_box),panel,FALSE,FALSE,FALSE);

    gtk_container_add(GTK_CONTAINER(main_window),main_box);
    gtk_widget_show_all(main_window);
    thread t_CLI(CLI,board,user_label,dice_label);
    t_CLI.detach();
}
int main(int argc,char *argv[]){
    GtkApplication* app;
    int return_status;
    app = gtk_application_new("org.caterpillar.flightchess",G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app,"activate",G_CALLBACK(ctp_main),NULL);
    return_status = g_application_run(G_APPLICATION(app),argc,argv);
    g_object_unref(app);
    return return_status;
}
