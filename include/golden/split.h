//
//  split.h
//  GoldenSite
//
//  Created by Daniel Riley on 5/27/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef GOLDEN_SPLIT_H
#define GOLDEN_SPLIT_H

#include "golden/golden.h"

namespace golden {

enum SplitType {
  kSplitTypeNone,
  kSplitTypeHorizontal,
  kSplitTypeVertical
};


struct Split {
  Split()
  :type(kSplitTypeNone),
   left_index(-1),
   right_index(-1)
  {}
  
  Split(SplitType type, RatioIndex left_index, RatioIndex right_index)
  :type(type),
   left_index(left_index),
   right_index(right_index)
  {}
  
  SplitType type;
  RatioIndex left_index;
  RatioIndex right_index;
};

} // namespace golden

#endif // GOLDEN_SPLIT_H

