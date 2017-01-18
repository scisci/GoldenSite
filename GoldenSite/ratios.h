//
//  ratios.h
//  GoldenSite
//
//  Created by Daniel Riley on 5/26/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef GoldenSite_ratios_h
#define GoldenSite_ratios_h

#include <vector>
#include <math.h>
#include <float.h>
#include <algorithm>

#include "assert.h"
#include "split.h"
#include <set>

typedef std::vector<std::vector<Split> > Complements;
typedef std::vector<std::set<int> > Containments;
class Ratios
{
public:
  Ratios(const std::vector<double>& ratios)
  {
    ratios_ = ratios;
    std::sort(ratios_.begin(), ratios_.end());
    BuildComplements();
  }
  
  template<typename T, size_t N>
  static Ratios FromArray(const T (&ratios)[N])
  {
    std::vector<double> ratios_init;
    ratios_init.assign(ratios, ratios + N);
    return Ratios(ratios_init);
  }
  
  inline const double& GetRatio(unsigned int index) const
  {
    assert(index < ratios_.size());
    return ratios_[index];
  }
  
  inline double& operator[] (unsigned int index)
  {
    return ratios_[index];
  }
  
  inline const double& operator[] (unsigned int index) const
  {
    return ratios_[index];
  }
  
  const std::vector<std::vector<Split> >& complements() const
  {
    return complements_;
  }
  
  const std::vector<double>& ratios() const
  {
    return ratios_;
  }
  
  int FindClosestIndex(double ratio) const
  {
    // Returns greater than or equal to ratio
    std::vector<double>::const_iterator it = std::lower_bound(ratios_.begin(), ratios_.end(), ratio);
    if (it == ratios_.begin()) {
      return 0;
    } else if (it == ratios_.end()) {
      return ratios_.size() - 1;
    } else {
      int index = (int)(it - ratios_.begin());
      return (ratio - *(it - 1) <= *it - ratio) ? index - 1 : index;
    }
  }
  
  int FindClosestIndex(double ratio, const std::set<int>& subset) const
  {
    // Returns greater than or equal to ratio
    std::set<int>::const_iterator it;
    
    double closest_distance = DBL_MAX;
    int closest_index = -1;
    
    for (it = subset.begin(); it != subset.end(); ++it) {
      double distance = fabs(ratios_[*it] - ratio);
      if (distance < closest_distance) {
        closest_distance = distance;
        closest_index = *it;
      }
    }
    
    return closest_index;
  }
  
  int FindClosestInverseIndex(double ratio) const
  {
    // Returns greater than or equal to ratio
    return FindClosestIndex(1 / ratio);
  }
  
  inline int FindInverseRatioIndex(int ratio_index) const
  {
    double inverse_ratio, closest_ratio;
    int closest_index;
    
    inverse_ratio = 1 / ratios_[ratio_index];
    closest_index = FindClosestIndex(inverse_ratio);
    closest_ratio = ratios_[closest_index];
    
    if (fabs(ratios_[closest_index] - inverse_ratio) < 0.0000001) {
      return closest_index;
    }
    
    return -1;
  }
  
  
  
private:
  
  //! For each ratio, we need to know the set of ratio pairs that can evenly split
  void BuildComplements()
  {
    int i, j, k;
    const double epsilon = 0.0000001;

    size_t ratio_count = ratios_.size();
    complements_.resize(ratio_count);
    
    for (i = 0; i < ratio_count; ++i) {
      double r = ratios_[i];
      
      // Try to split the width, in the ratio array the height is always considered
      // to be unity
      for (j = 0; j < ratio_count; ++j) {
        if (ratios_[j] < r) {
          for (k = j; k < ratio_count; ++k) {
            if (fabs(r - ratios_[j] - ratios_[k]) < epsilon) {
              complements_[i].push_back(Split(SPLIT_VERTICAL, j, k));
              break;
            }
          }
        }
      }
      
      // Now try to split the height, we need to invert the ratio, since all
      // ratios are setup based on unity height
      r = 1 / ratios_[i];
      
      for (j = 0; j < ratio_count; ++j) {
        if (ratios_[j] < r) {
          for (k = j; k < ratio_count; ++k) {
            if (fabs(r - ratios_[j] - ratios_[k]) < epsilon) {
              // The ratios here won't actually be j and k, they will be the inverses
              // Because they are vertically stacked and height is always
              // considered 1
              int invJ = FindInverseRatioIndex(j);
              int invK = FindInverseRatioIndex(k);
              assert(invJ >= 0 && invK >= 0);
              complements_[i].push_back(Split(SPLIT_HORIZONTAL, invJ, invK));
              break;
            }
          }
        }
      }
    }
  }
  
  // For each ratio we need to know which ratios it can potentially contain.
  // First of all, it can contain anything in its complements array obviously
  void BuildContainments()
  {
    
  }
  
private:
  std::vector<double> ratios_;
  Complements complements_;
  Containments containments_;
};


class GoldenRatios
{
public:
  static double golden_ratio;
  static double square_ratio;
  static double root_five_ratio;
  
  static Ratios ratios()
  {
    return Ratios::FromArray((double []){
      0.125,                          // 0.125 1/2 of 0.25
      1 / (sqrt(5) + 5),    // 0.138 1/2 of 0.276
      2 / (sqrt(5) * 3 + 7),// 0.146 1/2 of 0.2918
      1 / (2 * sqrt(5) + 2),// 0.154 1/2 of 0.309
      sqrt(5) / (4 + 4 * sqrt(5)), //0.172 1/2 of 0.3455
      2 / (9 + sqrt(5)),    // 0.178 1/2 of 0.3559
      1 / (10 - 2 * sqrt(5)),//0.181 1/2 of 0.3618
      1 / (sqrt(5) + 3),    // 0.191 1/2 of 0.382
      1 / (sqrt(5) *4 - 4), // 0.202 1/2 of 0.4045
      sqrt(5) / (2 + 4 * sqrt(5)), //0.204 1/2 of 0.408
      2 / (sqrt(5) + 7),    // 0.216 1/2 of 0.433
      1 / (2 * sqrt(5)),    // 0.2236 1/2 of 0.4472
      0.25,                           // 0.25
      1 / (sqrt(5) * 3 - 3),// 0.2696 1/2 of 0.5393
      
      
      2 / (sqrt(5) + 5),    // 0.2764
      4 / (sqrt(5) * 3 + 7),// 0.2918
      1 / (sqrt(5) + 1),    // 0.309
      sqrt(5) / (2 + 2 * sqrt(5)), // 0.3455
      4 / (9 + sqrt(5)),    // 0.3559
      1 / (5 - sqrt(5)),    // 0.3618
      2 / (sqrt(5) + 3),    // 0.382
      1 / (sqrt(5) * 2 - 2),// 0.4045
      sqrt(5) / (1 + 2 * sqrt(5)), // 0.408
      //2 / (7 - sqrt(5)),    // 0.4198 1/2 of 0.8396
      4 / (sqrt(5) + 7),    // 0.433
      1 / sqrt(5),          // 0.4472
      0.5,                            // 0.5
      2 / (sqrt(5) * 3 - 3),// 0.5393
      4 / (sqrt(5) + 5),    // 0.5528
      //sqrt(5) / 4,          // 0.559 1/2 of 1.118
      //8 / (sqrt(5) * 3 + 7), //0.5835 inverse of 1.17135
      0.875 - sqrt(5) / 8,  // 0.5955 1/2 of 1.191
      2 / (sqrt(5) + 1),    // 0.618
      //sqrt(5) / 8 + 0.375,  // 0.6545 1/2 of 1.309
      sqrt(5) / (1 + sqrt(5)), // 0.691
      //8 / (sqrt(5) + 9),  // 0.712 inverse of 1.4045
      2 / (5 - sqrt(5)),    // 0.7236
      4 / (sqrt(5) + 3),    // 0.764
      1 / (sqrt(5) - 1),    // 0.809
      //(sqrt(5) * 2) / (sqrt(5) * 2 + 1),// 0.817 inverse of 1.2236
      4 / (7 - sqrt(5)),    // 0.8396
      8 / (sqrt(5) + 7),  // 0.866 inverse of 1.1545
      2 / sqrt(5),          // 0.894
      //sqrt(5) / 8 + 0.625,  // 0.9045 1/2 of 1.809
      //0.75 * (sqrt(5) - 1), // 0.927 1/2 of 1.854
      1,                              // 1
      //4 / (sqrt(5) * 3 - 3), // 1.078 inverse of .927
      //8 / (sqrt(5) + 5),  // 1.105 inverse of 0.9045
      sqrt(5) / 2,          // 1.118
      sqrt(5) / 8 + 0.875,  // 1.1545
      1.75 - sqrt(5) / 4,   // 1.191,
      //1 / (sqrt(5) * 2) + 1,// 1.2236 1/2 of 2.4472
      sqrt(5) - 1,          // 1.236
      sqrt(5) / 4 + 0.75,   // 1.309
      2.5 - sqrt(5) / 2,    // 1.382,
      //sqrt(5) / 8 + 1.125,  // 1.4045 1/2 of 2.809
      1 / sqrt(5) + 1,      // 1.4472
      //8 / (sqrt(5) + 3),  // 1.527 inverse of 0.6545
      sqrt(5) / 2 + 0.5,    // 1.618
      8 / (7 - sqrt(5)),  // 1.679 inverse of 0.5955
      //sqrt(5) * 0.375 + 0.875,//1.7135 1/2 of 3.427
      //4 / sqrt(5),          // 1.788 inverse of 0.559
      sqrt(5) / 4 + 1.25,   // 1.809
      1.5 * (sqrt(5) - 1),  // 1.854
      2,                              // 2
      sqrt(5),              // 2.236
      sqrt(5) / 4 + 1.75,   // 2.309
      //(7 - sqrt(5)) / 2,    // 2.382 inverse of 0.4198
      1 / sqrt(5) + 2,      // 2.4472
      sqrt(5) * 2 - 2,      // 2.472
      sqrt(5) / 2 + 1.5,    // 2.618
      5 - sqrt(5),          // 2.764
      sqrt(5) / 4 + 2.25,   // 2.809
      2 / sqrt(5) + 2,      // 2.8944
      sqrt(5) + 1,          // 3.236
      sqrt(5) * 0.75 + 1.75,// 3.427
      sqrt(5) / 2 + 2.5,    // 3.618
      
      
      (sqrt(5) * 3 - 3),    // 3.709 inverse of 0.2696
      4,                               // 4
      (2 * sqrt(5)),        // 4.472 inverse of .2236
      (sqrt(5) + 7) / 2,    // 4.618 inverse of .216
      (2 + 4 * sqrt(5)) / sqrt(5), // 4.894 inverse of 0.204
      (sqrt(5) * 4 - 4), // 4.944 inverse of 0.202
      (sqrt(5) + 3),    // 5.23 inverse of 0.191
      (10 - 2 * sqrt(5)),//5.527 inverse of 0.181
      (9 + sqrt(5))/2,    // 5.618 inverse of 0.178
      (4 + 4 * sqrt(5)) / sqrt(5), //5.788 inverse of 0.172
      (2 * sqrt(5) + 2),// 6.47 inverse of 0.154
      (sqrt(5) * 3 + 7)/2,// 6.854 inverse of 0.146
      (sqrt(5) + 5),    // 7.23 inverse of 0.138
      8
    });
  }
};



#endif
