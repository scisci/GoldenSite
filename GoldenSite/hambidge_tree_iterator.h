//
//  hambidge_tree_iterator.h
//  GoldenSite
//
//  Created by Daniel Riley on 5/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef GoldenSite_hambidge_tree_iterator_h
#define GoldenSite_hambidge_tree_iterator_h

#include "hambidge_tree.h"
#include "hambidge_tree_node.h"

#include <stack>

class HambidgeTreeIterator
{
public:
  HambidgeTreeIterator()
  :tree_(0L)
  {}
  
  void set_tree(HambidgeTree* tree)
  {
    tree_ = tree;
    while (!node_chain_.empty()) {
      node_chain_.pop();
    }
    node_chain_.push(tree->getRoot());
  }
  
  bool HasNext()
  {
    return !node_chain_.empty();
  }
  
  HambidgeTreeNode* Next()
  {
    if (HasNext()) {
      HambidgeTreeNode* node = node_chain_.top();
      node_chain_.pop();
      
      if (!node->isLeaf()) {
        node_chain_.push(node->getRight());
        node_chain_.push(node->getLeft());
      }
      
      return node;
    }
    
    return 0L;
  }

private:
  HambidgeTree* tree_;
  std::stack<HambidgeTreeNode *> node_chain_;
};

#endif
