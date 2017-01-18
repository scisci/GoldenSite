//
//  json_writer.h
//  GoldenSite
//
//  Created by x on 4/12/15.
//
//

#ifndef GOLDEN_JSON_WRITER_H
#define GOLDEN_JSON_WRITER_H

#include <cstdio>
#include <sstream>
#include <fstream>

#include "json/json.h"

#include "golden/label_node.h"
#include "golden/dimension_iterator.h"
#include "golden/geometry.h"

namespace golden {

class JsonWriter {
public:
  JsonWriter(const Ratios& ratios)
  :ratios_(&ratios),
   trees_(Json::arrayValue),
   scale_(1.0)
  {}
  
  void Open(const char* filename)
  {
    filename_ = filename;
    trees_ = Json::arrayValue;
    
  }
  
  void Close()
  {
    root_["trees"] = trees_;
    std::ofstream of(filename_);
    of << root_;
    of.close();
  }

  void WriteTree(const LabelNode* root)
  {
    DimensionIterator iterator(root, ratios_);
    Box box(0.0, 0.0, 0.0, 0.0);

    Json::Value rects_arr(Json::arrayValue);
    
    while (iterator.HasNext()) {
      const LabelNode* node = iterator.Next(box);
      
      if (node->IsLeaf()) {
        Json::Value rect_obj;

        rect_obj["left"] = box.left * scale_;
        rect_obj["top"] = box.top * scale_;
        rect_obj["right"] = box.right * scale_;
        rect_obj["bottom"] = box.bottom * scale_;

        rects_arr.append(rect_obj);
      }
    }
    
    trees_.append(rects_arr);
  }

private:
  const char* filename_;
  Json::Value root_;
  Json::Value trees_;
  const Ratios* ratios_;
  double scale_;
};


}


#endif // GOLDEN_JSON_WRITER_H

