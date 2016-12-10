class TreeNode{
  
  friend class BoardTree;
  
  public:
   // constructor
    TreeNode(TreeNode *p):parent(p){ 
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
   int win, loss, total;
   int branch_size;
   float branch_ucb[64];
   float branch_acc[64];
   int have_branch;
};