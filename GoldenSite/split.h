#ifndef GoldenSite_split_h
#define GoldenSite_split_h

enum SplitType
{
  SPLIT_NONE,
  SPLIT_HORIZONTAL,
  SPLIT_VERTICAL
};


struct Split
{
  Split()
  :type(SPLIT_NONE),
   lhsIndex(-1),
   rhsIndex(-1)
  {}
  
  Split(SplitType type, int lhsIndex, int rhsIndex)
  :type(type),
   lhsIndex(lhsIndex),
   rhsIndex(rhsIndex)
  {}
  
  SplitType type;
  int lhsIndex;
  int rhsIndex;
};


#endif
