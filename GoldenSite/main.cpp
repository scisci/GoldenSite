//
//  main.cpp
//  GoldenSite
//
//  Created by Daniel Riley on 5/25/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <set>

#include <sys/stat.h>

#include "hambidge_tree_utils.h"
#include "hambidge_tree.h"
#include "site_section_reader.h"
#include "cg_jpeg_resizer.h"

#define WRITE_ALL_TREES
#define WRITE_PS_LAYOUTS

typedef std::vector<SiteSection> SiteSections;
typedef std::map<int, SiteSections> LayoutGroups;
typedef std::pair<int, SiteSections> LayoutGroupsPair;
typedef std::vector<Layout> Layouts;

static void CopyFile(const char* source_path, const char* dest_path)
{
  std::ifstream  src(source_path);
  std::ofstream  dst(dest_path);
  dst << src.rdbuf();
}

static std::vector<Layouts> BuildLayouts(const SiteSections& sections, const Ratios& ratios, const HambidgeRectangleConstraints& constraints, const std::vector<LayoutConstraints>& layout_targets);

static void ResizeAndExportSection(FILE* css_file, const SiteSection& section, const char* layout_id, const char* post_fix, const char* working_path, const char* build_path, double max_scale, double min_dim, FILE* detail_file);

int main (int argc, const char * argv[])
{
  const int max_path_size = 256;
  char path[max_path_size];
  std::string layout_id = "golden_layout_id";
  const char* working_path = "/Users/danielriley/Documents/Dan/Portfolio/Golden/";
  const char* golden_path = "/Users/danielriley/Documents/Web/Golden/output/debug/";
  strcpy(path, working_path);
  char* path_end = &path[strlen(working_path)];
  

  
  double default_width = 960;
  LayoutConstraints layout_320;
  layout_320.width = 320;
  layout_320.padding = 0.0;
  layout_320.min_rectangle_width = 75;
  layout_320.min_rectangle_height = 75;
  //layout_320.max_rectangle_width = 320;
  layout_320.max_rectangle_height = 400;
  
  LayoutConstraints layout_480;
  layout_480.width = 480;
  layout_480.padding = 0.0;
  layout_480.min_rectangle_width = 100;
  layout_480.min_rectangle_height = 100;
  layout_480.max_rectangle_width = 480;
  layout_480.max_rectangle_height = 400;
  
  
  LayoutConstraints layout_768;
  layout_768.width = 768;
  layout_768.padding = 1.0;
  layout_768.min_rectangle_width = 100;
  layout_768.min_rectangle_height = 100;
  layout_768.max_rectangle_width = 600;
  layout_768.max_rectangle_height = 800;
  
  LayoutConstraints layout_960;
  layout_960.width = 960;
  layout_960.padding = 2.0;
  layout_960.min_rectangle_width = 175;
  layout_960.min_rectangle_height = 100;
  layout_960.max_rectangle_width = 500;
  layout_960.max_rectangle_height = 600;
  
  LayoutConstraints layout_1280;
  layout_1280.width = 1280;
  layout_1280.padding = 2.0;
  layout_1280.min_rectangle_width = 200;
  layout_1280.min_rectangle_height = 100;
  layout_1280.max_rectangle_width = 650;
  layout_1280.max_rectangle_height = 750;
  
  std::vector<LayoutConstraints> layout_targets;
  layout_targets.push_back(layout_320);
  layout_targets.push_back(layout_480);
  layout_targets.push_back(layout_768);
  layout_targets.push_back(layout_960);
  layout_targets.push_back(layout_1280);
  
  std::vector<int> target_order;
  for (int i = 0; i < layout_targets.size(); ++i) {
    if (layout_targets[i].width == default_width) {
      target_order.insert(target_order.begin(), i);
    } else {
      target_order.push_back(i);
    }
  }
  
  SiteReader reader;
  
  strcpy(path_end, "projects.csv");
  SiteSections sections = reader.ReadSite(path);
  Ratios ratios = GoldenRatios::ratios();
  
  // Here we can store the maximum scale necessary per image, per target. Then
  // when we export media queries based on the device, we look up the various
  // targets that could serve that device and find the maximum image scale and
  // use that to create the image.
  std::vector<std::vector<double> > max_thumb_scale(layout_targets.size(), std::vector<double>(sections.size(), 0.0));
  

  
  // Build width
  /*
  double layout_ratio_index = ratios.FindClosestIndex(1.0);
  double layout_ratio = ratios[layout_ratio_index];
  double build_width = 960;
  double build_height = build_width / layout_ratio;
   */
  HambidgeRectangleConstraints constraints;
  //constraints.min_width = 200 / build_width;
  //constraints.min_height = 200 / build_height;
  //constraints.min_column_ratio = 0.33;
  //constraints.min_row_ratio = 0.33;
  //for (int i = 0; i < ratios.ratios().size(); i++) {
  //  constraints.leaf_ratios.insert(i);
  //}
  
  constraints.leaf_ratios.insert(ratios.FindClosestIndex(GoldenRatios::square_ratio));
  
  constraints.leaf_ratios.insert(ratios.FindClosestIndex(GoldenRatios::golden_ratio));
  constraints.leaf_ratios.insert(ratios.FindClosestInverseIndex(GoldenRatios::golden_ratio));
  
  constraints.leaf_ratios.insert(ratios.FindClosestIndex(GoldenRatios::root_five_ratio));
  constraints.leaf_ratios.insert(ratios.FindClosestInverseIndex(GoldenRatios::root_five_ratio));

  
  // Build layout groups
  
  LayoutGroups layout_groups;

  for (SiteSections::iterator section = sections.begin(); section != sections.end(); ++section) {
    if (section->layout > -1) {
      LayoutGroups::iterator it = layout_groups.find(section->layout);
      if (it == layout_groups.end()) {
        layout_groups.insert(LayoutGroupsPair(section->layout, SiteSections()));
        it = layout_groups.find(section->layout);
      }
      
      it->second.push_back(*section);
    }
  }
  
  fprintf(stdout, "Found %ld layout groups.\n", layout_groups.size());

  // Build each target, then combine later
  std::vector<std::vector<Layouts> > all_layouts;
  
  int layout_permutations = INT_MAX;
  
#ifndef build_layouts
  for (LayoutGroups::iterator it = layout_groups.begin(); it != layout_groups.end(); ++it) {
    std::vector<Layouts> layouts = BuildLayouts(it->second, ratios, constraints, layout_targets);
    
    for (int i = 0; i < layouts.size(); ++i) {
      if (layouts[i].size() < layout_permutations) {
        layout_permutations = (int)layouts[i].size();
        if (layout_permutations == 0) {
          fprintf(stderr, "No possible layouts!\n");
          fprintf(stderr, "Problem with layout target: %d.\n", (int)layout_targets[i].width);
          assert(0);
        }
      }
      
      for (int j = 0; j < layouts[i].size(); ++j) {
        layouts[i][j].OffsetLabels(it->second[0].row);
      }
    }
    
    all_layouts.push_back(layouts);
  }
#endif

  int export_permutations = MIN(layout_permutations, 3);
  
  fprintf(stdout, "Found %d permutations on these layouts.\n", export_permutations);
  
  for (int p = 0; p < export_permutations; ++p) {
    // Make sure file structure is in tact
    char build_path[256];
    strcpy(build_path, working_path);
    sprintf(&build_path[strlen(build_path)], "build_%d/", p + 1);
    char* build_path_end = &build_path[strlen(build_path)];
    struct stat st;

    if (stat(build_path, &st) != 0) {
      mkdir(build_path, 0777);
      
      strcpy(build_path_end, "css/");
      mkdir(build_path, 0777);
      
      strcpy(build_path_end, "sass/");
      mkdir(build_path, 0777);
      
      strcpy(build_path_end, "js/");
      mkdir(build_path, 0777);
      
      strcpy(build_path_end, "images/");
      mkdir(build_path, 0777);
    }

    // Read in the index file
    strcpy(path_end, "index.html");
    std::ifstream index_istream(path);
    strcpy(build_path_end, "index.html");
    std::ofstream index_file(build_path);
    
    std::string line;
    while (std::getline(index_istream, line))
    {
      if (line.find("{% GOLDEN_SITE") != std::string::npos)
      {
        std::map<std::string, std::string> attributes;
        size_t last_pos = 0;
        while (last_pos != std::string::npos) {
          last_pos = line.find("=", last_pos + 1);
          if (last_pos != std::string::npos) {
            size_t key_start = line.find_last_of(" ", last_pos);
            if (key_start != std::string::npos) {
              size_t value_start = line.find("\"", last_pos);
              if (value_start != std::string::npos) {
                size_t value_end = line.find("\"", value_start + 1);
                if (value_end != std::string::npos) {
                  std::string key;
                  std::string value;
                  key.assign(line, key_start + 1, last_pos - key_start - 1);
                  value.assign(line, value_start + 1, value_end - value_start - 1);
                  attributes.insert(std::pair<std::string, std::string>(key, value));
                }
              }
            }
          }
        }
        
        // See if we parsed our layout id
        std::map<std::string, std::string>::iterator attr = attributes.find("id");
        if (attr != attributes.end()) {
          layout_id = attr->second;
        }
        
        // Write out the site here
        index_file << "<div id=\"" << layout_id << "\" class=\"gcontainer\">\n  <div class=\"gaspect\">\n    <ul class=\"glayout\">" << std::endl;

        for (SiteSections::iterator section = sections.begin(); section != sections.end(); ++section) {
          index_file << "      <li class=\"grect grect-" << section->row << "\">" << std::endl;//</li>" << std::endl;
          index_file << "        <div class=\"ginfo\" data-video=\"" << section->video << "\" data-width=\"" << section->width << "\" data-height=\"" << section->height << "\">"<< std::endl;
          index_file << "          <div class=\"gcaption\">" << section->description << "</div>" << std::endl;
          index_file << "          <div class=\"glabel\">" << (section->row + 1) << "/" << sections.size() << "</div>" << std::endl;
          index_file << "        </div>" << std::endl;
          index_file << "      </li>" << std::endl;
        }
        
        index_file << "    </ul>\n  </div>\n</div>" << std::endl;
      } else {
        index_file << line << std::endl;
      }
    }
    
    index_file.close();
    index_istream.close();

    // Copy in the base sass file
    strcpy(build_path_end, "sass/_golden.scss");
    CopyFile("/Users/danielriley/Documents/Web/Golden/output/debug/sass/_golden.scss", build_path);

    strcpy(path_end, "css/layout.scss");
    FILE* css_header_file = fopen(path, "r");
    char copy_buffer[1024];
  
  
    strcpy(build_path_end, "sass/layout.scss");
    FILE* layout_css_file = fopen(build_path, "w");
    
    size_t numr;
    while (feof(css_header_file) == 0) {
      numr = fread(copy_buffer, 1, 1024, css_header_file);
      if (numr <= 0) {
        break;
      }
      fwrite(copy_buffer, 1, numr,layout_css_file);
    }	
    
    fclose(css_header_file);



    fprintf(layout_css_file, "@import \"_golden.scss\";\n\n");
    fprintf(layout_css_file, "@media only screen and (min-width : %dpx) {\n  .gcontainer {\n    width: 90%%;\n    max-width: 1280px;\n  }\n}\n\n", (int)default_width);

#ifdef WRITE_PS_LAYOUTS
    FILE* ps_file = fopen("/Users/danielriley/Documents/Dan/Portfolio/Golden/layouts.ps", "w");
    fprintf(ps_file, "%%!PS\n");
    fprintf(ps_file, "%%%%BoundingBox: 0 0 15000 15000\n");
#endif    
    
    for (int i = 0; i < target_order.size(); ++i) {
      const LayoutConstraints& target = layout_targets[target_order[i]];
      std::vector<double>& max_scales = max_thumb_scale[target_order[i]];
      
      int target_width = (int)target.width;

      fprintf(stdout, "Working on layout %d.\n", target_width);
      fprintf(layout_css_file, "/* Layout for %dpx width */\n", target_width);
      fprintf(layout_css_file, "@import \"_golden_%d.scss\";\n", target_width);
      
      sprintf(build_path_end, "sass/_golden_%d.scss", target_width);
      const char* tab = "";
      
#ifdef WRITE_PS_LAYOUTS
      double offset_x = i * 1000;
      double scale = 1.0;
#endif    

      FILE* css_file = fopen(build_path, "w");
      if (target.width != default_width) {
        if (target_order[i] == 0) {
          tab = "  ";
          fprintf(css_file, "@media only screen and (max-width : %dpx) {\n", (int)layout_targets[target_order[i] + 1].width - 1);
        } else if (target_order[i] == target_order.size() - 1) {
          tab = "  ";
          fprintf(css_file, "@media only screen and (min-width : %dpx) {\n", target_width);
        } else {
          tab = "  ";
          fprintf(css_file, "@media only screen and (min-width : %dpx) and (max-width : %dpx) {\n", target_width, (int)layout_targets[target_order[i] + 1].width - 1);
        }
      }
      
      // We now need to combine the various layout groups into a single layout
      double offset_y = 0.0;
      double max_height = 0.0;
      std::vector<LabelGroupDimension> ordered_dimensions;
      
      for (int j = 0; j < all_layouts.size(); ++j) {
        const Layouts& target_layouts = all_layouts[j][target_order[i]];
        const Layout& target_layout = target_layouts[0];
        
        LayoutDimensionIterator dimension_iterator(&target_layout, &ratios);
        
        while (dimension_iterator.HasNext()) {
          LabelGroupDimension dimension = dimension_iterator.Next();

          
          if (dimension.group->IsLeaf()) {
            const SiteSection& section = sections[dimension.group->leaf_label()];
            dimension.top += offset_y;
            dimension.bottom += offset_y;
            double width = dimension.right - dimension.left;
            double height = dimension.bottom - dimension.top;
            double width_scale = width / section.width;
            double height_scale = height / section.height;
            double scale = MAX(width_scale, height_scale);
            max_scales[section.row] = scale;
            
            bool inserted = false;
            if (dimension.bottom > max_height) {
              max_height = dimension.bottom;
            }
            std::vector<LabelGroupDimension>::iterator it;
            for (it = ordered_dimensions.begin(); it != ordered_dimensions.end(); ++it) {
              if (dimension.bottom <= it->top ||
                  (dimension.top < it->bottom &&
                   dimension.left < it->left)) {
                    ordered_dimensions.insert(it, dimension);
                    inserted = true;
                    break;
                  }
            }
            
            if (!inserted) {
              ordered_dimensions.push_back(dimension);
            }
          }
        }
        
        offset_y = max_height;
        fprintf(stdout, "Increasing layout height to: %f.\n", offset_y);
      }
      
      // All layouts for this target should now be in ordered_dimensions and
      // we should now the height of the whole area in max_height
      
      for (int j = 0; j < ordered_dimensions.size(); ++j) {
        const LabelGroupDimension& dimension = ordered_dimensions[j];

        int label = dimension.group->leaf_label();
        
#ifdef WRITE_PS_LAYOUTS
        fprintf(ps_file, "%.3f %.3f newpath moveto\n", (dimension.left) * scale + offset_x, (dimension.top) * scale);
        fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.right) * scale + offset_x, (dimension.top) * scale);
        fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.right) * scale + offset_x, (dimension.bottom) * scale);
        fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.left) * scale + offset_x, (dimension.bottom ) * scale);
        fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.left) * scale + offset_x, (dimension.top) * scale);
        fprintf(ps_file, "stroke\n");
#endif
        
        fprintf(css_file, "%s#%s .grect-%d {\n", tab, layout_id.c_str(), label);

        fprintf(css_file, "%s  left: %.5f%%;\n    top: %.5f%%;\n", tab,
            100.0 * dimension.left / target_width, 100.0 * dimension.top / max_height);
        fprintf(css_file, "%s  right: %.5f%%;\n    bottom: %.5f%%;\n", tab,
            100.0 * (target_width - dimension.right) / target_width, 100.0 * (max_height - dimension.bottom) / max_height);
        
        fprintf(css_file, "%s  z-index: %d;\n", tab, 500 + j);
        fprintf(css_file, "%s}\n\n", tab);
      }

      // Write out the aspect ratio for smooth fit
      fprintf(css_file, "%s#%s .gaspect {\n    padding-top: %.5f%%;\n  }\n\n", tab, layout_id.c_str(), 100.0 * max_height / target_width);
      
      // Close out the media query
      if (target.width != default_width) {
        fprintf(css_file, "}\n");
      }
      
      fclose(css_file);
    }
    
#ifdef WRITE_PS_LAYOUTS
    fclose(ps_file);
#endif
    
    //int mobile_break = 768;
    
    const char* retina_media_query = "@media \
    only screen and (-webkit-min-device-pixel-ratio: 2),\n\
    only screen and (   min--moz-device-pixel-ratio: 2),\n\
    only screen and (   -moz-min-device-pixel-ratio: 2),\n\
    only screen and (     -o-min-device-pixel-ratio: 2/1),\n\
    only screen and (    -ms-min-device-pixel-ratio: 2),\n\
    only screen and (        min-device-pixel-ratio: 2),\n\
    only screen and (                min-resolution: 192dpi),\n\
    only screen and (                min-resolution: 2dppx)";
    
    // This should give us every combination that is and below 320/480, mainly the iphone
    const char* retina_media_query_480 = "@media \
    only screen and (-webkit-min-device-pixel-ratio: 2)      and (max-device-width: 568px),\n\
    only screen and (   min--moz-device-pixel-ratio: 2)      and (max-device-width: 568px),\n\
    only screen and (   -moz-min-device-pixel-ratio: 2)      and (max-device-width: 568px),\n\
    only screen and (     -o-min-device-pixel-ratio: 2/1)    and (max-device-width: 568px),\n\
    only screen and (    -ms-min-device-pixel-ratio: 2)      and (max-device-width: 568px),\n\
    only screen and (        min-device-pixel-ratio: 2)      and (max-device-width: 568px),\n\
    only screen and (                min-resolution: 192dpi) and (max-device-width: 568px),\n\
    only screen and (                min-resolution: 2dppx)  and (max-device-width: 568px)\n";
    
    // In layout.scss we need to import the image loaders
    fprintf(layout_css_file, "/* Images for desktop widths */\n");
    
    // By default desktop resolution images are loaded
    fprintf(layout_css_file, "@import \"_golden_images_desktop.scss\";\n");
    
    // If its a retina display make sure to load images twice as big.
    fprintf(layout_css_file, "%s {\n", retina_media_query);
    fprintf(layout_css_file, "@import \"_golden_images_desktop@2x.scss\";\n");
    fprintf(layout_css_file, "}\n\n");
    
    // Include the mobile within the media query
    fprintf(layout_css_file, "@media only screen and (max-device-width: 568px) {\n");
    fprintf(layout_css_file, "@import \"_golden_images_480.scss\";\n");
    fprintf(layout_css_file, "}\n\n");
    
    // If its a retina display make sure to load images twice as big.
    fprintf(layout_css_file, "%s {\n", retina_media_query_480);
    fprintf(layout_css_file, "@import \"_golden_images_480@2x.scss\";\n");
    fprintf(layout_css_file, "}\n\n");
    
    sprintf(build_path_end, "sass/_golden_images_desktop.scss");
    FILE* css_file = fopen(build_path, "w");
    sprintf(build_path_end, "sass/_golden_images_desktop@2x.scss");
    FILE* css_retina_file = fopen(build_path, "w");
    sprintf(build_path_end, "sass/_golden_images_480.scss");
    FILE* css_mobile_file = fopen(build_path, "w");
    sprintf(build_path_end, "sass/_golden_images_480@2x.scss");
    FILE* css_mobile_retina_file = fopen(build_path, "w");


    // Find maximum scale per image for targets greater than or equal to 768
    for (SiteSections::iterator section = sections.begin(); section != sections.end(); ++section) {
      // Check if its a jpeg
      size_t extension_pos = section->poster.find_last_of(".");
      if (extension_pos == std::string::npos || (strcmp(&section->poster.c_str()[extension_pos], ".jpg") != 0 && strcmp(&section->poster.c_str()[extension_pos], ".jpeg") != 0)) {
        fprintf(stdout, "Non-jpeg, probably video, not printing css:%s\n", section->poster.c_str());
        continue;
      }
      
      double max_scale = 0.0;
      double max_scale_480 = 0.0;
      for (int i = 0; i < layout_targets.size(); ++i) {
        double scale = max_thumb_scale[i][section->row];
        
        if (layout_targets[i].width <= 480.0) {
          double mobile_scale = layout_targets[i].width < 480.0 ?
          scale * 480.0 / layout_targets[i].width : scale;

          if (mobile_scale > max_scale_480) {
            max_scale_480 = mobile_scale;
          }
        }
        
        // Increase scale since the whole block can be stretched to the next level
        
        if (i < layout_targets.size() - 1) {
          scale = scale * layout_targets[i + 1].width / layout_targets[i].width;
        }
        
        if (scale > max_scale) {
          max_scale = scale;
        }
      }
      
      // The scale could potentially go up to the next level as well
      
      *build_path_end = 0;
      ResizeAndExportSection(css_file, *section, layout_id.c_str(), "desktop", working_path, build_path, max_scale, 768.0, 0L);
      ResizeAndExportSection(css_retina_file, *section, layout_id.c_str(), "desktop@2x", working_path, build_path, max_scale * 1.5, 1152.0, css_file);
      ResizeAndExportSection(css_mobile_file, *section, layout_id.c_str(), "480", working_path, build_path, max_scale_480, 480.0, 0L);
      ResizeAndExportSection(css_mobile_retina_file, *section, layout_id.c_str(), "480@2x", working_path, build_path, max_scale_480 * 1.5, 720.0, css_mobile_file);

    }

    fclose(css_file);
    fclose(css_retina_file);
    fclose(css_mobile_file);
    fclose(css_mobile_retina_file);
      
    
    // Try resizing an image
    /*
     const char* poster_path = section->poster.c_str();
     size_t pos = section->poster.find_last_of("/") + 1;
     strcpy(path_end, "images/");
     strcat(path_end, &poster_path[pos]);
     std::string src = path;
     strcpy(path_end, "build/images/");
     strcat(path_end, &poster_path[pos]);
     std::string dst = path;
     
     CGImageUtils::Resize(src.c_str(), dst.c_str(), (int)round(section->width * 0.5), (int)round(section->height * 0.5), 0.85);
     */


    fclose(layout_css_file);
    // Build the sass command
    *build_path_end = 0;
    std::stringstream ss;
    ss << "/Users/danielriley/.gem/ruby/1.8/bin/sass " << build_path << "sass/layout.scss " << build_path << "css/layout.css";
    std::string sass_command = ss.str();
    system(sass_command.c_str());
  }
    

  
  // One set of images for 768 - 
        
    
  
  
  /*
   FILE* ps_file = fopen("/Users/danielriley/Documents/Dan/Portfolio/Golden/layouts.ps", "w");
   fprintf(ps_file, "%%!PS\n");
   fprintf(ps_file, "%%%%BoundingBox: 0 0 15000 15000\n");
   double offset_x = 0.0;
   
   for (int i = 0; i < layouts.size(); ++i) {
   layouts[i].Sort();
   LayoutDimensionIterator dimension_iterator(&layouts[i], &ratios);
   
   while (dimension_iterator.HasNext()) {
   LabelGroupDimension dimension = dimension_iterator.Next();
   if (dimension.group->IsLeaf()) {
   fprintf(ps_file, "%.3f %.3f newpath moveto\n", offset_x + dimension.x , dimension.y);
   fprintf(ps_file, "%.3f %.3f lineto\n", offset_x + dimension.x + dimension.width , dimension.y);
   fprintf(ps_file, "%.3f %.3f lineto\n", offset_x + dimension.x + dimension.width, dimension.y + dimension.height);
   fprintf(ps_file, "%.3f %.3f lineto\n", offset_x + dimension.x, dimension.y + dimension.height);
   fprintf(ps_file, "%.3f %.3f lineto\n", offset_x + dimension.x, dimension.y);
   fprintf(ps_file, "stroke\n");
   fprintf(ps_file, "/Arial findfont\n12 scalefont\nsetfont\nnewpath\n");
   fprintf(ps_file, "%.3f %.3f moveto", offset_x + dimension.x + dimension.width * 0.5, dimension.y + dimension.height * 0.5);
   fprintf(ps_file, "(%d) dup stringwidth pop 2 div neg 0 rmoveto show\n", dimension.group->leaf_label());
   }
   }
   
   offset_x += layouts[i].width() + 20;
   }
   
   fclose(ps_file);
   */
}

static void ResizeAndExportSection(FILE* css_file, const SiteSection& section, const char* layout_id, const char* post_fix, const char* working_path, const char* build_path, double max_scale, double min_dim, FILE* detail_file)
{
  size_t filename_pos = section.poster.find_last_of("/") + 1;
  size_t ext_pos = section.poster.find_last_of(".");
  std::string filename;
  std::string extension;
  std::stringstream ss;
  filename.assign(section.poster, filename_pos, ext_pos - filename_pos);
  extension.assign(section.poster, ext_pos, section.poster.size() - ext_pos);
  
  ss.str("");
  ss << working_path << "images/" << filename << extension;
  std::string source_path = ss.str();
  
  ss.str("");
  ss << build_path << "images/" << filename << "_" << post_fix << extension;
  std::string dest_path = ss.str();
  
  double scale = max_scale;
  if (section.width < section.height) {
    if (section.height * scale < min_dim) {
      scale = min_dim / section.height;
    }
  } else {
    if (section.width * scale < min_dim) {
      scale = min_dim / section.width;
    }
  }
  
  CGImageUtils::Resize(source_path.c_str(), dest_path.c_str(), (int)ceil(section.width * scale), (int)ceil(section.height * scale), 0.85);
  
  // Write it out to the css file
  fprintf(css_file, "#%s .grect-%d.no-js, #%s .grect-%d.gactive {\n  background-image: url(../images/%s_%s%s);\n}\n\n", layout_id, section.row, layout_id, section.row, filename.c_str(), post_fix, extension.c_str());
  
  if (detail_file != 0L) {
    fprintf(detail_file, ".gdetail-%d {\n  background-image: url(../images/%s_%s%s);\n}\n\n", section.row, filename.c_str(), post_fix, extension.c_str());
  }
}

static std::vector<Layouts> BuildLayouts(const SiteSections& sections,
    const Ratios& ratios, const HambidgeRectangleConstraints& constraints,
    const std::vector<LayoutConstraints>& layout_targets)
{
  std::vector<HambidgeRectangle> rectangles;
  std::vector<LabelGroup *> all_groups;

#ifdef NO_AUTO_SECTION
  std::map<int, std::vector<int> > section_groups;
  
  for (int i = 0; i < sections.size(); ++i) {
    if (sections[i].group > -1) {
      section_group_it = section_groups.find(sections[i].group);
      if (section_group_it == section_groups.end()) {
        section_groups.insert(std::pair<int, std::vector<int> >(sections[i].group, std::vector<int>()));
        section_group_it = section_groups.find(sections[i].group);
      }
      
      section_group_it->second.push_back(i);
    }
  }
  // Build groups based on the section groups
  std::map<int, std::vector<int> >::iterator section_group_it;
  std::map<int, std::vector<int> > section_groups;
  
  for (int i = 0; i < sections.size(); ++i) {
    if (sections[i].group > -1) {
      section_group_it = section_groups.find(sections[i].group);
      if (section_group_it == section_groups.end()) {
        section_groups.insert(std::pair<int, std::vector<int> >(sections[i].group, std::vector<int>()));
        section_group_it = section_groups.find(sections[i].group);
      }
      
      section_group_it->second.push_back(i);
    }
  }
  
  
  
  // Process each group
  for (section_group_it = section_groups.begin(); section_group_it != section_groups.end(); ++section_group_it) {
    fprintf(stdout, "Attempting to find groups for section group %d.\n", section_group_it->first);
    const std::vector<int> group_indices = section_group_it->second;
    
    for (int i = 0; i < group_indices.size(); ++i) {
      for (int j = 3; j < group_indices.size() - i; ++j) {
        std::vector<LabelGroup *> groups;
        rectangles.clear();
        
        for (int k = 0; k < j; ++k) {
          const SiteSection& section = sections[group_indices[k]];
          
          HambidgeRectangle rectangle;
          rectangle.ratio = (double)section.width / section.height;
          rectangle.importance = section.importance;
          rectangles.push_back(rectangle);
        }
        
        groups = HambidgeTreeUtils::CreateBaseGroups(rectangles, ratios, constraints, 0.5);
        
        if (groups.empty()) {
          fprintf(stdout, "Grouping failed, iterating...\n");
        } else {
          fprintf(stdout, "Grouping succeeded.\n");
          std::vector<LabelGroup *>::iterator group_it;
          
          for (group_it = groups.begin(); group_it != groups.end(); ++group_it) {
            LabelGroup* group = *group_it;
            group->OffsetLabels(group_indices[0]);
            // TODO: don't delete this we'll want to save it
            all_groups.push_back(group);
          }
        }

      }
    }
  }


#else // AUTO_SECTION
 
  // Draw the group somehow
  // Open a file for writing
#ifdef WRITE_ALL_TREES
  FILE* ps_file = fopen("/Users/danielriley/Documents/Dan/Portfolio/Golden/output.ps", "w");
  fprintf(ps_file, "%%!PS\n");
  fprintf(ps_file, "%%%%BoundingBox: 0 0 15000 15000\n");
#endif


  int i, j, section;
  int group_count = 0;
  const int max_section_size = 8;
  const int min_section_size = 2;
  
  
  
  for (section = 0; section < sections.size() - min_section_size; ++section) {
    fprintf(stdout, "Working on section %d\n", section);

    int section_size = section + max_section_size > sections.size() ? sections.size() - section : max_section_size;
    std::vector<LabelGroup *> groups;
    
    for (i = section_size; i >= min_section_size; --i) {
      fprintf(stdout, "Attempting to find groups for %d through %d\n", section, section + i);
      rectangles.clear();
      
      // Gather all rectangles needed for the section.
      for (j = section; j < section + i; ++j) {
        HambidgeRectangle rectangle;
        rectangle.ratio = (double)sections[j].width / sections[j].height;
        rectangle.importance = sections[j].importance;
        rectangles.push_back(rectangle);
      }
      
      groups = HambidgeTreeUtils::CreateBaseGroups(rectangles, ratios, constraints, 0.25);
      
      if (groups.empty()) {
        fprintf(stdout, "Grouping failed, iterating...\n");
      } else {
        fprintf(stdout, "Grouping succeeded.\n");

        std::vector<LabelGroup *>::iterator group_it;
#ifdef WRITE_ALL_TREES  
        double offset_x = 0;
        double offset_y = group_count * 1.1;
        double scale = 100.0;
        fprintf(ps_file, "\n\n");
        fprintf(ps_file, "%%%%%%%%%%%%%%%%%%%%%%%% Group %d %%%%%%%%%%%%%%%%%%%%%%%%%%\n\n", group_count);
#endif
        
        for (group_it = groups.begin(); group_it != groups.end(); ++group_it) {
          LabelGroup* group = *group_it;
          
          // We want to map the labels to global
          //fprintf(stdout, "Map labels from ");
          //group->PrintLabels();
          group->OffsetLabels(section);
          //fprintf(stdout, " to ");
          //group->PrintLabels();
          //fprintf(stdout, "\n");
          
          //group->Print();
          
          //fprintf(ps_file, "\n\n");
          //fprintf(ps_file, "%%%%%%%%%%%%%%%%%%%%%%%% Tree %d %%%%%%%%%%%%%%%%%%%%%%%%%%\n\n", i);
#ifdef WRITE_ALL_TREES          
          LabelGroupDimensionIterator iterator(group, &ratios);
          
          while (iterator.HasNext()) {
            LabelGroupDimension dimension = iterator.Next();
            if (dimension.group->IsLeaf() || dimension.group == group) {
              fprintf(ps_file, "%.3f %.3f newpath moveto\n", (dimension.left + offset_x) * scale, (dimension.top + offset_y) * scale);
              fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.right + offset_x) * scale, (dimension.top + offset_y) * scale);
              fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.right + offset_x) * scale, (dimension.bottom + offset_y) * scale);
              fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.left + offset_x) * scale, (dimension.bottom + offset_y) * scale);
              fprintf(ps_file, "%.3f %.3f lineto\n", (dimension.left + offset_x) * scale, (dimension.top + offset_y) * scale);
              fprintf(ps_file, "stroke\n");
              
              // Label it
              if (dimension.group->IsLeaf()) {
                fprintf(ps_file, "/Arial findfont\n12 scalefont\nsetfont\nnewpath\n");
                fprintf(ps_file, "%.3f %.3f moveto", (dimension.left + offset_x + (dimension.right - dimension.left) * 0.5) * scale, (dimension.top + offset_y + (dimension.bottom - dimension.top) * 0.5) * scale);
                fprintf(ps_file, "(%c) dup stringwidth pop 2 div neg 0 rmoveto show\n", dimension.group->leaf_label() + 65);
              } else {
                fprintf(ps_file, "/Arial findfont\n14 scalefont\nsetfont\nnewpath\n");
                fprintf(ps_file, "%.3f %.3f moveto", (dimension.left + offset_x + (dimension.right - dimension.left) * 0.5) * scale, (dimension.top + offset_y + (dimension.bottom - dimension.top) * 0.5) * scale);
                fprintf(ps_file, "(%d) dup stringwidth pop 2 div neg 0 rmoveto show\n", i);
              }
            }
          }
          
          fprintf(ps_file, "\n\n");
          
          offset_x += ratios[group->ratio_index()] + 0.1;
#endif
          // TODO: don't delete this we'll want to save it
          all_groups.push_back(group);
        }
        
        
        group_count++;
      }
    }
  }
  
#ifdef WRITE_ALL_TREES
  fclose(ps_file);
#endif
  
#endif // AUTO_SECTION
  
  
  // Now we have all groups try to recombine them
  fprintf(stdout, "Gathered %ld groups.\n", all_groups.size());
  
  for (int i = 0; i < all_groups.size(); ++i) {
    all_groups[i]->Print();
    fprintf(stdout, " labels:");
    all_groups[i]->PrintLabels();
    fprintf(stdout, "\n");
  }
  
  
  ComplementsMap complements_rmap = HambidgeTreeUtils::BuildComplementsMap(ratios.complements());
  
  rectangles.clear();
  
  for (int j = 0; j < sections.size(); ++j) {
    HambidgeRectangle rectangle;
    rectangle.ratio = (double)sections[j].width / sections[j].height;
    rectangle.importance = sections[j].importance;
    rectangles.push_back(rectangle);
  }
  
  fprintf(stdout, "After first grouping:%ld groups.\n", all_groups.size());
  size_t all_groups_size = all_groups.size();
  int error = -1;
  double min_cost = 0.25;
  do {
    fprintf(stdout, "Iterating grouping:%ld groups.\n", all_groups.size());
    all_groups = HambidgeTreeUtils::Group(all_groups, rectangles, ratios, complements_rmap, min_cost, false, error);
    if (error < 0 || all_groups.size() > 10000) {
      min_cost *= 0.65;
      fprintf(stdout, "Minimizing cost -> %f.\n", min_cost);
      
      // This is terrible but need to do it quick.
      // The group function pushes items to the front of all_groups, those items
      // are now trash so delete them and move the back back to the front.
      // then resize the vector.
      for (int i = 0; i < all_groups.size() - all_groups_size; ++i) {
        delete all_groups[i];
      }
      
      for (int i = 0; i < all_groups_size; ++i) {
        all_groups[i] = all_groups[i + all_groups.size() - all_groups_size];
      }
      
      all_groups.resize(all_groups_size);
    }
  } while (error < 0);

  std::sort(all_groups.begin(), all_groups.end(), Layout::SortGroupsByRange);
  
  fprintf(stdout, "After second grouping:%ld groups.\n", all_groups.size());
  
  int total = 0;
  
  /*
  for (int i = 0; i < all_groups.size(); ++i) {
    all_groups[i]->Print();
    fprintf(stdout, "\n");
  }
   */

  
  /*
  for (target = layout_targets.begin(); target != layout_targets.end(); ++target) {
    if (target->width == default_width) {
      if (target != layout_targets.begin()) {
        LayoutConstraints default_target = *target;
        layout_targets.erase(target);
        layout_targets.insert(layout_targets.begin(), default_target);
      }
      break;
    }
  }
   */


  /*
  double transition_duration = 0.35;
  strcpy(path_end, "build/sass/golden_.scss");
  FILE* base_css_file = fopen(path, "w");
  fprintf(base_css_file, ".grect {\n");
  fprintf(base_css_file, "  position: absolute;\n");
  fprintf(base_css_file, "  background-size: cover;\n");
  fprintf(base_css_file, "  border-left: 1px solid #fff;\n");
  fprintf(base_css_file, "  border-top: 1px solid #fff;\n");
  fprintf(base_css_file, "  -webkit-transition: left %.2fs, top %.2fs, width %.2fs, height %.2fs;\n", transition_duration, transition_duration, transition_duration, transition_duration);
  fprintf(base_css_file, "  -moz-transition: left %.2fs, top %.2fs, width %.2fs, height %.2fs;\n", transition_duration, transition_duration, transition_duration, transition_duration);
  fprintf(base_css_file, "  -o-transition: left %.2fs, top %.2fs, width %.2fs, height %.2fs;\n", transition_duration, transition_duration, transition_duration, transition_duration);
  fprintf(base_css_file, "  -ms-transition: left %.2fs, top %.2fs, width %.2fs, height %.2fs;\n", transition_duration, transition_duration, transition_duration, transition_duration);
  fprintf(base_css_file, "  transition: left %.2fs, top %.2fs, width %.2fs, height %.2fs;\n", transition_duration, transition_duration, transition_duration, transition_duration);
  fprintf(base_css_file, "}\n\n");
  
  fprintf(base_css_file, ".glayout {\n");
  fprintf(base_css_file, "  position: absolute;\n");
  fprintf(base_css_file, "  left: 0; top: 0; right: 0; bottom: 0;\n");
  fprintf(base_css_file, "  padding: 0;\n");
  fprintf(base_css_file, "  list-style-type: none;\n");
  fprintf(base_css_file, "}\n\n");
  
  fprintf(base_css_file, ".gcontainer {\n");
  fprintf(base_css_file, "  display: inline-block;\n");
  fprintf(base_css_file, "  position: relative;\n");
  fprintf(base_css_file, "  width: 100%%;\n");
  fprintf(base_css_file, "}\n\n");
  
  fclose(base_css_file);
   */
  
 
  std::vector<Layouts> layouts;
  
  for (std::vector<LayoutConstraints>::const_iterator target = layout_targets.begin(); target != layout_targets.end(); ++target) {
    int target_width = (int)target->width;
    
    fprintf(stdout, "Building layout for %d.\n", target_width);
    
    std::vector<Layout> target_layouts = HambidgeTreeUtils::BuildLayout(all_groups, rectangles, ratios, *target, 0.25);
    for (int i = 0; i < target_layouts.size(); ++i) {
      target_layouts[i].Sort();
    }
    
    layouts.push_back(target_layouts);
  }

  for (int i = 0; i < all_groups.size(); ++i) {
    delete all_groups[i];
  }
  
  return layouts;

  
}

