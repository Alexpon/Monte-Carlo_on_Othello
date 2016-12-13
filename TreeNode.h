class TreeNode{
  
  friend class BoardTree;
  
  public:
   // constructor
    TreeNode(TreeNode *p, const int &d):parent(p), id(d){ 
      win = 0;
      loss = 0;
      total = 0;
      branch_size = 0;
      have_branch = 0;
    }

//private:
   TreeNode *branch[64];
   TreeNode *parent;
   unsigned char map[8][8];
   int id;
   int win, loss, total;
   int branch_size;
   double branch_ucb[64];
   double branch_acc[64];
   int have_branch;
   int my_tile;
};
