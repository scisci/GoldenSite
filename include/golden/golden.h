//
//  golden.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/9/13.
//
//

#ifndef GOLDEN_GOLDEN_H
#define GOLDEN_GOLDEN_H

#include <map>
#include <vector>

namespace golden {

typedef int RatioIndex;
typedef std::vector<RatioIndex> RatioGroup;
typedef std::map<RatioIndex, RatioGroup> RatioGroupMap;
typedef std::pair<RatioIndex, RatioGroup> RatioGroupPair;



//! A mapping from a ratio index to all of the labels that have that index
typedef std::map<RatioIndex, std::vector<int> > LabelMap;

const RatioIndex kRatioIndexInvalid = -1;
const double kRatioEpsilon = 0.0000001;

} // namespace golden


#endif // GOLDEN_GOLDEN_H
