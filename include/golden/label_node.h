//
//  labelnode.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/10/13.
//
//

#ifndef GOLDEN_LABELNODE_H
#define GOLDEN_LABELNODE_H

namespace golden {

struct Label {
  Label(RatioIndex ratio_index, int first_index)
  :ratio_index(ratio_index),
   indexes(std::vector<int>(1, first_index))
  {}
  
  RatioIndex ratio_index;
  std::vector<int> indexes;
};


class LabelNode {
public:
  LabelNode(RatioIndex ratio_index)
  :ratio_index_(ratio_index),
   split_type_(kSplitTypeNone),
   leaf_labels_(0L),
   leaf_label_count_(0),
   left_(0L),
   right_(0L)
  {}
  
  ~LabelNode()
  {
    if (leaf_labels_ != 0L) {
      delete [] leaf_labels_;
    }
    
    if (left_ != 0L) {
      delete left_;
    }
    
    if (right_ != 0L) {
      delete right_;
    }
  }
  
  // Splits as a node, takes ownership of left and right
  void Init(SplitType split_type, LabelNode* left, LabelNode* right)
  {
    assert(split_type_ == kSplitTypeNone);
    assert(leaf_label_count_ == 0);
    
    split_type_ = split_type;
    left_ = left;
    right_ = right;
  }
  
  //! Labels as a leaf, this can only be called once!
  void Init(const Label& label)
  {
    assert(split_type_ == kSplitTypeNone);
    assert(leaf_label_count_ == 0);
    
    leaf_label_count_ = (int)label.indexes.size();
    leaf_labels_ = new int[leaf_label_count_];
    std::copy(label.indexes.begin(), label.indexes.end(), leaf_labels_);
  }
  
  bool IsLeaf() const
  {
    return split_type_ == kSplitTypeNone;
  }
  
  const LabelNode* left() const
  {
    return left_;
  }
  
  const LabelNode* right() const
  {
    return right_;
  }
  
  const int* leaf_labels() const
  {
    return leaf_labels_;
  }
  
  int leaf_labels_size() const
  {
    return leaf_label_count_;
  }
  
  RatioIndex ratio_index() const
  {
    return ratio_index_;
  }
  
  SplitType split_type() const
  {
    return split_type_;
  }

private:
  int leaf_label_count_;
  int* leaf_labels_;
  RatioIndex ratio_index_;
  SplitType split_type_;
  LabelNode* left_;
  LabelNode* right_;
};

}

#endif // GOLDEN_LABELNODE_H
