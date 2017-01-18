//
//  hungarian_assigner.h
//  GoldenSite
//
//  Created by Daniel Riley on 12/23/12.
//
//

#ifndef __GoldenSite__hungarian_assigner__
#define __GoldenSite__hungarian_assigner__

#include "assert.h"
#include <iostream>
#include <vector>
#include <map>
#define INF 100000000    //just infinity

class HungarianAssigner {
public:
  void Initialize(const std::vector<std::vector<int> >& cost_matrix)
  {
    assert(cost_matrix.size() && cost_matrix.size() == cost_matrix[0].size());
    node_count_ = cost_matrix.size();
    cost_matrix_.resize(node_count_ + 1);
    for (int i = 1; i <= node_count_; ++i) {
      cost_matrix_[i].resize(node_count_ + 1);
      for (int j = 1; j <= node_count_; j++) {
        cost_matrix_[i][j] = cost_matrix[i - 1][j - 1];
      }
    }
    
    cost_matrix_ = cost_matrix;
    best_match_.clear();
    cost_ = 0;
    
    assert(cost_matrix_[0].size() == node_count_);
  }
  
  int cost() const
  {
    return cost_;
  }
  
  const std::map<int, int> best_match() const
  {
    return best_match_;
  }
  
  void Solve()
  {
    assert(cost_matrix_.size());
    std::vector<int> u(node_count_ + 1);
    std::vector<int> v(node_count_ + 1);
    std::vector<int> p(node_count_ + 1);
    std::vector<int> way(node_count_ + 1);
    
    for (int i = 1; i <= node_count_; ++i) {
      p[0] = i;
      int j0 = 0;
      std::vector<int> minv(node_count_ + 1, INF);
      std::vector<char> used(node_count_ + 1, false);
      do {
        used[j0] = true;
        int i0 = p[j0], delta = INF, j1;
        for (int j = 1; j <= node_count_; ++j) {
          if (!used[j]) {
            int cur = cost_matrix_[i0][j] - u[i0] - v[j];
            if (cur < minv[j]) {
              minv[j] = cur;
              way[j] = j0;
            }
            if (minv[j] < delta) {
              delta = minv[j];
              j1 = j;
            }
          }
        }
        
        for (int j = 0; j <= node_count_; ++j) {
          if (used[j]) {
            u[p[j]] += delta;
            v[j] -= delta;
          } else {
            minv[j] -= delta;
          }
        }
        
        j0 = j1;
      } while (p[j0] != 0);
      
      do {
        int j1 = way[j0];
        p[j0] = p[j1];
        j0 = j1;
      } while (j0);
    }
    
    
    std::vector<int> ans(node_count_ + 1);
    for (int j = 1; j <= node_count_; ++j) {
      best_match_.insert(std::pair<int, int>(p[j], j));
    }
    
    cost_ = -v[0];
  }
  
private:
  int node_count_;
  std::vector<std::vector<int> > cost_matrix_;
  std::map<int, int> best_match_;
  int cost_;
};

#endif /* defined(__GoldenSite__hungarian_assigner__) */
