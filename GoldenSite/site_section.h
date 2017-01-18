/*! \file site_section.h
    \brief File overview.
    
    Details.
    
    by Dan Riley, 9/3/12.
    Copyright (c) 2012 Scientific Sciences. All rights reserved.
*/

#ifndef GoldenSite_site_section_h
#define GoldenSite_site_section_h

#include <string>

enum SiteSectionField {
  kSiteSectionFieldUnknown,
  kSiteSectionFieldName,
  kSiteSectionFieldDescription,
  kSiteSectionFieldPoster,
  kSiteSectionFieldImportance,
  kSiteSectionFieldGroup,
  kSiteSectionFieldLayout,
  kSiteSectionFieldDimensions,
  kSiteSectionFieldDisabled
};

struct SiteSection {
  std::string name;
  std::string description;
  std::string poster;
  std::string video;
  int row;
  int layout;
  int group;
  int width;
  int height;
  double importance;
  friend std::ostream& operator<<(std::ostream&, const SiteSection&);
};

std::ostream& operator<<(std::ostream &strm, const SiteSection& section) {
  return strm << "site_section [name:" << section.name << ", description:" << section.description << "]";
}





#endif
