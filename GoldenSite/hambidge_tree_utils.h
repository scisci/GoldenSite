//
//  hambidge_tree_utils.h
//  GoldenSite
//
//  Created by Daniel Riley on 5/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef GoldenSite_hambidge_tree_utils_h
#define GoldenSite_hambidge_tree_utils_h

#include "split.h"
#include "hambidge_tree.h"
#include "hambidge_tree_iterator.h"
#include "hungarian.h"

#include "assert.h"

#include <set>
#include <map>

typedef std::vector<int> Labels;

class LabelGroup {
public:
  LabelGroup(const LabelGroup& rhs)
  :ratio_index_(rhs.ratio_index_),
  leaf_label_(rhs.leaf_label_),
  labels_(rhs.labels_),
  split_type_(rhs.split_type_)
  {}
  
  
  LabelGroup(int ratio_index, int label)
  :ratio_index_(ratio_index),
  leaf_label_(label),
  //left_(0L),
  //right_(0L),
  split_type_(SPLIT_NONE),
  labels_(1, label)
  {
    //labels_[0] = label;
    //Labels::iterator it;
    //for (it=labels_.begin(); it!=labels_.end(); ++it)
    //  fprintf(stdout, "%d,", *it);
    //fprintf(stdout, "\n");
  }
  
  LabelGroup(int ratio_index, SplitType type, const LabelGroup* lhs, const LabelGroup* rhs)
  :ratio_index_(ratio_index),
  leaf_label_(-1),
  //left_(lhs),
  //right_(rhs),
  split_type_(type),
  labels_(lhs->labels_.size() + rhs->labels_.size())
  {
    //labels_.resize(lhs->labels_.size() + rhs->labels_.size());
    std::merge(lhs->labels_.begin(), lhs->labels_.end(), rhs->labels_.begin(), rhs->labels_.end(), labels_.begin());
    //labels_.insert(labels_.end(), lhs->labels_.begin(), lhs->labels_.end());
    //labels_.insert(labels_.end(), rhs->labels_.begin(), rhs->labels_.end());
    Labels::const_iterator it;
    //for (it=labels_.begin(); it!=labels_.end(); ++it)
    //  fprintf(stdout, "%d,", *it);
    //fprintf(stdout, "\n");
  }
  
  virtual ~LabelGroup() {}
  
  SplitType split_type() const
  {
    return split_type_;
  }
  
  int leaf_label() const
  {
    return leaf_label_;
  }
  
  bool ContainsLabel(int label) const
  {
    return std::find(labels_.begin(), labels_.end(), label) != labels_.end();
  }
  
  int ratio_index() const
  {
    return ratio_index_;
  }
  
  const Labels& labels() const
  {
    return labels_;
  }
  
  bool IsLeaf() const
  {
    return leaf_label_ > -1;
  }

  virtual void Sort() = 0;
  
  bool IsDisjoint(const LabelGroup* group) const
  {
    if (group == this) return false;
    const std::vector<int>& rhs_labels = group->labels();
    
    if (labels_.empty() || rhs_labels.empty()) return true;
    
    Labels::const_iterator it1 = labels_.begin();
    Labels::const_iterator it1_end = labels_.end();
    Labels::const_iterator it2 = rhs_labels.begin();
    Labels::const_iterator it2_end = rhs_labels.end();
    
    if (*it1 > *rhs_labels.rbegin() || *it2 > *labels_.rbegin()) return true;
    
    while (it1 != it1_end && it2 != it2_end) {
      if (*it1 == *it2) return false;
      if (*it1 < *it2) { it1++; }
      else { it2++; }
    }
    
    return true;
  }
  
  virtual const LabelGroup* left() const = 0;
  
  virtual const LabelGroup* right() const = 0;
  virtual void OffsetLabels(int offset) = 0;
  virtual void Swap() = 0;
  
  void Print() const
  {
    
    if (leaf_label_ > -1) {
      fprintf(stdout, "%d", leaf_label_);
    } else {
      assert(left() != 0L && right() != 0L);
      fprintf(stdout, "[");
      left()->Print();
      fprintf(stdout, ",");
      right()->Print();
      fprintf(stdout, "]");
    }
  }
  
  void PrintLabels() const
  {
    for (int i = 0; i < labels_.size(); ++i) {
      if (i == 0) fprintf(stdout, "%d", labels_[i]);
      else fprintf(stdout, ",%d", labels_[i]);
    }
  }
  
protected:
  int ratio_index_;
  int leaf_label_;
  Labels labels_;
  SplitType split_type_;
  //const LabelGroup* left_;
  //const LabelGroup* right_;
};

class LabelGroupShallow : public LabelGroup {
public:
  LabelGroupShallow(int ratio_index, int label)
  :LabelGroup(ratio_index, label),
  left_(0L),
  right_(0L)
  {}
  
  LabelGroupShallow(int ratio_index, SplitType type, const LabelGroup* lhs, const LabelGroup* rhs)
  :LabelGroup(ratio_index, type, lhs, rhs),
  left_(lhs),
  right_(rhs)
  {}
  
  virtual const LabelGroup* left() const
  {
    return left_;
  }
  
  virtual const LabelGroup* right() const
  {
    return right_;
  }
  
  virtual void OffsetLabels(int offset)
  {
    // Shouldn't get here!
    assert(0);
  }
  
  virtual void Swap()
  {
    const LabelGroup* temp = left_;
    left_ = right_;
    right_ = temp;
  }
  
  void Sort()
  {
    assert(0);
  }
  
private:
  const LabelGroup* left_;
  const LabelGroup* right_;
};

class LabelGroupDeep : public LabelGroup {
public:
  LabelGroupDeep(int ratio_index, int label)
  :LabelGroup(ratio_index, label),
  left_(0L),
  right_(0L)
  {}
  
  LabelGroupDeep(int ratio_index, SplitType type, LabelGroupDeep* lhs, LabelGroupDeep* rhs)
  :LabelGroup(ratio_index, type, lhs, rhs),
  left_(lhs),
  right_(rhs)
  {}
  
  LabelGroupDeep(const LabelGroup& rhs)
  :LabelGroup(rhs),
  left_(0L),
  right_(0L)
  {
    if (!rhs.IsLeaf()) {
      assert(rhs.left() != 0L && rhs.right() != 0L);
      left_ = new LabelGroupDeep(*rhs.left());
      right_ = new LabelGroupDeep(*rhs.right());
      
    }
  }
  
  ~LabelGroupDeep()
  {
    delete left_;
    delete right_;
  }
  
  virtual void OffsetLabels(int offset)
  {
    std::vector<int>::iterator it;
    for (it = labels_.begin(); it != labels_.end(); ++it) {
      *it += offset;
    }

    if (IsLeaf()) {
      leaf_label_ += offset;
    } else {
      left_->OffsetLabels(offset);
      right_->OffsetLabels(offset);
    }
  }
  
  virtual void Swap()
  {
    LabelGroupDeep* temp = left_;
    left_ = right_;
    right_ = temp;
  }
  
  void Sort()
  {
    if (!IsLeaf()) {
      left_->Sort();
      right_->Sort();
      
      int switch_count = 0;
      double mid_point = labels_.front() + (labels_.back() - labels_.front()) * 0.5;
      const std::vector<int>& left_labels = left_->labels();
      const std::vector<int>& right_labels = right_->labels();
      
      for (int i = 0; i < left_labels.size(); ++i) {
        if (left_labels[i] > mid_point) {
          switch_count++;
        } else {
          switch_count--;
        }
      }
      
      for (int i = 0; i < right_labels.size(); ++i) {
        if (right_labels[i] < mid_point) {
          switch_count++;
        } else {
          switch_count--;
        }
      }
      
      if (switch_count > 0) {
        LabelGroupDeep* temp = left_;
        left_ = right_;
        right_ = temp;
      }
      

    }
  }
  
  virtual const LabelGroup* left() const
  {
    return left_;
  }
  
  virtual const LabelGroup* right() const
  {
    return right_;
  }
  
private:
  LabelGroupDeep* left_;
  LabelGroupDeep* right_;
};

struct LabelGroupDimension {
  LabelGroupDimension(const LabelGroup* group, double left, double top, double right, double bottom)
  :group(group),
  left(left),
  top(top),
  right(right),
  bottom(bottom)
  {}
  
  inline double height() const
  {
    return bottom - top;
  }
  
  inline double width() const
  {
    return right - left;
  }
  
  const LabelGroup* group;
  double left;
  double top;
  double right;
  double bottom;
};

class LabelGroupDimensionIterator {
public:
  LabelGroupDimensionIterator()
  :root_(0L),
   ratios_(0L)
  {}
  
  LabelGroupDimensionIterator(const LabelGroup* root, const Ratios* ratios)
  :root_(root),
  ratios_(ratios)
  {
    dimensions_.push(LabelGroupDimension(root, 0.0, 0.0, (*ratios)[root->ratio_index()], 1.0));
  }
  
  // Finally we can do a cost assessment based on the items in the various
  // groups comparing each item to each other item.
  bool HasNext()
  {
    return !dimensions_.empty();
  }
  
  LabelGroupDimension Next()
  {
    LabelGroupDimension dimension = dimensions_.top();
    dimensions_.pop();
    
    if (!dimension.group->IsLeaf()) {
      double ratio = (*ratios_)[dimension.group->ratio_index()];
      const LabelGroup* left = dimension.group->left();
      const LabelGroup* right = dimension.group->right();
      
      /*
      if (dimension.group->split_type() == SPLIT_HORIZONTAL) {
        const double left_height = dimension.height * ratio / (*ratios_)[left->ratio_index()];
        const double right_height = dimension.height * ratio / (*ratios_)[right->ratio_index()];
        dimensions_.push(LabelGroupDimension(right, dimension.x, dimension.y + left_height, dimension.width, right_height));
        dimensions_.push(LabelGroupDimension(left, dimension.x, dimension.y, dimension.width, left_height));
      } else {
        assert(dimension.group->split_type() != SPLIT_NONE);
        const double left_width = dimension.width * (*ratios_)[left->ratio_index()] / ratio;
        const double right_width = dimension.width * (*ratios_)[right->ratio_index()] / ratio;
        dimensions_.push(LabelGroupDimension(right, dimension.x + left_width, dimension.y, right_width, dimension.height));
        dimensions_.push(LabelGroupDimension(left, dimension.x, dimension.y, left_width, dimension.height));
      }
       */
      if (dimension.group->split_type() == SPLIT_HORIZONTAL) {
        const double left_height = dimension.height() * ratio / (*ratios_)[left->ratio_index()];
        //const double right_height = dimension.height() * ratio / (*ratios_)[right->ratio_index()];
        dimensions_.push(LabelGroupDimension(right,
                                             dimension.left,
                                             dimension.top + left_height,
                                             dimension.right,
                                             dimension.bottom));
        dimensions_.push(LabelGroupDimension(left,
                                             dimension.left,
                                             dimension.top,
                                             dimension.right,
                                             dimension.top + left_height));
      } else {
        assert(dimension.group->split_type() != SPLIT_NONE);
        const double left_width = dimension.width() * (*ratios_)[left->ratio_index()] / ratio;
        //const double right_width = dimension.width * (*ratios_)[right->ratio_index()] / ratio;
        dimensions_.push(LabelGroupDimension(right,
                                             dimension.left + left_width,
                                             dimension.top,
                                             dimension.right,
                                             dimension.bottom));
        dimensions_.push(LabelGroupDimension(left,
                                             dimension.left,
                                             dimension.top,
                                             dimension.left + left_width,
                                             dimension.bottom));
      }
    }
    
    return dimension;
  }
  
  
private:
  const LabelGroup* root_;
  const Ratios* ratios_;
  std::stack<LabelGroupDimension> dimensions_;
};





struct HambidgeRectangle {
  double ratio;
  double importance;
};


struct LayoutConstraints {
  LayoutConstraints()
  :min_rectangle_width(DBL_MIN),
   min_rectangle_height(DBL_MIN),
   max_rectangle_width(DBL_MAX),
   max_rectangle_height(DBL_MAX),
   width(0.0),
   padding(0.0)
  {}
  
  double padding;
  double width;
  double min_rectangle_width;
  double max_rectangle_width;
  double min_rectangle_height;
  double max_rectangle_height;
};

class Layout {
public:
  Layout(const Layout& rhs)
  :width_(rhs.width_)
  {
    for (int i = 0; i < rhs.groups_.size(); ++i) {
      groups_.push_back(new LabelGroupDeep(*rhs.groups_[i]));
    }
  }
  
  Layout(double width, const std::vector<const LabelGroup *>& groups)
  :width_(width)
  {
    for (int i = 0; i < groups.size(); ++i) {
      groups_.push_back(new LabelGroupDeep(*groups[i]));
    }
  }
  
  const Layout& operator=(const Layout& rhs)
  {
    if (&rhs != this) {
      clear();
      width_ = rhs.width_;
      for (int i = 0; i < rhs.groups_.size(); ++i) {
        groups_.push_back(new LabelGroupDeep(*rhs.groups_[i]));
      }
    }
    
    return *this;
  }
  
  static bool SortGroupsByRange(const LabelGroup* lhs, const LabelGroup* rhs)
  {
    if (lhs->labels().back() < rhs->labels().front()) {
      return true;
    } else if (rhs->labels().back() < lhs->labels().front()) {
      return false;
    } else if (lhs->labels().front() < rhs->labels().front()) {
      return true;
    } else if (rhs->labels().front() < lhs->labels().front()) {
      return false;
    } else if (lhs->labels().back() < rhs->labels().back()) {
      return true;
    }
    
    return false;
  }
  
  void Sort()
  {
    std::sort(groups_.begin(), groups_.end(), SortGroupsByRange);
    for (int i = 0; i < groups_.size(); ++i) {
      groups_[i]->Sort();
    }
  }
  
  void OffsetLabels(int offset) {
    for (int i = 0; i < groups_.size(); ++i) {
      groups_[i]->OffsetLabels(offset);
    }
  }
  
  void clear()
  {
    for (int i = 0; i < groups_.size(); ++i) {
      delete groups_[i];
    }
    
    groups_.clear();
  }
  ~Layout()
  {
    clear();
  }
  
  const double width() const
  {
    return width_;
  }
  
  const std::vector<LabelGroup *>& groups() const
  {
    return groups_;
  }
  
private:
  double width_;
  std::vector<LabelGroup *> groups_;
};

class LayoutDimensionIterator {
public:
  LayoutDimensionIterator(const Layout* layout, const Ratios* ratios)
  :layout_(layout),
   ratios_(ratios),
   offset_(0.0)
  {
    group_iterator = layout->groups().begin();
    dimension_iterator = LabelGroupDimensionIterator(*group_iterator, ratios);
    scale_ = layout_->width() / (*ratios_)[(*group_iterator)->ratio_index()];
  }
  
  // Finally we can do a cost assessment based on the items in the various
  // groups comparing each item to each other item.
  bool HasNext()
  {
    if (!dimension_iterator.HasNext()) {
      return group_iterator != layout_->groups().end() && group_iterator != layout_->groups().end() - 1;
    }
    
    return true;
  }
  
  LabelGroupDimension Next()
  {
    LabelGroupDimension dimension = dimension_iterator.Next();
    dimension.left *= scale_;
    dimension.top = dimension.top * scale_ + offset_;
    dimension.right *= scale_;
    dimension.bottom = dimension.bottom * scale_ + offset_;
    
    /*
    dimension.x *= scale_;
    dimension.y = dimension.y * scale_ + offset_;
    dimension.width *= scale_;
    dimension.height *= scale_;
     */
    
    if (!dimension_iterator.HasNext()) {
      offset_ += scale_;
      if (group_iterator != layout_->groups().end()) {
        if (++group_iterator != layout_->groups().end()) {
          dimension_iterator = LabelGroupDimensionIterator(*group_iterator, ratios_);
          scale_ = layout_->width() / (*ratios_)[(*group_iterator)->ratio_index()];
        }
      }
    }
    
    return dimension;
  }
  
  
private:
  std::vector<LabelGroup *>::const_iterator group_iterator;
  LabelGroupDimensionIterator dimension_iterator;
  const Layout* layout_;
  const Ratios* ratios_;
  double offset_;
  double scale_;
};

class LayoutBuilder {
public:
  bool IsDisjoint(const LabelGroup* group) const
  {
    std::vector<const LabelGroup *>::const_iterator it;
    for (it = groups_.begin(); it != groups_.end(); ++it) {
      if (!(*it)->IsDisjoint(group)) {
        return false;
      }
    }
    
    return true;
  }
  
  
  
  const std::vector<int>& labels() const
  {
    return labels_;
  }
  
  const std::vector<const LabelGroup*> groups() const
  {
    return groups_;
  }
  
  bool IsComplete(int labels_size) const
  {
    return labels_.size() >= labels_size;
  }
  
  void set_cost(double cost)
  {
    cost_ = cost;
  }
  
  void Push(const LabelGroup* group)
  {
    // Add labels from group
    
    
    std::vector<int> temp(labels_.size() + group->labels().size());
    std::merge(labels_.begin(), labels_.end(), group->labels().begin(), group->labels().end(), temp.begin());
    /*
    fprintf(stdout, "Merging ");
    for (int i = 0; i < labels_.size(); ++i) {
      fprintf(stdout, "%d,", labels_[i]);
    }
    fprintf(stdout, " with ");
    
    for (int i = 0; i < group->labels().size(); ++i) {
      fprintf(stdout, "%d,", group->labels()[i]);
    }
    fprintf(stdout, " = ");
    
    for (int i = 0; i < temp.size(); ++i) {
      fprintf(stdout, "%d,", temp[i]);
    }
    fprintf(stdout, "\n");
    */
    labels_ = temp;
    groups_.push_back(group);
  }
  
  void Pop()
  {
    assert(!groups_.empty());
    std::vector<int> temp(labels_.size() - groups_.back()->labels().size());
    std::set_difference (labels_.begin(), labels_.end(), groups_.back()->labels().begin(), groups_.back()->labels().end(), temp.begin());
    
    
    
    labels_ = temp;
    groups_.pop_back();
  }
  
  bool IsEmpty()
  {
    return groups_.empty();
  }
  
  void clear()
  {
    labels_.clear();
    groups_.clear();
  }
  
  double cost() const
  {
    return cost_;
  }
  
private:
  double cost_;
  std::vector<int> labels_;
  std::vector<const LabelGroup*> groups_;
};

struct HambidgeRectangleConstraints {
  double min_width;
  double min_height;
  double min_column_ratio;
  double min_row_ratio;
  std::set<int> leaf_ratios;
};

struct Complement {
  int ratio_index;
  SplitType split_type;
};

typedef std::map<unsigned int, std::vector<Complement> > ComplementsMap;
class HambidgeTreeUtils
{
public:
  static std::vector<HambidgeTree *>* buildAll(int num_splits, const Ratios& ratios, int ratio_index, const HambidgeRectangleConstraints& constraints)
  {
    int i;

    // Create a tree array that contains just the initial ratio index that we
    // are trying to fill.
    std::vector<HambidgeTree *>* base_trees = new std::vector<HambidgeTree *>();
    
    // Add the initial ratio index.
    base_trees->push_back(new HambidgeTree(&ratios, ratio_index));
    
    // Create a tree array to fill with the next cycle.
    std::vector<HambidgeTree*>* trees = new std::vector<HambidgeTree *>();
    std::vector<HambidgeTree*>* temp_trees = 0L;
    
    for (i = 1; i < num_splits; ++i) {
      std::vector<HambidgeTree *>::iterator it;
      for (it = base_trees->begin(); it != base_trees->end(); ++it) {
        splitTree(*it, trees, ratios, constraints);
        
        delete *it;
        *it = 0L;
      }
      
      base_trees->clear();
      
      fprintf(stderr, "split level:%d, %d trees", i, trees->size());
      
      temp_trees = trees;
      // Use the cleared empty vector for the new trees array
      trees = base_trees;
      // Base trees is what we just completed
      base_trees = temp_trees;
    }
    
    delete trees;
    return base_trees;
  }
  


  
  
  static bool SortLeavesLex(const LabelGroupDimension& lhs, const LabelGroupDimension& rhs)
  {
    if (lhs.top < rhs.top) return true;
    else if (lhs.top > rhs.top) return false;
    if (lhs.left > rhs.left) return false;
    return true;
  }
  static bool GroupsEqual(const LabelGroup* lhs, const LabelGroup* rhs, const Ratios& ratios)
  {
    if (lhs == rhs) return true;
    if (lhs->labels().size() != rhs->labels().size()) return false;
    if (lhs->ratio_index() != rhs->ratio_index()) return false;
    
    LabelGroupDimensionIterator it1(lhs, &ratios);
    LabelGroupDimensionIterator it2(rhs, &ratios);
    
    std::vector<LabelGroupDimension> leaves_lhs;
    std::vector<LabelGroupDimension> leaves_rhs;
    
    while (it1.HasNext()) {
      LabelGroupDimension dim1 = it1.Next();
      LabelGroupDimension dim2 = it2.Next();
      
      if (dim1.group->IsLeaf()) {
        leaves_lhs.push_back(dim1);
      }
      if (dim2.group->IsLeaf()) {
        leaves_rhs.push_back(dim2);
      }
    }
    
    std::sort(leaves_lhs.begin(), leaves_lhs.end(), SortLeavesLex);
    std::sort(leaves_rhs.begin(), leaves_rhs.end(), SortLeavesLex);
    
    for (int i = 0; i < leaves_lhs.size(); ++i) {
      if (fabs(leaves_lhs[i].left - leaves_rhs[i].left) > 0.0001 || fabs(leaves_lhs[i].top - leaves_rhs[i].top) > 0.0001) {
        return false;
      }
    }
    
    return true;
  }
  
  // Given a group of rectangles coerce them to their closes leaf ratio, then
  // find all possible groups that could contain them. A maximum of 8 rectangles
  // should be used per group
  static std::vector<LabelGroup *> CreateBaseGroups(const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const HambidgeRectangleConstraints& constraints, double min_cost)
  {
    int rectangle_count = rectangles.size();
    std::vector<int> ratio_indices;
    std::vector<std::set<int> > leaf_ratios = std::vector<std::set<int> >(rectangles.size(), constraints.leaf_ratios);
    
    std::set<int>::iterator it;
    for (it = constraints.leaf_ratios.begin(); it != constraints.leaf_ratios.end(); ++it) {
      fprintf(stdout, "Leaf Ratio:%.2f\n", ratios[*it]);
    }
    
    // First load the closest index for each rectangle
    for (int i = 0; i < rectangle_count; ++i) {
      int ratio_index = ratios.FindClosestIndex(rectangles[i].ratio, constraints.leaf_ratios);
      ratio_indices.push_back(ratio_index);
      leaf_ratios[i].erase(ratio_index);
      fprintf(stdout, "%.2f->%.2f(%d)\n", rectangles[i].ratio, ratios[ratio_index], ratio_index);
    }
    
    int min_dif_index = 0;
    std::vector<LabelGroup *> result;
    //Complements pruned_complements = PruneComplements(ratios.complements(), allowed_ratios);
    ComplementsMap complements_rmap = BuildComplementsMap(ratios.complements());
    

    while ((result = PermuteBaseGroups(rectangles, ratios, ratio_indices, complements_rmap, min_cost)).empty()) {
      // Reset min_dif_index, it failed
      ratio_indices[min_dif_index] = ratios.FindClosestIndex(rectangles[min_dif_index].ratio, constraints.leaf_ratios);
      
      // Find which permutation does the least damage
      double min_dif = DBL_MAX;
      min_dif_index = -1;
      
      for (int i = 0; i < rectangle_count; ++i) {
        if (leaf_ratios[i].empty()) continue;
        int ratio_index = ratios.FindClosestIndex(rectangles[i].ratio, leaf_ratios[i]);
        double dif = fabs(ratios[ratio_index] - rectangles[i].ratio);
        if (dif < min_dif) {
          min_dif = dif;
          min_dif_index = i;
        }
      }
      
      if (min_dif_index < 0) {
        break;
      } else {
        int ratio_index = ratios.FindClosestIndex(rectangles[min_dif_index].ratio, leaf_ratios[min_dif_index]);
        leaf_ratios[min_dif_index].erase(ratio_index);
        ratio_indices[min_dif_index] = ratio_index;
        fprintf(stdout, "%.2f->%.2f(%d)\n", rectangles[min_dif_index].ratio, ratios[ratio_index], ratio_index);
      }
    }
    
    return result;
  }
  
  static std::vector<LabelGroup *> PermuteBaseGroups(const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const std::vector<int>& ratio_indices, const ComplementsMap& complements_rmap, double min_cost_filter)
  {
    std::set<int> allowed_ratios;
    std::vector<LabelGroup *> label_groups;
    
    // Start with the leaves
    for (int i = 0; i < ratio_indices.size(); ++i) {
      label_groups.push_back(new LabelGroupShallow(ratio_indices[i], i));
      allowed_ratios.insert(ratio_indices[i]);
    }

    int error = 0;
    label_groups = Group(label_groups, rectangles, ratios, complements_rmap, min_cost_filter, false, error);
    
    const int best_size = 20;
    int total = 0;
    std::vector<double> min_cost(best_size, DBL_MAX);
    std::vector<int> min_cost_index(best_size, -1);

    // Go through the list and see if there is anything worth keeping
    for (int i = 0; i < label_groups.size(); ++i) {
      if (label_groups[i]->labels().size() == rectangles.size()) {
        total++;
        double cost = CalculateGroupCost(rectangles, ratios, label_groups[i]);

        for (int j = 0; j < best_size; ++j) {
          if (min_cost_index[j] > -1 && GroupsEqual(label_groups[i], label_groups[min_cost_index[j]], ratios)) {
            break;;
          }
          
          if (cost < min_cost[j]) {
            for (int k = best_size - 1; k > j; --k) {
              min_cost[k] = min_cost[k-1];
              min_cost_index[k] = min_cost_index[k-1];
            }
            
            min_cost[j] = cost;
            min_cost_index[j] = i;
            break;
          }
        }
      }
    }
    
    fprintf(stdout, "Total possible groups:%d\n", total);
    
    std::vector<LabelGroup *> best;
    
    for (int i = 0; i < best_size; ++i) {
      if (min_cost_index[i] > -1) {
        best.push_back(new LabelGroupDeep(*label_groups[min_cost_index[i]]));
      } else {
        break;
      }
    }
    
    // Now we can free the label groups
    for (int i = 0; i < label_groups.size(); ++i) {
      delete label_groups[i];
    }
    
    
    return best;
  }
  
  struct SortLabelGroupsByCost {
    SortLabelGroupsByCost(const std::vector<HambidgeRectangle>& rects, const Ratios& ratios)
    :rectangles(&rects),
     ratios(&ratios)
    {}
    
    const std::vector<HambidgeRectangle>* rectangles;
    const Ratios* ratios;
    
    bool operator()(const LabelGroup* lhs, const LabelGroup* rhs)
    {
      //if (lhs->labels().size() > rhs->labels().size()) return true;
      //else if (lhs->labels().size() < rhs->labels().size()) return false;
      
      double cost_lhs = CalculateGroupCost(*rectangles, *ratios, lhs);
      double cost_rhs = CalculateGroupCost(*rectangles, *ratios, rhs);
      
      if (cost_lhs < cost_rhs) return true;
    }
  };
  
  
  static std::vector<LabelGroup *> Group(const std::vector<LabelGroup *>& groups, const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const ComplementsMap& complements_rmap, double min_cost, bool filter_exp, int& error)
  {
    error = 0;
    std::vector<LabelGroup *> label_groups = groups;
    size_t label_groups_size = label_groups.size();
    size_t max_size = rectangles.size();
    const int exp_threshold = 20000;
    const double exp_ratio = 0.70;

    while (label_groups_size) {
      // Iterate by combining every possible group
      std::vector<LabelGroup *> new_groups;

      for (int i = 0; i < label_groups_size; ++i) {
        const LabelGroup* base_group = label_groups[i];
        size_t base_group_size = base_group->labels().size();
        if (base_group_size == max_size) {
          continue;
        }
        
        for (int j = i + 1; j < label_groups.size(); ++j) {
          const LabelGroup* other_group = label_groups[j];
          size_t other_group_size = other_group->labels().size();
          if (other_group_size + base_group_size > max_size) {
            continue;
          }
          
          // First of all check if the two ratios exist in the complement table
          unsigned int hash = ((base_group->ratio_index() & 0xFFFF) << 16) | (other_group->ratio_index() & 0xFFFF);
          ComplementsMap::const_iterator it = complements_rmap.find(hash);
          if (it == complements_rmap.end()) {
            // Try the opposite way in case it wasn't in there
            hash = ((other_group->ratio_index() & 0xFFFF) << 16) | (base_group->ratio_index() & 0xFFFF);
            it = complements_rmap.find(hash);
          }
          
          
          if (it != complements_rmap.end()) {
            // If the group can be combined with a sister group without colliding
            // then combine and add to end
            bool disjoint = false;
            if (false /*max_dist > 0*/) {
              /*
              const int f1 = base_group->labels().front();
              const int b1 = base_group->labels().back();
              const int f2 = other_group->labels().front();
              const int b2 = other_group->labels().back();
              
              disjoint = f1 > b2 ? (f1 - b2 <= max_dist && f1 - b2 >= min_dist) :
                                   (f2 > b1 ? (f2 - b1 <= max_dist && f2 - b1 >= min_dist) :
                                    base_group->IsDisjoint(other_group));
              */
            } else {
              disjoint = base_group->IsDisjoint(other_group);
            }
            
            if (disjoint) {
              int num = it->second.size();
              assert(num > 0 && num <= 2);
              if (num == 2) {
                assert(it->second[0].ratio_index != it->second[1].ratio_index);
              }
              for (int k = 0; k < num; ++k) {
                LabelGroupShallow* new_group = new LabelGroupShallow(it->second[k].ratio_index, it->second[k].split_type, base_group, other_group);
                
                double cost = CalculateGroupCost(rectangles, ratios, new_group);
                // This is an arbitrary choice but cost function should return
                // value between 0 and 1
                if (cost <= min_cost) {
                  /*
                   fprintf(stdout, "Combined ");
                   base_group->Print();
                   fprintf(stdout, " with ");
                   other_group->Print();
                   fprintf(stdout, " made:");
                   new_group->Print();
                   fprintf(stdout, "\n");
                   */
                  
                  // Now we should check the result and make sure its not already existing
                  //bool exists = false;
                  /*
                   for (int l = 0; l < label_groups.size(); ++l) {
                   if (GroupsEqual(new_group, label_groups[l], ratios)) {
                   exists = true;
                   break;
                   }
                   }
                   */
                  /*
                   for (int l = 0; l < new_groups.size(); ++l) {
                   if (GroupsEqual(new_group, new_groups[l], ratios)) {
                   exists = true;
                   break;
                   }
                   }
                   */
                  //if (!exists) {
                  new_groups.push_back(new_group);
                  //} else {
                  // delete new_group;
                  //}
                } else {
                  delete new_group;
                }
              }
              //label_groups.push_back(new_group);
            }
          }
        }
      }
      
      size_t new_groups_size = new_groups.size();
      
      if (filter_exp && new_groups_size > 1500) {
        std::sort(new_groups.begin(), new_groups.end(), SortLabelGroupsByCost(rectangles, ratios));
        new_groups_size = 1500;
      }
      
      // Copy new groups in
      label_groups.insert(label_groups.begin(), new_groups.begin(), new_groups.begin() + new_groups_size);
      label_groups_size = new_groups_size;
      fprintf(stdout, "%ld/%ld\n", label_groups_size, label_groups.size());
      
      if (label_groups_size > exp_threshold && (double)label_groups_size / label_groups.size() > exp_ratio) {
        fprintf(stdout, "Exponential detected!\n");
        error = -1;
        break;
      }
    }

    return label_groups;
  }
  
  
  static std::vector<Layout> BuildLayout(const std::vector<LabelGroup *>& groups, const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const LayoutConstraints& layout_constraints, double min_cost)
  {
    LayoutBuilder layout;
    size_t layout_size;
    std::vector<LayoutBuilder> layouts;
    std::stack<int> it_stack;
    
    const size_t final_size = rectangles.size();
    
    int min_group_size = INT_MAX;
    int max_group_size = 0;
    for (int i = 0; i < groups.size(); ++i) {
      if (groups[i]->labels().size() < min_group_size) {
        min_group_size = groups[i]->labels().size();
      }
      
      if (groups[i]->labels().size() > max_group_size) {
        max_group_size = groups[i]->labels().size();
      }
    }

    int seed = 0;
    /*
    // Try only using max_group_sizes as seeds
    for (int i = 0; i < groups.size(); ++i) {
      if (groups[i]->labels().size() == max_group_size) {
        seed = i;
        break;
      }
    }*/
    
    layout_size = 0;
    it_stack.push(seed);

    while (!it_stack.empty()) {
      int pos = it_stack.top();
      const LabelGroup* group = groups[it_stack.top()];
      assert(group != 0L);
      const size_t group_size = group->labels().size();

      if (layout_size == 0 && group_size == final_size) {
        fprintf(stdout, "Already done.");
      }
      // If we can add the current group, try to add it
      if ((layout_size + group_size == final_size ||
           layout_size + group_size <= final_size - min_group_size) &&
          layout.IsDisjoint(group)) {
        // Calculate the cost of adding the group
        layout.Push(group);
        double cost = CalculateLayoutCost(rectangles, ratios, layout_constraints, layout);
        // If the cost is valid add the item
        if (cost > min_cost) {
          layout.Pop();
        } else {
          layout_size = layout.labels().size();
          // If we made a complete layout add it to our list of layouts, and
          // move the first iterator
          if (layout_size == final_size) {
            layout.set_cost(cost);
            
            fprintf(stdout, "Adding layout.\n");
            
            if (layouts.empty()) {
              layouts.push_back(layout);
            } else {
              std::vector<LayoutBuilder>::iterator insert_it;
              for (insert_it = layouts.begin(); insert_it != layouts.end(); ++insert_it) {
                if (insert_it->cost() > cost) {
                  break;
                }
              }
           
              
              layouts.insert(insert_it, layout);
            }
            
            
            //fprintf(stdout, "%ld layout\n", layouts.size());
            layout.clear();
            layout_size = 0;
            // Don't need to hold on to any iterators since they're all good
            // so pop down to the first level.
            while (it_stack.size() > 1) {
              it_stack.pop();
            }
            
            // Find the next item that is seedable
            //while (it_stack.top() + 1 < groups.size() && groups[it_stack.top() + 1]->labels().size() < max_group_size) {
            //  ++it_stack.top();
            //}
            
          } else {
            if (it_stack.top() < groups.size() - 1) {
              //fprintf(stdout, "pushing stack %ld\n", it_stack.size() + 1);
              it_stack.push(it_stack.top() + 1);
              continue;
            } else {
              layout.Pop();
              layout_size = layout.labels().size();
            }
          }
        }
      }
      
      // The current group didn't offer anything, check the next one, if we
      // reached the end, we've failed so go back and try to iterate the previous
      // guy
      if (it_stack.size() == 1) {
        fprintf(stdout,"finished %d/%ld\n", it_stack.top(), groups.size());
      }
      
      while (!it_stack.empty() && ++it_stack.top() == groups.size()) {
        it_stack.pop();
        //fprintf(stdout, "popped to level:%ld\n", it_stack.size());
        if (layout_size > 0) {
          layout.Pop();
          layout_size = layout.labels().size();
        }
      }
    }
    
    // Need to flatten all the layouts
    std::vector<Layout> flat_layouts;
    int layouts_count = layouts.size() > 20 ? 20 : layouts.size();
    for (int i = 0; i < layouts_count; ++i) {
      flat_layouts.push_back(Layout(layout_constraints.width, layouts[i].groups()));
    }
    
    return flat_layouts;
  }

  
  static double CalculateGroupCost(const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const LabelGroup* group)
  {
    // Finally we can do a cost assessment based on the items in the various
    // groups comparing each item to each other item.

    int i;
    double cost, total_area, total_weight;
    const Labels& labels = group->labels();
    std::vector<double> areas(rectangles.size(), 0.0);

    total_area = ratios[group->ratio_index()];
    
    LabelGroupDimensionIterator iterator(group, &ratios);
    
    while (iterator.HasNext()) {
      LabelGroupDimension dimension = iterator.Next();
      if (dimension.group->IsLeaf()) {
        areas[dimension.group->leaf_label()] = dimension.width() * dimension.height();
      }
    }
        // Tally up weights
    cost = 0.0;
    total_weight = 0.0;
    for (i = 0; i < labels.size(); ++i) {
      total_weight += rectangles[labels[i]].importance;
    }

    for (i = 0; i < labels.size(); ++i) {
      cost += fabs(areas[labels[i]] / total_area - rectangles[labels[i]].importance / total_weight);
    }

    // TODO: figure out what the maximum value could be
    return cost / labels.size();
  }
  
  
  static double CalculateLayoutCost(const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const LayoutConstraints& constraints, const LayoutBuilder& layout)
  {
    // Finally we can do a cost assessment based on the items in the various
    // groups comparing each item to each other item.
    
    int i, label_count;
    double cost, total_area, total_weight;
    //const Labels& labels = group->labels();
    std::vector<double> areas;
    std::vector<double> weights;
    
    total_area = 0.0;
    total_weight = 0.0;
    
    std::vector<const LabelGroup*>::const_iterator it;
    const std::vector<const LabelGroup*>& groups = layout.groups();
    
    for (it = groups.begin(); it != groups.end(); ++it) {
      double scale = constraints.width / ratios[(*it)->ratio_index()];
      
      total_area += constraints.width * scale;
      
      int constraint_errors = 0;
      
      LabelGroupDimensionIterator iterator(*it, &ratios);
      
      while (iterator.HasNext()) {
        LabelGroupDimension dimension = iterator.Next();
        if (dimension.group->IsLeaf()) {
          double leaf_width = dimension.width() * scale;
          double leaf_height = dimension.height() * scale;
          if (leaf_width < constraints.min_rectangle_width ||
              leaf_height < constraints.min_rectangle_height ||
              leaf_width > constraints.max_rectangle_width ||
              leaf_height > constraints.max_rectangle_height) {
            return 1.0;
            constraint_errors++;
          }
          
          double weight = rectangles[dimension.group->leaf_label()].importance;
          areas.push_back(leaf_width * leaf_height);
          weights.push_back(weight);
          total_weight += weight;
        }
      }
    }
    
    for (i = 0; i < areas.size(); ++i) {
      cost += fabs(areas[i] / total_area - weights[i] / total_weight);
    }
      
    // TODO: figure out what the maximum value could be
    return cost / areas.size();
  }

  
  
  
  static ComplementsMap BuildComplementsMap(const Complements& complements)
  {
    //fprintf(stdout, "Building complements reverse map:\n");
    ComplementsMap complements_rmap;
    
    for (int i = 0; i < complements.size(); ++i) {
      for (int j = 0; j < complements[i].size(); ++j) {
        unsigned int hash;
        ComplementsMap::iterator it;
        const Split& split = complements[i][j];
        
        hash = ((split.lhsIndex & 0xFFFF) << 16) | (split.rhsIndex & 0xFFFF);
        it = complements_rmap.find(hash);

        if (it == complements_rmap.end()) {
          complements_rmap.insert(std::pair<int, std::vector<Complement> >(hash, std::vector<Complement>()));
          it = complements_rmap.find(hash);
        }

        it->second.push_back((Complement){.ratio_index = i, .split_type = split.type});
        //fprintf(stdout, "(%d,%d,%s):%d\n", split.lhsIndex, split.rhsIndex, split.type == SPLIT_HORIZONTAL ? "h" : "v", i);
      }
    }
    return complements_rmap;
  }
  
  static std::vector<int> ConstrainRectangles(const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, const std::set<int>& subset)
  {
    fprintf(stdout, "Constraining rectangles:\n");
    std::vector<int> ratio_indices;
    std::vector<HambidgeRectangle>::const_iterator it;
    
    for (it = rectangles.begin(); it != rectangles.end(); it++) {
      int ratio_index = ratios.FindClosestIndex(it->ratio, subset);
      assert(ratio_index > -1);
      ratio_indices.push_back(ratio_index);
      fprintf(stdout, "%.2f->%.2f(%d)\n", it->ratio, ratios[ratio_index], ratio_index);
    }
    
    return ratio_indices;
  }
  
  /*!
   This method should return a list of complements only composed of ratios from
   the subset.
  */
  static Complements PruneComplements(const Complements& complements, const std::set<int>& subset)
  {
    Complements pruned_complements;
    Complements::const_iterator it;
    int index = 0;
    
    for (it = complements.begin(); it != complements.end(); ++it, ++index) {
      std::vector<Split> splits;
      std::vector<Split>::const_iterator split_it;
      
      for (split_it = it->begin(); split_it != it->end(); ++split_it) {
        if (subset.find(split_it->lhsIndex) != subset.end() && subset.find(split_it->rhsIndex) != subset.end()) {
          splits.push_back(*split_it);
        }
      }
      
      pruned_complements.push_back(splits);
      fprintf(stdout, "Complement %d: %ld splits pruned to %ld.\n", index, it->size(), splits.size());
    }
    
    return pruned_complements;
  }
  

  static HambidgeTree* getMatchingTree(const std::vector<HambidgeRectangle>& rectangles, const Ratios& ratios, int ratioIndex, HambidgeRectangleConstraints constraints)
  {
    std::vector<HambidgeTree*>* trees = buildAll(rectangles.size(), ratios, ratioIndex, constraints);
    std::vector<HambidgeTree *>::iterator it;

    std::vector<HambidgeTreeNode *> leaves;

    size_t rectangle_count = rectangles.size();
    
    hungarian_problem_t p;
    int** cost_matrix = new int*[rectangle_count];
    int** best_match = new int*[rectangle_count];
    for (int i = 0; i < rectangle_count; ++i) {
      cost_matrix[i] = new int[rectangle_count];
      best_match[i] = new int[rectangle_count];
    }
    
    int minimum_cost = INT_MAX;
    int minimum_cost_index = 0;
    int index = 0;
    for (it = trees->begin(); it != trees->end(); ++it, index++) {
      // Build a cost matrix between leaves and rectangles
      (*it)->getLeaves(&leaves);
      
      // Need to normalize importances based on area
      double maximum_area = DBL_MIN;
      
      if (leaves.size() != rectangle_count) {
        assert(0);
        leaves.clear();
        continue;
      }

      for (int i = 0; i < rectangle_count; ++i) {
        double area = leaves[i]->getArea();
        if (area > maximum_area) maximum_area = area;
      }

      for (int i = 0; i < rectangle_count; ++i) {
        double leaf_importance = maximum_area == 0.0 ? 0.5 : leaves[i]->getArea() / maximum_area;
        double leaf_ratio = leaves[i]->getAspectRatio();

        for (int j = 0; j < rectangle_count; ++j) {
          double importance_cost = rectangles[j].importance - leaf_importance;
          double ratio_cost = rectangles[j].ratio - leaf_ratio;
          int cost = sqrt(importance_cost * importance_cost + ratio_cost * ratio_cost) * 0xFFFF;
          cost_matrix[i][j] = cost;
          
          //fprintf(stderr, "Leaf[%d](%.2f,%.2f), Rect[%d](%.2f,%.2f) = %d\n", i, leaf_importance, leaf_ratio, j, rectangles[j].importance, rectangles[j].ratio, cost);
        }
      }
      
      int matrix_size = hungarian_init(&p, cost_matrix , rectangle_count, rectangle_count, HUNGARIAN_MODE_MINIMIZE_COST) ;
      //fprintf(stderr, "assignement matrix has a now a size %d rows and %d columns.\n\n",  matrix_size,matrix_size);
      /* some output */
      //fprintf(stderr, "cost-matrix:");
      //hungarian_print_costmatrix(&p);
      /* solve the assignement problem */
      int cost = hungarian_solve(&p);
      
      /* some output */
      //fprintf(stderr, "assignment:");
      //hungarian_print_assignment(&p);

      fprintf(stderr, "index: %d, cost: %d\n\n", index, cost);
      
      if (cost < minimum_cost) {
        minimum_cost = cost;
        minimum_cost_index = index;
        for (int i = 0; i < rectangle_count; ++i) {
          memcpy(best_match[i], p.assignment[i], rectangle_count * sizeof(int));
        }
      }
      
      /* free used memory */
      hungarian_free(&p);

            
      leaves.clear();
    }
    
    fprintf(stderr, "best match was index:%d", minimum_cost_index);

    HambidgeTree* tree = (*trees)[minimum_cost_index];
    
    for (int i = 0; i < trees->size(); i++) {
      if ((*trees)[i] != tree) {
        delete (*trees)[i];
      }
      
      (*trees)[i] = 0L;
    }
    
    delete trees;

    
    for (int i = 0; i < rectangle_count; ++i) {
      delete [] cost_matrix[i];
      delete [] best_match[i];
    }
    
    delete [] cost_matrix;
    delete [] best_match;
    
    return tree;
  }
#if 0
  static HambidgeTree* getMatchingTree(std::vector<double>* areas, const Ratios& ratios, int ratioIndex)
  {
    HambidgeRectangleConstraints constraints;
    std::vector<HambidgeTree*>* trees = buildAll(areas->size(), ratios, ratioIndex, constraints);
    
    std::sort(areas->begin(), areas->end());
    
    double minError = DBL_MAX;
    int minErrorIndex = -1;
    std::vector<double> leafAreas;
    
    for (int i = 0; i < trees->size(); i++)
    {
      double error = 0.0;
      (*trees)[i]->getAreas(&leafAreas);
      std::sort(leafAreas.begin(), leafAreas.end());
      
      for (int j = 0; j < leafAreas.size(); j++)
      {
        error += pow(((*areas)[j] - (leafAreas[j])) / (*areas)[j], 2);
      }
      
      error /= leafAreas.size();
      
      if (error < minError)
      {
        minError = error;
        minErrorIndex = i;
      }
      
      leafAreas.clear();
    }
    
    HambidgeTree* tree = 0L;
    
    if (minErrorIndex > -1)
    {
      tree = (*trees)[minErrorIndex];
    }
    
    for (int i = 0; i < trees->size(); i++)
    {
      if ((*trees)[i] != tree)
      {
        delete (*trees)[i];
      }
      
      (*trees)[i] = 0L;
    }
    
    delete trees;
    
    return tree;
  }
#endif 
  
  inline static bool IsSplittable(double width, double height, int ratio_index, const Split& split, const Ratios& ratios, const HambidgeRectangleConstraints& constraints, bool recursive)
  {
    const Complements& complements = ratios.complements();
    
    if (split.type == SPLIT_VERTICAL) {
      const double left_width = width * ratios[split.lhsIndex] / ratios[ratio_index];
      const double right_width = width * ratios[split.rhsIndex] / ratios[ratio_index];
      
      assert(left_width + right_width - width < 0.000001);
      
      if (left_width < constraints.min_width || right_width < constraints.min_width ||
          left_width / height < constraints.min_column_ratio ||
          right_width / height < constraints.min_column_ratio) {
        return false;
      }
      
      if (recursive) {
        if (constraints.leaf_ratios.find(split.lhsIndex) == constraints.leaf_ratios.end()) {
          // We need to check, if this sub-section is not splittable, then we should
          // return false
          int i;
          bool splittable = false;
          for (i = 0; i < complements[split.lhsIndex].size(); ++i) {
            if (IsSplittable(left_width, height, split.lhsIndex, complements[split.lhsIndex][i], ratios, constraints, false)) {
              splittable = true;
              break;
            }
          }
          
          if (!splittable) {
            return false;
          }
        }
        
        if (constraints.leaf_ratios.find(split.rhsIndex) == constraints.leaf_ratios.end()) {
          // We need to check, if this sub-section is not splittable, then we should
          // return false
          int i;
          bool splittable = false;
          for (i = 0; i < complements[split.rhsIndex].size(); ++i) {
            if (IsSplittable(right_width, height, split.rhsIndex, complements[split.rhsIndex][i], ratios, constraints, false)) {
              splittable = true;
              break;
            }
          }
          
          if (!splittable) {
            return false;
          }
        }
      }  
    } else {
      const double top_height = height * ratios[ratio_index] / ratios[split.lhsIndex];
      const double bottom_height = height * ratios[ratio_index] / ratios[split.rhsIndex];
      
      assert(top_height + bottom_height - height < 0.000001);
      
      if (top_height < constraints.min_height || bottom_height < constraints.min_height ||
          top_height / width < constraints.min_row_ratio ||
          bottom_height / width < constraints.min_row_ratio) {
        return false;
      }
      
      
      if (recursive) {
        if (constraints.leaf_ratios.find(split.lhsIndex) == constraints.leaf_ratios.end()) {
          // We need to check, if this sub-section is not splittable, then we should
          // return false
          int i;
          bool splittable = false;
          for (i = 0; i < complements[split.lhsIndex].size(); ++i) {
            if (IsSplittable(width, top_height, split.lhsIndex, complements[split.lhsIndex][i], ratios, constraints, false)) {
              splittable = true;
              break;
            }
          }
          
          if (!splittable) {
            return false;
          }
        }
        
        if (constraints.leaf_ratios.find(split.rhsIndex) == constraints.leaf_ratios.end()) {
          // We need to check, if this sub-section is not splittable, then we should
          // return false
          int i;
          bool splittable = false;
          for (i = 0; i < complements[split.rhsIndex].size(); ++i) {
            if (IsSplittable(width, bottom_height, split.rhsIndex, complements[split.rhsIndex][i], ratios, constraints, false)) {
              splittable = true;
              break;
            }
          }
          
          if (!splittable) {
            return false;
          }
        }
      }
    }
    
    return true;
  }


  
  // Splits each leaf of the tree, if the split is successful the new trees
  // are added to the trees array.
  static void splitTree(HambidgeTree* tree, std::vector<HambidgeTree*>* trees,
                        const Ratios& ratios, const HambidgeRectangleConstraints& constraints)
  {
    int i, j;
    const std::vector<std::vector<Split> >& complements = ratios.complements();
    std::vector<HambidgeTreeNode*>* leaves = tree->getLeaves();
    std::vector<HambidgeTreeNode*>::const_iterator it;
    int prune_count = 0;
    
    // Go through each 
    for (it = leaves->begin(), i = 0; it != leaves->end(); ++it, ++i) {
      HambidgeTreeNode* leaf = *it;
      int ratio_index = leaf->getRatioIndex();
      
      int numSplits = complements[ratio_index].size();
      for (j = 0; j < numSplits; j++)
      {
        const Split& split = complements[ratio_index][j];
        if (IsSplittable(leaf->getWidth(), leaf->getHeight(), leaf->getRatioIndex(), split, ratios, constraints, true)) {
          HambidgeTree* split_tree = tree->clone();
          HambidgeTreeNode* split_leaf = split_tree->getLeafAt(i);
          
          assert(split_leaf != 0L);
          
          bool split_result = split_leaf->split(split);
          assert(split_result);
          
          // Make sure this tree doesn't equal any before it
          bool exists = false;
          /*
          std::vector<HambidgeTree*>::iterator tree_iterator;
          for (tree_iterator = trees->begin(); tree_iterator != trees->end(); ++tree_iterator) {
            if (HambidgeTree::equals(*tree_iterator, split_tree)) {
              exists = true;
              break;
            }
          }
          */
          if (!exists) {
            trees->push_back(split_tree);
          } else {
            delete split_tree;
          }

        }
      }
    }
#if 0
        Split split = complements[ratio_index][j];
        if (split.type == SPLIT_VERTICAL)
        {
          assert(fabs(ratios[split.lhsIndex] + ratios[split.rhsIndex] - ratios[ratio_index]) < 0.0000001);
          
          double w = leaf->getWidth();
          double h = leaf->getHeight();
          double width_left = w * ratios[split.lhsIndex] / leaf->getRatio();
          double width_right = w * ratios[split.rhsIndex] / leaf->getRatio();
          
          if (width_left < min_width ||
              width_right < min_width)
          {
            prune_count++;
            continue;
          }
          
          // If it can't be split again and its less than minimum sizes
          if (width_left / h < min_ratio)
          {
            bool valid = false;
            sub_split_count = complements[split.lhsIndex].size();
            for (int k = 0; k < sub_split_count && !valid; ++k) {
              double sub_width_left = width_left
            }
            
            if (!valid) {
              prune_count++;
              continue;
            }
          }
              width_right / h < min_ratio)
          
        }
        else if (split.type == SPLIT_HORIZONTAL)
        {
          assert(fabs(1/ratios[split.lhsIndex] + 1/ratios[split.rhsIndex] - 1/ratios[ratio_index]) < 0.0000001);
          
          double w = leaf->getWidth();
          double h = leaf->getHeight();
          double newHeightLeft = h * leaf->getRatio() / ratios[split.lhsIndex];
          double newHeightRight = h * leaf->getRatio() / ratios[split.rhsIndex];
          
          if (newHeightLeft < min_height ||
              newHeightRight < min_height)// ||
              w / newHeightLeft > max_ratio ||
              w / newHeightRight > max_ratio)
          {
            prune_count++;
            continue;
          }     
        }
        else
        {
          assert(0);
        }

        HambidgeTree* split_tree = tree->clone();
        HambidgeTreeNode* split_leaf = split_tree->getLeafAt(i);
        
        assert(split_leaf != 0L);
        
        bool split_result = split_leaf->split(split);
        assert(split_result);
        
        trees->push_back(split_tree);
      }
    }
#endif
    //fprintf(stderr, "SplitTree: created %d trees, pruned %d trees.", trees->size(), prune_count);

    
    delete leaves;
  }
};



#endif
