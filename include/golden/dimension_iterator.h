//
//  dimension_iterator.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/10/13.
//
//

#ifndef GOLDEN_DIMENSION_ITERATOR_H
#define GOLDEN_DIMENSION_ITERATOR_H

#include "golden/geometry.h"

namespace golden {

class DimensionIterator {
public:
  DimensionIterator(const LabelNode* root, const Ratios* ratios)
  :root_(root),
   ratios_(ratios)
  {
    node_stack_.push(root);
    dimension_stack_.push(Box(0.0, 0.0, (*ratios)[root->ratio_index()], 1.0));
  }
  
  bool HasNext() const
  {
    return !node_stack_.empty();
  }
  
  const LabelNode* Next(Box& box)
  {
    const LabelNode* node = node_stack_.top();
    box = dimension_stack_.top();
    
    node_stack_.pop();
    dimension_stack_.pop();
    
    if (!node->IsLeaf()) {
      double ratio = (*ratios_)[node->ratio_index()];
      const LabelNode* left = node->left();
      const LabelNode* right = node->right();

      if (node->split_type() == kSplitTypeHorizontal) {
        const double left_height = box.height() * ratio / (*ratios_)[left->ratio_index()];
        dimension_stack_.push(Box(box.left, box.top + left_height, box.right, box.bottom));
        dimension_stack_.push(Box(box.left, box.top, box.right, box.top + left_height));
        
      } else {
        assert(node->split_type() != kSplitTypeNone);
        const double left_width = box.width() * (*ratios_)[left->ratio_index()] / ratio;
        dimension_stack_.push(Box(box.left + left_width, box.top, box.right, box.bottom));
        dimension_stack_.push(Box(box.left, box.top, box.left + left_width, box.bottom));
      }
      
      node_stack_.push(right);
      node_stack_.push(left);
    }
    
    return node;
  }
  
private:
  const LabelNode* root_;
  const Ratios* ratios_;
  std::stack<Box> dimension_stack_;
  std::stack<const LabelNode *> node_stack_;
};


}





#endif // GOLDEN_DIMENSION_ITERATOR_H
