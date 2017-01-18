//
//  hambidge_tree_node.cpp
//  GoldenSite
//
//  Created by Daniel Riley on 5/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include "hambidge_tree.h"
#include "hambidge_tree_node.h"


bool HambidgeTreeNode::split(Split s)
{
  if (split_.type == SPLIT_NONE && 
      s.type != SPLIT_NONE)
  {
    split_ = s;
    left_ = new HambidgeTreeNode();
    left_->setRoot(root_);
    left_->setParent(this);
    
    right_ = new HambidgeTreeNode();
    right_->setRoot(root_);
    right_->setParent(this);
    
    return true;
  }
  
  return false;
}

SplitType HambidgeTreeNode::getSplitType() const
{
  return split_.type;
}

const Split* HambidgeTreeNode::getSplit() const
{
  return &split_;
}

int HambidgeTreeNode::getRatioIndex() const
{
  if (parent_ != 0L)
  {
    if (parent_->getLeft() == this)
    {
      return parent_->getSplit()->lhsIndex;
    }
    else
    {
      return parent_->getSplit()->rhsIndex;
    }
  }
  
  return root_->getRatioIndex();
}

double HambidgeTreeNode::getRatio() const
{
  if (root_ != 0L)
  {
    return root_->ratios()->GetRatio(getRatioIndex());
  }
  
  return 0.0;
}

double HambidgeTreeNode::getAspectRatio()
{
  return getWidth() / getHeight();
}

const Ratios* HambidgeTreeNode::getRatios() const
{
  if (root_ != 0L)
  {
    return root_->ratios();
  }
  
  return 0L;
}

HambidgeTreeNode* HambidgeTreeNode::clone(HambidgeTree* root, HambidgeTreeNode* parent)
{
  HambidgeTreeNode* node = new HambidgeTreeNode();
  node->setRoot(root);
  node->setParent(parent);
  node->split_ = split_;
  node->setLeft(left_ ? left_->clone(root, node) : 0L);
  node->setRight(right_ ? right_->clone(root, node) : 0L);
  return node;
}

double HambidgeTreeNode::getX()
{
  if (cachedX_ >= 0.0) return cachedX_;
  
  if (parent_)
  {
    switch (parent_->getSplitType())
    {
      case SPLIT_NONE:
      case SPLIT_HORIZONTAL:
        return cachedX_ = parent_->getX();
      default:
        return cachedX_ = this == parent_->getLeft() ? parent_->getX() : parent_->getX() + parent_->getLeft()->getWidth();
    }
  }
  
  return cachedX_ = 0.0;
}

double HambidgeTreeNode::getY()
{
  if (cachedY_ >= 0.0) return cachedY_;
  
  if (parent_ != 0L)
  {
    switch (parent_->getSplitType())
    {
      case SPLIT_NONE:
      case SPLIT_VERTICAL:
        return cachedY_ = parent_->getY();
      default:
        return cachedY_ = this == parent_->getLeft() ? parent_->getY() : parent_->getY() + parent_->getLeft()->getHeight();
    }
  }
  
  return cachedY_ = 0.0;
}

double HambidgeTreeNode::getArea()
{
  if (cachedArea_ > 0.0) return cachedArea_;
  cachedArea_ = getWidth() * getHeight();
  return cachedArea_;
}

double HambidgeTreeNode::getWidth()
{
  if (cachedWidth_ > 0.0) return cachedWidth_;
  
  if (parent_ != 0L)
  {
    switch (parent_->getSplitType())
    {
      case SPLIT_NONE:
      case SPLIT_HORIZONTAL:
        return cachedWidth_ = parent_->getWidth();
      default:
        const int index = this == parent_->getLeft() ? parent_->getSplit()->lhsIndex : parent_->getSplit()->rhsIndex;
        return cachedWidth_ = parent_->getWidth() *
        getRatios()->GetRatio(index) /
        getRatios()->GetRatio(parent_->getRatioIndex());
    }
  }
  else
  {
    return cachedWidth_ = getRatios()->GetRatio(root_->getRatioIndex()) * root_->getScale();
  }
}

double HambidgeTreeNode::getHeight()
{
  if (cachedHeight_ > 0.0) return cachedHeight_;
  
  if (parent_ != 0L)
  {
    switch (parent_->getSplitType())
    {
      case SPLIT_NONE:
      case SPLIT_VERTICAL:
        return cachedHeight_ = parent_->getHeight();
      default:
        const int index = this == parent_->getLeft() ? parent_->getSplit()->lhsIndex : parent_->getSplit()->rhsIndex;
        return cachedHeight_ = parent_->getHeight() *
        getRatios()->GetRatio(parent_->getRatioIndex()) / 
        getRatios()->GetRatio(index);
    }
  }
  else
  {
    return cachedHeight_ = root_->getScale();
  }
}

void HambidgeTreeNode::getLeaves(std::vector<HambidgeTreeNode*>* leaves)
{
  if (isLeaf())
  {
    leaves->push_back(this);
  }
  else
  {
    left_->getLeaves(leaves);
    right_->getLeaves(leaves);
  }
}

void HambidgeTreeNode::getAreas(std::vector<double>* areas)
{
  if (isLeaf())
  {
    areas->push_back(getArea());
  }
  else
  {
    left_->getAreas(areas);
    right_->getAreas(areas);
  }
}

void HambidgeTreeNode::getDimensions(std::vector<double>* dimensions)
{
  if (isLeaf())
  {
    dimensions->push_back(getWidth());
    dimensions->push_back(getHeight());
  }
  else
  {
    left_->getDimensions(dimensions);
    right_->getDimensions(dimensions);
  }
}

bool HambidgeTreeNode::isLeaf() const
{
  return split_.type == SPLIT_NONE;
}

HambidgeTreeNode* HambidgeTreeNode::getLeft()
{
  return left_;
}

HambidgeTreeNode* HambidgeTreeNode::getRight()
{
  return right_;
}

void HambidgeTreeNode::serialize(std::streambuf* buf)
{
  buf->sputc((unsigned char) getSplitType());
  
  if (!isLeaf())
  {
    buf->sputc(split_.lhsIndex);
    buf->sputc(split_.rhsIndex);
    
    left_->serialize(buf);
    right_->serialize(buf);
  }
}

void HambidgeTreeNode::deserialize(std::streambuf* buf, HambidgeTree* root, HambidgeTreeNode* parent)
{
  root_ = root;
  parent_ = parent;
  split_.type = (SplitType) buf->sgetc();
  
  if (split_.type != SPLIT_NONE)
  {
    split_.lhsIndex = buf->sgetc();
    split_.rhsIndex = buf->sgetc();
    left_ = new HambidgeTreeNode();
    right_ = new HambidgeTreeNode();
    left_->deserialize(buf, root, this);
    right_->deserialize(buf, root, this);
  }
}

void HambidgeTreeNode::setRoot(HambidgeTree* root)
{
  root_ = root;
  
  if (!isLeaf())
  {
    left_->setRoot(root);
    right_->setRoot(root);
  }
}

void HambidgeTreeNode::setParent(HambidgeTreeNode* parent)
{
  parent_ = parent;
}

void HambidgeTreeNode::setLeft(HambidgeTreeNode* left)
{
  left_ = left;
}

void HambidgeTreeNode::setRight(HambidgeTreeNode* right)
{
  right_ = right;
}
