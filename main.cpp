#include<bits/stdc++.h>
//#define DEBUG
using namespace std;
/* BEGIN CONSTANT */
const int player_delta = 1000;
/* END CONSTANT */
/* BEGIN ADT */
string junk;
void Error(string msg){
    cerr<<"[ERROR] "<<msg<<endl;
    exit(0);
}
void Print(string msg){
    cout<<msg<<endl;
}
void Message(string msg){
    cout<<"[MESSAGE] "<<msg<<endl;
}
int intPrompt(string msg, int min=INT_MIN,int max=INT_MAX){
    cout<<msg;
    fflush(stdout);
    int ret;
    do{
        cin>>ret;
    }while(ret<min||ret>=max);
    return ret;
}
double doublePrompt(string msg, double min=-INFINITY,double max=INFINITY){
    cout<<msg;
    fflush(stdout);
    double ret;
    do{
        cin>>ret;
    }while(ret<min||ret>=max);
    return ret;
}
int dice(){
    # ifdef DEBUG
    return intPrompt("Dice value: ",1,7);
    # endif
    int x=rand()%6+1;
    Print("Dice value: "+string(1,x+'0'));
    return x;
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
    int color=-1;
    vector<Place*>exit;
    vector<Place*>entrance;
    set<Chess*>chesses;
    Place(int _id);
    void set_exit(vector<Place*>_exit);
    Place* get_exit(Chess* chess);
    void set_entrance(vector<Place*>_entrance);
    Place* get_entrance(Chess* chess);
    
    void add_chess(Chess* chess);
    void remove_chess(Chess* chess);
    
};
class Chess{
public:
    Player* player;
    Place* place;
    Chess(Player* _player,Place* _place);
    Place* get_dest(int step);
    void move(int step);
    bool conflict(int step);
    void move_to(Place* place);
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
    Player(int id,string _color, vector<Place*> start_point, int type, int number_of_chess);
    void take_back(Chess* chess);
    void action();
};
class GameState{
public:
    vector<Player*>players;
    vector<Place*>places;
    vector<int>type_of_players;
    int number_of_players;
    int number_of_places;
    Place* exit;
    GameState(){};
    GameState(int n,vector<int>type_of_players);
    void read_map(istream& in);
    int main_loop();
}gamestate;

/* END CLASS DEF */

/* BEGIN CHESS */
Chess::Chess(Player* _player,Place* _place){
    this->place = _place;
    this->player = _player;
    this->place->add_chess(this);
}

Place* Chess::get_dest(int step){
    Place* cur = this->place;
    int i=0;
    for(;i<step;++i){
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
    return cur;
}
void Chess::move(int step){
    Place* dest=this->get_dest(step);
    this->move_to(dest);
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
    this->place->chesses.erase(this);
    place->chesses.insert(this);
    this->place=place;
}
/* END CHESS */

/* BEGIN PLACE */
Place::Place(int _id){
    this->id=_id;
}
void Place::set_exit(vector<Place*>_exit){
    this->exit=_exit;
}

Place* Place::get_exit(Chess* chess){
    return this->exit[chess->player->id-player_delta];
}
void Place::set_entrance(vector<Place*>_entrance){
    this->entrance=_entrance;
}
Place* Place::get_entrance(Chess* chess){
    return this->entrance[chess->player->id-player_delta];
}
void Place::add_chess(Chess* chess){
    this->chesses.insert(chess);
}
void Place::remove_chess(Chess* chess){
    this->chesses.erase(chess);
}
/* END PLACE */

/* BEGIN PLAYER */
Player::Player(){};
Player::Player(int id,string _color, vector<Place*>start_point, int type, int number_of_chess = 4){
    this->type=type;
    this->id=id;
    this->airport=new Place(this->id);
    this->airport->set_exit(start_point);
    this->number_of_chess=number_of_chess;
    this->color=color;
    this->chesses.resize(number_of_chess);
    for(int i=0;i<number_of_chess;++i){
        chesses[i]=new Chess(this,airport);
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
            int dice_res=dice();
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
            int dice_res1=dice();
            if(dice_res1==6){
                first_in_airport_chess->move(1);
                int dice_res2=dice();
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
            int dice_res=dice();
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
            int dice_res1=dice();
            if(dice_res1==6){
                first_in_airport_chess->move(1);
                int dice_res2=dice();
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
    this->players.resize(n);
    this->places.resize(0);
    this->exit=new Place(-1);
}
void GameState::read_map(istream& in){
    int tmp;
    in >> tmp;
    if(tmp!=this->number_of_players){
        Error("The map is incapable for this number of player(s).");
    }
    in >> this->number_of_places;
    if(number_of_places>player_delta-3){
        Error("The number_of_places is too large!");
    }
    this->places.resize(number_of_places+1);
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
    vector<vector<Place*>>entertmp;
    entertmp.resize(this->number_of_places);
    for(int i=0;i<this->number_of_places;++i){
        entertmp[i].resize(number_of_players);
    }
    for(int i=0;i<this->number_of_places;++i){
        while(in.peek()=='#')getline(in,junk);
        int id;
        in >> id;
        vector<Place*>exits(this->number_of_players);
        for(int j=0;j<this->number_of_players;++j){
            int pt;
            in>>pt;
            if(pt<0||pt>this->number_of_places)Error("Invalid map file! #1");
            if(pt==exit)exits[j]=this->exit;
            else{
                exits[j]=this->places[pt];
                entertmp[pt][j]=places[i];
            }
        }
        this->places[i]->set_exit(exits);
    }
    for(int i=0;i<this->number_of_places;++i){
        this->places[i]->set_entrance(entertmp[i]);
    }
    
    for(int i=0;i<this->number_of_players;++i){
        vector<Place*>tmpv(this->number_of_players);
        for(int j=0;j<this->number_of_players;++j){
            tmpv[j]=this->places[entrances[i]];
        }
        players[i]=new Player(i+player_delta,colors[i],tmpv,type_of_players[i]);
    }
}
int GameState::main_loop(){
    for(int round=0;;++round){
        Print("\n\nRound "+itos(round));
        for(int i=0;i<this->number_of_players;++i){
            Print("Player "+itos(i)+":");
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
    return -1;
}
/* END GAMESTATE */


int main(){
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
    gamestate.read_map(mapin);
    int result = gamestate.main_loop();
    cerr<<"\n\nWinner: "<<result<<endl;
    return 0;
}
