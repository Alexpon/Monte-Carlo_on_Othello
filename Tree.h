#include <iostream>
#include <new>
#include <math.h>
#include "TreeNode.h"
using namespace std;

class Tree{

public:
   Tree(); // constructor
   void set_root(unsigned char[][8], int);
   void set_branch_size(int);
   void update(int, int, int);
   void update_branch_ucb_acc(int, int);
   void generate_new_branch(unsigned char[][8], int);
   void forward(int);
   void backtract();
   int get_win();
   int get_loss();
   int get_total();
   int get_branch_size();
   int get_highest_ucb_index();
   int get_highest_acc_index();
   TreeNode *nowPtr;
};

// constructor
Tree::Tree(){
   nowPtr=0;
}

void Tree::set_root(unsigned char map[][8], int my_tile){
   nowPtr = new TreeNode(0);
   for (int i=0; i<8; i++){
      for (int j=0; j<8; j++){
         nowPtr->map[i][j] = map[i][j];
      }
   }
   nowPtr -> my_tile = my_tile;
}

void Tree::generate_new_branch(unsigned char map[][8], int index){
   (nowPtr->branch[index]) = new TreeNode(nowPtr);
   for (int i=0; i<8; i++){
      for (int j=0; j<8; j++){
         (nowPtr->branch[index])->map[i][j] = map[i][j];
      }
   }
   (*nowPtr).have_branch = 1;
}

void Tree::set_branch_size(int branch_size){
   if(nowPtr!=0)
      (*nowPtr).branch_size = branch_size;
}

void Tree::update(int win, int loss, int total){
   if(nowPtr!=0){
      (*nowPtr).win += win;
      (*nowPtr).loss += loss;
      (*nowPtr).total += total;
   }
}


void Tree::update_branch_ucb_acc(int size, int idx){
   TreeNode *tmpNode;
   for (int i=0; i<size; i++){
      tmpNode = nowPtr->branch[i];
      nowPtr->branch_ucb[i] = (double)tmpNode->win/tmpNode->total + 1.18*sqrt(log(nowPtr->total)/tmpNode->total);
   }
   tmpNode = nowPtr->branch[idx];
   nowPtr->branch_acc[idx] = (double) tmpNode->win/tmpNode->total;
}

void Tree::forward(int index){
   nowPtr = (*nowPtr).branch[index];
}

void Tree::backtract(){
   nowPtr = (*nowPtr).parent;
}

int Tree::get_win(){
   return (*nowPtr).win;
}

int Tree::get_loss(){
   return (*nowPtr).loss;
}

int Tree::get_total(){
   return (*nowPtr).total;
}

int Tree::get_branch_size(){
   return (*nowPtr).branch_size;
}

int Tree::get_highest_ucb_index(){
   int idx=0;
   double tmp=0;
   for(int i=0; i<(*nowPtr).branch_size; i++){
      if(tmp < (*nowPtr).branch_ucb[i]){
         tmp = (*nowPtr).branch_ucb[i];
         idx = i;
      }
   }
   return idx;
}

int Tree::get_highest_acc_index(){
   int idx=0;
   double tmp=0;
   for(int i=0; i<(*nowPtr).branch_size; i++){
      if(tmp < (*nowPtr).branch_acc[i]){
         tmp = (*nowPtr).branch_acc[i];
         idx = i;
      }
   }
   return idx;
}

