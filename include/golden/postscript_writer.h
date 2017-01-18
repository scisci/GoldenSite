//
//  postscriptwriter.h
//  GoldenSite
//
//  Created by Daniel Riley on 1/10/13.
//
//

#ifndef GOLDEN_POSTSCRIPT_WRITER_H
#define GOLDEN_POSTSCRIPT_WRITER_H

#include <cstdio>
#include <sstream>

#include "golden/label_node.h"
#include "golden/dimension_iterator.h"
#include "golden/geometry.h"

namespace golden {

class PostscriptWriter {
public:
  PostscriptWriter(const Ratios& ratios)
  :ratios_(&ratios),
   file_(0L),
   scale_(100.0),
   row_width_(10000.0)
  {
  }
  
  void Open(const char* filename)
  {
    assert(file_ == 0L);
    file_ = fopen(filename, "w");
    fprintf(file_, "%%!PS\n");
    fprintf(file_, "%%%%BoundingBox: 0 0 15000 15000\n");
    offset_x_ = 0;
    offset_y_ = 0;
    row_height_ = 0;
  }
  
  void Close()
  {
    assert(file_ != 0L);
    fclose(file_);
    file_ = 0L;
    
  }

  void WriteTree(const LabelNode* root)
  {
    assert(file_ != 0L);
    
    DimensionIterator iterator(root, ratios_);
    Box box(0.0, 0.0, 0.0, 0.0);
    
    double max_x = DBL_MIN;
    double max_y = DBL_MIN;
    std::stringstream ss;
    bool print_label = false;
    
    while (iterator.HasNext()) {
      const LabelNode* node = iterator.Next(box);
      
      if (node->IsLeaf()) {
        if (box.right > max_x) max_x = box.right;
        if (box.bottom > max_y) max_y = box.bottom;
        
        fprintf(file_, "%.3f %.3f newpath moveto\n", box.left * scale_ + offset_x_, box.top * scale_ + offset_y_);
        fprintf(file_, "%.3f %.3f lineto\n", box.right * scale_ + offset_x_, box.top * scale_ + offset_y_);
        fprintf(file_, "%.3f %.3f lineto\n", box.right * scale_ + offset_x_, box.bottom * scale_ + offset_y_);
        fprintf(file_, "%.3f %.3f lineto\n", box.left * scale_ + offset_x_, box.bottom * scale_ + offset_y_);
        fprintf(file_, "%.3f %.3f lineto\n", box.left * scale_ + offset_x_, box.top * scale_ + offset_y_);
        fprintf(file_, "stroke\n");
        
        // Label it
        if (print_label) {
          fprintf(file_, "/Arial findfont\n12 scalefont\nsetfont\nnewpath\n");
          fprintf(file_, "%.3f %.3f moveto", (box.left + box.width() * 0.5) * scale_ + offset_x_, (box.top + box.height() * 0.5) * scale_ + offset_y_);
          
          ss.str("");
          for (int i = 0; i < node->leaf_labels_size(); ++i) {
            if (i != 0) ss << ", ";
            ss << node->leaf_labels()[i];
          }
          
          std::string labels = ss.str();
          
          fprintf(file_, "(%s) dup stringwidth pop 2 div neg 0 rmoveto show\n", labels.c_str());
        }
      }
    }
    
    offset_x_ += max_x * scale_ + 20.0;
    if (row_height_ < max_y * scale_) {
      row_height_ = max_y * scale_;
    }
    
    if (offset_x_ > row_width_) {
      offset_x_ = 0;
      offset_y_ += row_height_ + 20.0;
      row_height_ = 0.0;
    }
  }

private:
  const Ratios* ratios_;
  FILE* file_;
  double scale_;
  double row_width_;
  double offset_x_;
  double offset_y_;
  double row_height_;
};


}


#endif // GOLDEN_POSTSCRIPT_WRITER_H
