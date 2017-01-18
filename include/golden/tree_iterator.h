//
//  treeiterator.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/10/13.
//
//

#ifndef GOLDEN_TREEITERATOR_H
#define GOLDEN_TREEITERATOR_H

#include <stack>

namespace golden {

template <class T>
class TreeIterator {
public:
  TreeIterator(T* root)
  {
    node_chain_.push(root);
  }
  
  inline bool HasNext() const
  {
    return !node_chain_.empty();
  }
  
  T* Next()
  {
    if (HasNext()) {
      T* node = node_chain_.top();
      node_chain_.pop();
      
      if (!node->IsLeaf()) {
        node_chain_.push(node->right());
        node_chain_.push(node->left());
      }
      
      return node;
    }
    
    return 0L;
  }

private:
  std::stack<T *> node_chain_;
};



} // namespace golden

#endif // GOLDEN_TREEITERATOR_H
