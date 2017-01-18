//
//  hambidge_tree.h
//  GoldenSite
//
//  Created by Daniel Riley on 5/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef GoldenSite_hambidge_tree_h
#define GoldenSite_hambidge_tree_h

#include "ratios.h"

class HambidgeTreeNode;

class HambidgeTree
{
public:
  HambidgeTree();
  
  HambidgeTree(const Ratios* ratios, int ratioIndex, double scale=1.0, HambidgeTreeNode* root=0L);
  
  HambidgeTree(const HambidgeTree& rhs);
  
  HambidgeTree& operator=(const HambidgeTree& rhs);
  
  virtual ~HambidgeTree();
  
  const Ratios* ratios() const
  {
    return ratios_;
  }
  
  double getScale() const
  {
    return scale_;
  }
  
  int getRatioIndex() const 
  {
    return ratioIndex_;
  }
  
  HambidgeTreeNode* getRoot()
  {
    return root_;
  }
  
  double getRatio()
  {
    if (ratios_ != 0L && ratioIndex_ >= 0)
    {
      return (*ratios_)[ratioIndex_];
    }
    
    return 0.0;
  }
  
  HambidgeTree* clone()
  {
    return new HambidgeTree(*this);
  }
  
  static bool SortLeaves(HambidgeTreeNode* lhs, HambidgeTreeNode* rhs);
  static bool LeavesEqual(HambidgeTreeNode* lhs, HambidgeTreeNode* rhs);
  
  static bool equals(HambidgeTree* lhs, HambidgeTree* rhs)
  {
    if (lhs == rhs) return true;
    
    std::vector<HambidgeTreeNode *> lhs_leaves;
    std::vector<HambidgeTreeNode *> rhs_leaves;
    
    lhs->getLeaves(&lhs_leaves);
    rhs->getLeaves(&rhs_leaves);
    
    if (lhs_leaves.size() != rhs_leaves.size()) return false;
    
    std::sort(lhs_leaves.begin(), lhs_leaves.end(), SortLeaves);
    std::sort(rhs_leaves.begin(), rhs_leaves.end(), SortLeaves);
    return std::equal(lhs_leaves.begin(), lhs_leaves.end(), rhs_leaves.begin());
  }
  
  
  std::vector<HambidgeTreeNode*>* getLeaves(std::vector<HambidgeTreeNode*>* leaves = 0L);
  
  HambidgeTreeNode* getLeafAt(int index);
  
  std::vector<double>* getAreas(std::vector<double>* areas = 0L);
  std::vector<double>* getDimensions(std::vector<double>* dimensions = 0L);


private:
  
  
  const Ratios* ratios_;
  int ratioIndex_;
  double scale_;
  HambidgeTreeNode* root_;
};

#endif
