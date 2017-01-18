//
//  grouper.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/9/13.
//
//

#ifndef GOLDEN_GROUPER_H
#define GOLDEN_GROUPER_H

#include <cstdio>
#include <stack>


#include "golden/golden.h"
#include "golden/ratios.h"
#include "golden/label_node.h"

namespace golden {


class GroupNode {
public:
  GroupNode(int labels_size, RatioIndex ratio_index, int leaf_label)
  :label_counts_(labels_size, 0),
   label_scales_(labels_size, 0L),
   ratio_index_(ratio_index),
   all_labels_count_(1),
   split_type_(kSplitTypeNone),
   min_hscale_(2.0),
   min_vscale_(2.0),
   left_(0L),
   right_(0L)
  {
    label_counts_[leaf_label] = 1;
    label_scales_[leaf_label] = new double[1];
    label_scales_[leaf_label][0] = 1.0;
    //min_label_index_ = max_label_index_ = leaf_label;
    hash_ = GenerateHash();
  }
  
  GroupNode(int labels_size, RatioIndex ratio_index, SplitType split_type, GroupNode* left, GroupNode* right, double left_scale, double right_scale)
  :label_counts_(labels_size, 0),
   label_scales_(labels_size, 0L),
   split_type_(split_type),
   ratio_index_(ratio_index),
   min_hscale_(2.0),
   min_vscale_(2.0),
   left_(left),
   right_(right)
  {
    //min_label_index_ = left->min_label_index_ < right->min_label_index_ ? left->min_label_index_ : right->min_label_index_;
    //max_label_index_ = left->max_label_index_ > right->max_label_index_ ? left->max_label_index_ : right->max_label_index_;
    // Need to merge scales
    // and label counts
    for (int i = 0; i < labels_size; ++i) {
      int label_count_left = left->label_counts_[i];
      int label_count_right = right->label_counts_[i];
      int label_count_both = label_count_left + label_count_right;
      label_counts_[i] = label_count_both;
      
      if (label_count_both > 0) {
        label_scales_[i] = new double[label_count_both];
        double* both_scales = label_scales_[i];
        double* left_scales = left->label_scales_[i];
        double* right_scales = right->label_scales_[i];
        
        
        if (label_count_right == 0) {
          assert(label_count_left == label_count_both);
          // Copy left array
          if (split_type == kSplitTypeVertical) {
            std::copy(left_scales, left_scales + label_count_left, both_scales);
          } else {
            for (int j = 0; j < label_count_left; ++j) {
              both_scales[j] = left_scales[j] * left_scale;
            }
          }
        } else if (label_count_left == 0) {
          assert(label_count_right == label_count_both);
          // Copy right array
          if (split_type == kSplitTypeVertical) {
            std::copy(right_scales, right_scales + label_count_right, both_scales);
          } else {
            for (int j = 0; j < label_count_right; ++j) {
              both_scales[j] = right_scales[j] * right_scale;
            }
          }
        } else {
          // Merge the two arrays
          // iterate through left and right scales inserting them in order
          int index_left = 0, index_right = 0, index_both = 0;
          
          
          if (split_type == kSplitTypeVertical) {
            // No scaling
            while (index_left < label_count_left && index_right < label_count_right) {
              if (left_scales[index_left] < right_scales[index_right]) {
                both_scales[index_both++] = left_scales[index_left++];
              } else {
                both_scales[index_both++] = right_scales[index_right++];
              }
            }
            
            if (index_left < label_count_left) {
              std::copy(left_scales + index_left, left_scales + label_count_left, both_scales + index_both);
            } else if (index_right < label_count_right){
              std::copy(right_scales + index_right, right_scales + label_count_right, both_scales + index_both);
            }
          } else {
            // Need to deal with scaling
            while (index_left < label_count_left && index_right < label_count_right) {
              if (left_scales[index_left] * left_scale < right_scales[index_right] * right_scale) {
                both_scales[index_both] = left_scales[index_left] * left_scale;
                index_left++;
              } else {
                both_scales[index_both] = right_scales[index_right] * right_scale;
                index_right++;
              }
              index_both++;
            }
            
            while (index_left < label_count_left) {
              both_scales[index_both++] = left_scales[index_left++] * left_scale;
            }
            
            while (index_right < label_count_right) {
              both_scales[index_both++] = right_scales[index_right++] * right_scale;
            }
          }
        }
      }
    }
    
    all_labels_count_ = left->all_labels_count_ + right->all_labels_count_;
    hash_ = GenerateHash();
  }
  
  
  virtual ~GroupNode()
  {
    for (int i = 0; i < label_scales_.size(); ++i) {
      if (label_counts_[i] > 0) {
        delete [] label_scales_[i];
      } else {
        assert(label_scales_[i] == 0L);
      }
    }
  }
  
  inline bool IsLeaf() const
  {
    return split_type_ == kSplitTypeNone;
  }
  
  unsigned int hash() const
  {
    return hash_;
  }
  
  double FindMinimumHeightScale(const Ratios& ratios)
  {
    if (min_hscale_ < 2.0) return min_hscale_;
    double min = 2.0;
    for (int i = 0; i < label_scales_.size(); ++i) {
      for (int j = 0; j < label_counts_[i]; ++j) {
        if (label_scales_[i][j] < min) {
          min = label_scales_[i][j];
        }
      }
    }
    return min_hscale_ = min;
  }
  
  double FindMinimumWidthScale(const std::vector<double>& label_ratios)
  {
    if (min_vscale_ < 2.0) return min_vscale_;
    double min = 1.0;
    for (int i = 0; i < label_scales_.size(); ++i) {
      for (int j = 0; j < label_counts_[i]; ++j) {
        double width = label_ratios[i] * label_scales_[i][j];
        if (width < min) {
          min = width;
        }
      }
    }
    return min_vscale_ = min;
  }
  
  /*
  int min_label_index() const
  {
    return min_label_index_;
  }
  
  int max_label_index() const
  {
    return max_label_index_;
  }
  */

  
  inline unsigned int GenerateHash() const
  {
    unsigned int hash = 5381;

    hash = ((hash << 5) + hash) + ratio_index_;
    
    for (int i = 0; i < label_counts_.size(); ++i) {
      hash = ((hash << 5) + hash) + label_counts_[i];
      for (int j = 0; j < label_counts_[i]; ++j) {
        unsigned int rounded = label_scales_[i][j] * 1000000 + 0.5;
        hash = ((hash << 5) + hash) + rounded;
      }
    }
    
    return hash;
  }
  
  bool Equals(const GroupNode* rhs) const
  {
    // Equals if all number of children are same and scales are same
    if (rhs->ratio_index_ != ratio_index_) return false;
    if (rhs->all_labels_count_ != all_labels_count_) return false;
    for (int i = 0; i < label_counts_.size(); ++i) {
      if (rhs->label_counts_[i] != label_counts_[i]) return false;
      for (int j = 0; j < label_counts_[i]; ++j) {
        if (abs(rhs->label_scales_[i][j] - label_scales_[i][j]) > 0.000001) return false;
      }
    }
    
    return true;
  }
  
  const GroupNode* left() const
  {
    return left_;
  }
  
  const GroupNode* right() const
  {
    return right_;
  }
  
  const SplitType split_type() const 
  {
    return split_type_;
  }
  
  const std::vector<int>& label_counts() const
  {
    return label_counts_;
  }
  
  const std::vector<double*>& label_scales() const
  {
    return label_scales_;
  }
  
  const int all_labels_count() const
  {
    return all_labels_count_;
  }
  
  RatioIndex ratio_index() const
  {
    return ratio_index_;
  }
  
  void Print() const
  {
    fprintf(stdout, "GroupNode[%d]", ratio_index_);
    for (int i = 0; i < label_counts_.size(); ++i) {
      fprintf(stdout, "\n\t");
      fprintf(stdout, "%d:", i);
      for (int j = 0; j < label_counts_[i]; ++j) {
        if (j != 0) fprintf(stdout, ",");
        fprintf(stdout, "%f", label_scales_[i][j]);
      }
    }
    fprintf(stdout, "\n");
  }

protected:
  //! This is a vector that contains the sizes for each possible label.
  /*!
    The labels include all children labels, a flattened set if you will.
   */
  std::vector<int> label_counts_;
  std::vector<double*> label_scales_;
  int all_labels_count_;
  RatioIndex ratio_index_;
  SplitType split_type_;
  GroupNode* left_;
  GroupNode* right_;
  unsigned int hash_;
  double min_hscale_;
  double min_vscale_;
  //int min_label_index_;
  //int max_label_index_;
};



class Grouper {
public:
  typedef std::unordered_map<unsigned int, std::vector<GroupNode *> > ExistsMap;
  typedef std::pair<unsigned int, std::vector<GroupNode *> > ExistsPair;
  
  
  Grouper(const RatioGroup& ratio_indexes, const Ratios& ratios)
  :indexes_(ratio_indexes),
   complements_rmap_flat_(0L),
   ratios_(ratios)
  {
    std::sort(indexes_.begin(), indexes_.end());
    complements_ = ratios.complements();
    BuildComplementsReverseMap(complements_);
    LabelIndexes();
  }
  
  virtual ~Grouper()
  {
    delete [] complements_rmap_flat_;
  }
  
  
  std::vector<LabelNode*> Group()
  {
    
    std::vector<LabelNode *> complete_trees;
    std::vector<GroupNode *> trees;
    std::vector<GroupNode *> new_trees;
    std::vector<GroupNode *> new_complete_trees;
    
    exists_lookup_.clear();
    //ComplementsReverseMap::const_iterator it;
    
    int labels_size = (int)labels_.size();
    int trees_size = labels_size;
    int new_trees_size = labels_size;
    const int indexes_size = (int)indexes_.size();
    const double min_wscale = 0.19;
    const double min_hscale = 0.08; // Only allow for a rect that is at least 1/10 of the total height
    
    // Build up the first level of leaves
    for (int i = 0; i < labels_size; ++i) {
      GroupNode* group_node = new GroupNode(labels_size, labels_[i].ratio_index, i);
      trees.push_back(group_node);
      exists_lookup_.insert(ExistsPair(group_node->hash(), std::vector<GroupNode *>(1, group_node)));
    }
    
    while (new_trees_size) {
      for (int i = 0; i < new_trees_size; ++i) {
        GroupNode* base_node = trees[trees_size - i - 1];
        
        for (int j = i; j < trees_size; ++j) {
          GroupNode* other_node = trees[trees_size - j - 1];
          
          if (base_node->all_labels_count() + other_node->all_labels_count() > indexes_.size()) {
            continue;
          }

          unsigned int compact_hash = HashRatioIndexesCompact(base_node->ratio_index(), other_node->ratio_index());
          if (complements_rmap_flat_[compact_hash] == 0) {
            continue;
          }
          
          //unsigned int hash = HashRatioIndexes(base_node->ratio_index(), other_node->ratio_index());
          //it = complements_rmap_.find(hash);

          // If the two trees can be combined, combine them in every possible way,
          // this pretty much is just a vertical, horizontal or both split
          if (IsDisjoint(base_node, other_node)) {
            
            unsigned int complement_flat = complements_rmap_flat_[compact_hash];
            int split_count_flat = complements_rmap_flat_[compact_hash] >> 30;
            
            //int split_count_map = (int)it->second.size();
            //assert(split_count_map == split_count_flat);
            
            for (int k = 0; k < split_count_flat; ++k, complement_flat >>= 15) {
              // TODO: extract this to another function and then 
              //double scale_left, scale_right;
              
                            
              SplitType split_type_flat = complement_flat & 0x4000 ? kSplitTypeHorizontal : kSplitTypeVertical;
              int index_flat = complement_flat & 0x3FFF;
              
              //SplitType split_type_map = it->second[k].split_type;
              //int index_map = it->second[k].index;
              //assert(split_type_map == split_type_flat);
              //assert(index_map == index_flat);
              
              // Here we figure out the scale imposed on the children, the childs
              // width can be gotten by taken its ratio multiplied by its scale in the label_scales_ array
              // height will simply be the scale
              if (IsUnconstrained(index_flat, split_type_flat, base_node, other_node, min_wscale, min_hscale)) {
                double scale_left, scale_right, ratio;
                ratio = ratios_[index_flat];
                if (split_type_flat == kSplitTypeHorizontal) {
                  scale_left = ratio / ratios_[base_node->ratio_index()];
                  scale_right = ratio / ratios_[other_node->ratio_index()];
                } else {
                  scale_left = 1.0;
                  scale_right = 1.0;
                }
                
                GroupNode* new_node = new GroupNode(labels_size, index_flat, split_type_flat, base_node, other_node, scale_left, scale_right);
                

                // Does the node already exist
                if (!InsertIfUnique(new_node)) {
                  delete new_node;
                  continue;
                }
                
                if (new_node->all_labels_count() == indexes_size) {
                  complete_trees.push_back(ConvertToLabelNode(new_node));
                  new_complete_trees.push_back(new_node);
                  //new_node->Print();
                } else {
                  new_trees.push_back(new_node);
                }
              }
            }
          }
        }
      }
      
      new_trees_size = (int)new_trees.size();
      trees_size += new_trees_size;
      fprintf(stdout, "Created %d new trees...\n", new_trees_size);
      
      trees.reserve(trees_size);
      trees.insert(trees.end(), new_trees.begin(), new_trees.end());

      new_trees.clear();
    }
    
    fprintf(stdout, "Found a total of %d complete tree combinations.\n", (int)complete_trees.size());

    for (int i = 0; i < trees.size(); ++i) {
      delete trees[i];
    }
    
    for (int i = 0; i < new_complete_trees.size(); ++i) {
      delete new_complete_trees[i];
    }
    
    return complete_trees;
  }
  
  bool IsDisjoint(const GroupNode* lhs, const GroupNode* rhs) const
  {
    //if (lhs->all_labels_count() + rhs->all_labels_count() > indexes_.size()) return false;
    int start = 0;//lhs->min_label_index() < rhs->min_label_index() ? lhs->min_label_index() : rhs->min_label_index();
    int end = labels_size_;//lhs->max_label_index() > rhs->max_label_index() ? lhs->max_label_index() : rhs->max_label_index();
    for (; start < end; ++start) {
      if (lhs->label_counts()[start] + rhs->label_counts()[start] > label_sizes_[start]) {
        return false;
      }
    }
    
    return true;
  };
  
  bool InsertIfUnique(GroupNode* group_node)
  {
    ExistsMap::iterator exists_it = exists_lookup_.find(group_node->hash());
    if (exists_it != exists_lookup_.end()) {
      for (std::vector<GroupNode*>::iterator cmp_it = exists_it->second.begin(); cmp_it != exists_it->second.end(); ++cmp_it) {
        if (group_node->Equals(*cmp_it)) {
          return false;
        }
      }
    }
    
    if (exists_it == exists_lookup_.end()) {
      exists_lookup_.insert(ExistsPair(group_node->hash(), std::vector<GroupNode *>(1, group_node)));
    } else {
      exists_it->second.push_back(group_node);
    }
    
    return true;
  };
  
  bool IsUnconstrained(int index_flat, SplitType split_type_flat, GroupNode* base_node, GroupNode* other_node, double min_wscale, double min_hscale)
  {
    // Here we figure out the scale imposed on the children, the childs
    // width can be gotten by taken its ratio multiplied by its scale in the label_scales_ array
    // height will simply be the scale
    double ratio = ratios_[index_flat];
    
    if (split_type_flat == kSplitTypeHorizontal) {
      double scale_top = ratio / ratios_[base_node->ratio_index()];
      double scale_bottom = ratio / ratios_[other_node->ratio_index()];

      // Figure out if any child of either one will now be smaller than the minimum height percentage
      const std::vector<double *>& scales_left = base_node->label_scales();
      const std::vector<int>& counts_left = base_node->label_counts();
      const std::vector<double *>& scales_right = other_node->label_scales();
      const std::vector<int>& counts_right = other_node->label_counts();
      
      for (int l = 0; l < labels_size_; ++l) {
        for (int m = 0; m < counts_left[l]; ++m) {
          if (scales_left[l][m] * scale_top < min_hscale) {
            return false;
          }
        }

        for (int m = 0; m < counts_right[l]; ++m) {
          if (scales_right[l][m] * scale_bottom < min_hscale) {
            return false;
          }
        }
      }
    } else {
      double scale_left = ratios_[base_node->ratio_index()] / ratio;
      double scale_right = ratios_[other_node->ratio_index()] / ratio;

      // Figure out if any child of either one will now be smaller than the minimum width
      const std::vector<double *>& scales_left = base_node->label_scales();
      const std::vector<int>& counts_left = base_node->label_counts();
      const std::vector<double *>& scales_right = other_node->label_scales();
      const std::vector<int>& counts_right = other_node->label_counts();
      
      for (int l = 0; l < labels_size_; ++l) {
        double sub_ratio = ratios_[labels_[l].ratio_index];
        
        for (int m = 0; m < counts_left[l]; ++m) {
          if (scales_left[l][m] * sub_ratio * scale_left / ratios_[base_node->ratio_index()] < min_wscale) {
            return false;
          }
        }

        for (int m = 0; m < counts_right[l]; ++m) {
          if (scales_right[l][m] * sub_ratio * scale_right / ratios_[other_node->ratio_index()] < min_wscale) {
            return false;
          }
        }
      }
    }
    
    return true;
  }
  
  LabelNode* ConvertToLabelNode(const GroupNode* group_root)
  {
    
    LabelNode* label_root = new LabelNode(group_root->ratio_index());
    //TreeIterator<const GroupNode> group_it(group_root);
  
    std::stack<LabelNode*> label_stack;
    std::stack<const GroupNode*> group_stack;
    label_stack.push(label_root);
    group_stack.push(group_root);
    
    
    while (!group_stack.empty()) {
      const GroupNode* group_node = group_stack.top();
      LabelNode* label_node = label_stack.top();
      group_stack.pop();
      label_stack.pop();
      assert(label_node != 0L);
      
      if (!group_node->IsLeaf()) {
        LabelNode* right = new LabelNode(group_node->right()->ratio_index());
        LabelNode* left = new LabelNode(group_node->left()->ratio_index());
        label_node->Init(group_node->split_type(), left, right);
        label_stack.push(right);
        label_stack.push(left);
        group_stack.push(group_node->right());
        group_stack.push(group_node->left());
      } else {
        std::map<RatioIndex, int>::iterator it = index_label_map_.find(group_node->ratio_index());
        assert(it != index_label_map_.end());
        const Label& label = labels_[it->second];
        label_node->Init(label);
      }
    }
    
    assert(label_root->split_type() != kSplitTypeNone);
    return label_root;
  }

  
private:
  inline unsigned int HashRatioIndexes(RatioIndex left, RatioIndex right) const 
  {
    return left < right ?
            ((left & 0xFFFF) << 16) | (right & 0xFFFF) :
            ((right & 0xFFFF) << 16) | (left & 0xFFFF);
  }
  
  inline unsigned int HashRatioIndexesCompact(RatioIndex left, RatioIndex right) const
  {
    return left < right ?
            ((left & 0xFF) << 8) | (right & 0xFF) :
            ((right & 0xFF) << 8) | (left & 0xFF);
  }
  

  
  void BuildComplementsReverseMap(const Complements& complements)
  {
    if (complements_rmap_flat_ != 0L) {
      delete [] complements_rmap_flat_;
    }
    
    int ratios_size = (int)ratios_.ratios().size();
    int max_compact_hash = ((ratios_size - 1) << 8) + ratios_size - 1;
    
    // Create the flat version, initialize to zero
    complements_rmap_flat_ = new unsigned int[max_compact_hash]();
    // If we find a match the top bit signifies that a match is found
    
    for (int i = 0; i < complements.size(); ++i) {
      for (int j = 0; j < complements[i].size(); ++j) {
        unsigned int hash, compact_hash;
        ComplementsReverseMap::iterator it;
        const Split& split = complements[i][j];
        Complement complement = (Complement){.index = i, .split_type = split.type};

        hash = HashRatioIndexes(split.left_index, split.right_index);
        compact_hash = HashRatioIndexesCompact(split.left_index, split.right_index);
        
        if (complements_rmap_flat_[compact_hash] == 0) {
          // Initialize insert first match
          complements_rmap_flat_[compact_hash] |= (1 << 30);
          
          if (split.type == kSplitTypeHorizontal) {
            complements_rmap_flat_[compact_hash] |= (1 << 14);
          }
          
          complements_rmap_flat_[compact_hash] |= i & 0x3FFF;
          
        } else {
          // Signify there's two
          complements_rmap_flat_[compact_hash] &= ~(1 << 30);
          complements_rmap_flat_[compact_hash] |= (1 << 31);
          
          if (split.type == kSplitTypeHorizontal) {
            complements_rmap_flat_[compact_hash] |= (1 << 29);
          }
          
          complements_rmap_flat_[compact_hash] |= (i & 0x3FFF) << 15;
        }

        it = complements_rmap_.find(hash);

        if (it == complements_rmap_.end()) {
          complements_rmap_.insert(std::pair<unsigned int, std::vector<Complement> >(hash, std::vector<Complement>(1, complement)));
        } else {
          it->second.push_back(complement);
        }
      }
    }
  }
  
  void LabelIndexes()
  {
    labels_.clear();
    index_label_map_.clear();
    label_sizes_.clear();
    
    for (int i = 0; i < indexes_.size(); ++i) {
      RatioIndex ratio_index = indexes_[i];
      
      // See if the ratio already exists if so we add to that array, otherwise
      // we create a new index with this as the first index
      std::map<RatioIndex, int>::iterator map_it = index_label_map_.find(ratio_index);
      
      if (map_it == index_label_map_.end()) {
        labels_.push_back(Label(ratio_index, i));
        index_label_map_.insert(std::pair<RatioIndex, int>(ratio_index, (int)labels_.size() - 1));
      } else {
        labels_[map_it->second].indexes.push_back(i);
      }
    }
    
    // Cache all the label sizes
    label_sizes_.resize(labels_.size());
    for (int i = 0; i < labels_.size(); ++i) {
      label_sizes_[i] = (int)labels_[i].indexes.size();
    }
    
    labels_size_ = labels_.size();
  }

  //! A map from a unique ratio index to a vector of indexes into the indexes_ array
  std::vector<Label> labels_;
  int labels_size_;
  //! Maps from the ratio index to its entry in the labels_ array
  std::map<RatioIndex, int> index_label_map_;
  //! A cached value containing the size of each of the labels_ arrays
  std::vector<int> label_sizes_;
  //! The ratio indexes that need to be grouped
  RatioGroup indexes_;
  //! The ratios to use for the grouping
  Ratios ratios_;
  //! A complements array of the ratios to use for the grouping
  Complements complements_;
  //! A reverse map for looking up a complement by 2 ratio indexes
  ComplementsReverseMap complements_rmap_;
  /*! 
    A flattened version of the reverse map for faster lookup, this is an array
    of two byte values. We use the hash lookup as the index and the top bit signals
    if its a horizontal or vertical split.
  */
  unsigned int* complements_rmap_flat_;
  
  
  ExistsMap exists_lookup_;
};

} // namespace golden


#endif // GOLDEN_GROUPER_H
