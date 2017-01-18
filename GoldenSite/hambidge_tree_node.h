//
//  hambidge_tree_node.h
//  GoldenSite
//
//  Created by Daniel Riley on 5/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef GoldenSite_hambidge_tree_node_h
#define GoldenSite_hambidge_tree_node_h

#include "ratios.h"

class HambidgeTree;

class HambidgeTreeNode
{
public:
  HambidgeTreeNode()
  :root_(0L),
   parent_(0L),
   left_(0L),
   right_(0L),
   cachedWidth_(0.0),
   cachedHeight_(0.0),
   cachedArea_(0.0),
   cachedX_(-1.0),
   cachedY_(-1.0)
  {}
  
  
  virtual ~HambidgeTreeNode()
  {
    if (!isLeaf())
    {
      delete left_;
      delete right_;
    }
  }
  
  bool split(Split s);
  
  SplitType getSplitType() const;
  
  const Split* getSplit() const;
  
  int getRatioIndex() const;
  double getRatio() const;
  double getAspectRatio();
  const Ratios* getRatios() const;
  HambidgeTreeNode* clone(HambidgeTree* root=0L, HambidgeTreeNode* parent=0L);
  
  double getX();
  double getY();
  double getWidth();
  double getHeight();
  double getArea();
  
  void getLeaves(std::vector<HambidgeTreeNode*>* leaves);
  
  void getAreas(std::vector<double>* areas);
  void getDimensions(std::vector<double>* dimensions);
  
  bool isLeaf() const;
  
  HambidgeTreeNode* getLeft();
  
  HambidgeTreeNode* getRight();
  
  void serialize(std::streambuf* buf);
  
  void deserialize(std::streambuf* buf, HambidgeTree* root, HambidgeTreeNode* parent);
  void setRoot(HambidgeTree* root);
  
  void setParent(HambidgeTreeNode* parent);
  
  void setLeft(HambidgeTreeNode* left);
  
  void setRight(HambidgeTreeNode* right);

private:
  
  HambidgeTree* root_;
  Split split_;
  HambidgeTreeNode* parent_;
  HambidgeTreeNode* left_;
  HambidgeTreeNode* right_;
  double cachedArea_;
  double cachedWidth_;
  double cachedHeight_;
  double cachedX_;
  double cachedY_; 
};



#endif
