//
//  geometry.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/10/13.
//
//

#ifndef GOLDEN_GEOMETRY_H
#define GOLDEN_GEOMETRY_H

namespace golden {

class  Box {
public:
  Box(double left, double top, double right, double bottom)
  :left(left),
   top(top),
   right(right),
   bottom(bottom)
  {}
  
  inline double width() const
  {
    return right - left;
  }
  
  inline double height() const
  {
    return bottom - top;
  }
  
  double left;
  double top;
  double right;
  double bottom;
};


}

#endif // GOLDEN_GEOMETRY_H
