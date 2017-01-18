//
//  hambidge_tree.cpp
//  GoldenSite
//
//  Created by Daniel Riley on 5/29/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include "hambidge_tree.h"
#include "hambidge_tree_node.h"
#include "hambidge_tree_iterator.h"

HambidgeTree::HambidgeTree()
:ratios_(0L),
 ratioIndex_(-1),
 scale_(1.0)
{
  root_ = new HambidgeTreeNode();
  root_->setRoot(this);
}

HambidgeTree::HambidgeTree(const Ratios* ratios, int ratioIndex, double scale, HambidgeTreeNode* root)
:ratios_(ratios),
 ratioIndex_(ratioIndex),
 scale_(scale)
{
  if (root == 0L)
  {
    root_ = new HambidgeTreeNode();
    root_->setRoot(this);
  }
  else
  {
    root_ = root->clone(this);
  }
}

HambidgeTree::HambidgeTree(const HambidgeTree& rhs)
:ratios_(rhs.ratios_),
 ratioIndex_(rhs.ratioIndex_),
 scale_(rhs.scale_)
{
  if (rhs.root_ != 0L)
  {
    root_ = rhs.root_->clone(this);
  }
  else
  {
    root_ = new HambidgeTreeNode();
    root_->setRoot(this);
  }
}

HambidgeTree& HambidgeTree::operator=(const HambidgeTree& rhs)
{
  if (this != &rhs)
  {
    delete root_;
    root_ = rhs.root_->clone(this);
    ratios_ = rhs.ratios_;
    ratioIndex_ = rhs.ratioIndex_;
    scale_ = rhs.scale_;
  }
  
  return *this;
}

HambidgeTree::~HambidgeTree()
{
  delete root_;
}

bool HambidgeTree::SortLeaves(HambidgeTreeNode* lhs, HambidgeTreeNode* rhs) {
  double lhs_y = lhs->getY();
  double rhs_y = rhs->getY();
  if (lhs_y < rhs_y) {
    return true;
  } else if (lhs_y > rhs_y) {
    return false;
  } else {
    double lhs_x = lhs->getX();
    double rhs_x = rhs->getX();
    if (lhs_x < rhs_x) {
      return true;
    } else if (lhs_x > rhs_x) {
      return false;
    }
  }
  
  return true;
}

bool HambidgeTree::LeavesEqual(HambidgeTreeNode* lhs, HambidgeTreeNode* rhs) {
  const double epsilon = 0.0001;
  return abs(lhs->getX() - rhs->getX()) < epsilon &&
        abs(lhs->getY() - rhs->getY()) < epsilon &&
        abs(lhs->getWidth() - rhs->getWidth()) < epsilon &&
  abs(lhs->getHeight() - rhs->getHeight()) < epsilon;
}

std::vector<HambidgeTreeNode*>* HambidgeTree::getLeaves(std::vector<HambidgeTreeNode*>* leaves)
{
  if (leaves == 0L) leaves = new std::vector<HambidgeTreeNode*>();
  root_->getLeaves(leaves);
  return leaves;
  /*
   Above is probably faster, but should be equal to this.
  HambidgeTreeNode* leaf;
  HambidgeTreeIterator iterator;
  iterator.set_tree(this);
  
  while (iterator.HasNext()) {
    if ((leaf = iterator.Next())->isLeaf()) {
      leaves->push_back(leaf);
    }
  }
  
  return leaves;
   */
}

HambidgeTreeNode* HambidgeTree::getLeafAt(int index)
{
#ifdef USE_ITERATOR_GET_LEAF_AT
  int i = 0;
  HambidgeTreeNode* leaf;
  HambidgeTreeIterator iterator;
  iterator.set_tree(this);
  
  while (iterator.HasNext()) {
    if ((leaf = iterator.Next())->isLeaf()) {
      if (i++ == index) {
        return leaf;
      }
    }
  }
  
  return 0L;

#else
  std::vector<HambidgeTreeNode *> leaves;
  getLeaves(&leaves);
  return leaves[index];
#endif
}

std::vector<double>* HambidgeTree::getAreas(std::vector<double>* areas)
{
  if (areas == 0L) areas = new std::vector<double>();
  root_->getAreas(areas);
  return areas;
}


std::vector<double>* HambidgeTree::getDimensions(std::vector<double>* dimensions)
{
  if (dimensions == 0L) dimensions = new std::vector<double>();
  root_->getDimensions(dimensions);
  return dimensions;
}