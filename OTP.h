#include"board.h"
#include"Tree.h"
#include<random>
#ifdef _WIN32
#include<chrono>
#endif
#include<cstring>
#include<string>
#include<math.h>

constexpr char m_tolower(char c){
    return c+('A'<=c&&c<='Z')*('a'-'A');
}
constexpr unsigned my_hash(const char*s,unsigned long long int hv=0){
    return *s&&*s!=' '?my_hash(s+1,(hv*('a'+1)+m_tolower(*s))%0X3FFFFFFFU):hv;
}
struct history{
    int x,y,pass,tiles_to_flip[27],*ed;
};
template<class RIT>RIT random_choice(RIT st,RIT ed){
#ifdef _WIN32
    //std::random_device is deterministic with MinGW gcc 4.9.2 on Windows
    static std::mt19937 local_rand(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
#else
    static std::mt19937 local_rand(std::random_device{}());
#endif
    return st+std::uniform_int_distribution<int>(0,ed-st-1)(local_rand);
}
class OTP{
    board B;
    history H[128],*HED;
    //initialize in do_init
    void do_init(){
        B = board();
        HED = H;
    }
    //choose the best move in do_genmove
    int do_genmove(){
        
        int ML[64],*MLED(B.get_valid_move(ML));
        int node_ucb_list[64];
        int best_move;
        int depth = 10;
        int re_simu_size = 10;
        int pre_win=0, re_simu_win=0;
        int pre_loss=0, re_simu_loss=0;
        int idx=0;
        int min_max=0;
        // If there is no legel move, return directly
        if (MLED==ML){
        	do_play(8,0);
        	return 64;
        }
        else{
            Tree tree;
            tree.set_root(B.a, B.get_my_tile());

            for (int i=0; i<depth; i++){
                if (tree.get_have_branch()==0){
                    pre_win = tree.get_win();
                    pre_loss = tree.get_loss();
                    initial_sampling(tree, MLED, ML);
                    for (int j=0; j<re_simu_size; j++){
                        re_simulation(tree);
                    }
                    re_simu_win = tree.get_win() - pre_win;
                    re_simu_loss = tree.get_loss() - pre_loss;
                    while(tree.nowPtr->parent != 0){
                        idx = tree.get_id();
                        tree.backtract();
                        tree.update(re_simu_win, re_simu_loss, re_simu_win+re_simu_loss);
                        tree.update_one_branch(idx);
                    }
                    min_max = 0;    
                }
                else{
                    // update all branch from this node
                    tree.update_branch_ucb_acc(tree.get_branch_size(), 100);
                    if(min_max%2==0)
                        tree.forward(tree.get_highest_ucb_index());
                    else
                        tree.forward(tree.get_lowest_ucb_index());
                    min_max++;
                }
            }
            // go back to root
            while(tree.nowPtr->parent != 0){
                tree.backtract();
            }
            best_move = ML[tree.get_highest_acc_index()];
            B.reset_board((tree.nowPtr->map), (tree.nowPtr)->my_tile);
        }

        
    	return best_move;
    }

    void initial_sampling(Tree tree, int *MLED, int* ML){
        int initial_sampling_size=1000;
        int pre_xy, xy, win, loss, pass, score;
        int legel_cnt=0;

        // reset map
        B.reset_board((tree.nowPtr)->map, (tree.nowPtr)->my_tile);
        
        while (*MLED != *(ML+legel_cnt))
                legel_cnt++;
        tree.set_branch_size(legel_cnt);
            
        for (int i=0; i<tree.get_branch_size(); i++){
            pre_xy = *(ML+i);
            win = 0;
            loss = 0;
            pass = 0;

            // number of simulation in a node
            for (int s=0; s<initial_sampling_size; s++){
                B.simulate_update(pre_xy/8,pre_xy%8);
                
                tree.generate_new_branch(B.a, i, (tree.nowPtr)->my_tile);

                while (pass!=2){
                    xy = random_move();
                    if (xy==64)
                        pass ++;
                    else
                        pass = 0;
                    B.simulate_update(xy/8,xy%8);
                }

                score = B.get_score();
                if (score>0)
                    win++;
                else
                    loss++;
                B.reset_board((tree.nowPtr)->map, (tree.nowPtr)->my_tile);
                pass = 0;
            }
            tree.update(win, loss, win+loss);
            (tree.nowPtr)->branch_ucb[i] = (double) win/(win+loss) + 1.18*sqrt(log(tree.get_branch_size()*initial_sampling_size)/initial_sampling_size);
            (tree.nowPtr)->branch_acc[i] = (double) win/(win+loss);
            tree.forward(i);
            tree.update(win, loss, win+loss);
            tree.backtract();
        }
    }

    void re_simulation(Tree tree){
        int idx;
        int best_ucb=0;
        int redo_size=200;
        int pass=0, win=0, loss=0, xy;
        int ucb, acc;

        // get the best node via ucb
        idx = tree.get_highest_ucb_index();
        
        tree.forward(idx);
        // reset map
        B.reset_board((tree.nowPtr)->map, (tree.nowPtr)->my_tile);

        // simulation
        for (int i=0; i<redo_size; i++){
            while (pass!=2){
                xy = random_move();
                if (xy==64){
                    pass += 1;
                }
                else{
                    pass = 0;
                }
                B.simulate_update(xy/8,xy%8);
            }

            if (B.get_score()>0)
                win++;
            else
                loss++;
            B.reset_board((tree.nowPtr)->map, (tree.nowPtr)->my_tile);
            pass = 0;
        }
        // update tree->board[i]
        tree.update(win, loss, win+loss);
        tree.backtract();

        // update tree->nowPtr
        tree.update(win, loss, win+loss);
        tree.update_branch_ucb_acc(tree.get_branch_size(), idx);
    }

    int random_move(){
    	int K[64],*KED(B.get_valid_move(K));
    	return KED==K?64:*random_choice(K,KED);
	}

    //update board and history in do_play
    void do_play(int x,int y){
        if(HED!=std::end(H)&&B.is_game_over()==0&&B.is_valid_move(x,y)){
            HED->x = x;
            HED->y = y;
            HED->pass = B.get_pass();
            HED->ed = B.update(x,y,HED->tiles_to_flip);
            ++HED;
        }else{
            fputs("wrong play.\n",stderr);
        }
    }
    //undo board and history in do_undo
    void do_undo(){
        if(HED!=H){
            --HED;
            B.undo(HED->x,HED->y,HED->pass,HED->tiles_to_flip,HED->ed);
        }else{
            fputs("wrong undo.\n",stderr);
        }
    }
public:
    OTP():B(),HED(H){
        do_init();
    }
    bool do_op(const char*cmd,char*out,FILE*myerr){
        switch(my_hash(cmd)){
            case my_hash("name"):
                sprintf(out,"name template7122");
                return true;
            case my_hash("clear_board"):
                do_init();
                B.show_board(myerr);
                sprintf(out,"clear_board");
                return true;
            case my_hash("showboard"):
                B.show_board(myerr);
                sprintf(out,"showboard");
                return true;
            case my_hash("play"):{
                int x,y;
                sscanf(cmd,"%*s %d %d",&x,&y);
                do_play(x,y);
                B.show_board(myerr);
                sprintf(out,"play");
                return true;
            }
            case my_hash("genmove"):{
                int xy = do_genmove();
                int x = xy/8, y = xy%8;
                do_play(x,y);
                B.show_board(myerr);
                sprintf(out,"genmove %d %d",x,y);
                return true;
            }
            case my_hash("undo"):
                do_undo();
                sprintf(out,"undo");
                return true;
            case my_hash("final_score"):
                sprintf(out,"final_score %d",B.get_score());
                return true;
            case my_hash("quit"):
                sprintf(out,"quit");
                return false;
            //commmands used in simple_http_UI.cpp
            case my_hash("playgen"):{
                int x,y;
                sscanf(cmd,"%*s %d %d",&x,&y);
                do_play(x,y);
                if(B.is_game_over()==0){
                    int xy = do_genmove();
                    x = xy/8, y = xy%8;
                    do_play(x,y);
                }
                B.show_board(myerr);
                sprintf(out,"playgen %d %d",x,y);
                return true;
            }
            case my_hash("undoundo"):{
                do_undo();
                do_undo();
                sprintf(out,"undoundo");
                return true;
            }
            case my_hash("code"):
                do_init();
                B = board(cmd+5,cmd+strlen(cmd));
                B.show_board(myerr);
                sprintf(out,"code");
                return true;
            default:
                sprintf(out,"unknown command");
                return true;
        }
    }
    std::string get_html(unsigned,unsigned)const;
};
