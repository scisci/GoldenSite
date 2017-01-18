/*! \file site_section_reader.h
    \brief File overview.
    
    Details.
    
    by Dan Riley, 9/3/12.
    Copyright (c) 2012 Scientific Sciences. All rights reserved.
*/

#ifndef GoldenSite_site_section_reader_h
#define GoldenSite_site_section_reader_h

#include <sstream>
#include "site_section.h"
#include "csv_parser.h"

#include "jpeglib.h"
#include "libexif/exif-data.h"
#include "libexif/exif-entry.h"
#include "libexif/exif-ifd.h"
#include "libexif/exif-content.h"

class SiteReader {
public:
  SiteSectionField GetField(const std::string& field_name)
  {
    if (field_name.compare("name") == 0) return kSiteSectionFieldName;
    if (field_name.compare("description") == 0) return kSiteSectionFieldDescription;
    if (field_name.compare("poster") == 0) return kSiteSectionFieldPoster;
    if (field_name.compare("importance") == 0) return kSiteSectionFieldImportance;
    if (field_name.compare("group") == 0) return kSiteSectionFieldGroup;
    if (field_name.compare("layout") == 0) return kSiteSectionFieldLayout;
    if (field_name.compare("dimensions") == 0) return kSiteSectionFieldDimensions;
    if (field_name.compare("disabled") == 0) return kSiteSectionFieldDisabled;

    return kSiteSectionFieldUnknown;
  };
  
  int ReadJpegDimensions(const char* filename, int& width, int& height)
  {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;

    /*
     0th Row      0th Column
     1  top          left side
     2  top          right side
     3  bottom       right side
     4  bottom       left side
     5  left side    top
     6  right side   top
     7  right side   bottom
     8  left side    bottom
     */
    int orientation = 0;
    ExifData *data = exif_data_new_from_file(filename);
    if (data) {
      ExifByteOrder byteOrder = exif_data_get_byte_order(data);
      ExifEntry *exifEntry = exif_data_get_entry(data,
                                                 EXIF_TAG_ORIENTATION);
      if (exifEntry)
        orientation = exif_get_short(exifEntry->data, byteOrder);
      
      exif_data_free(data);
    }

    // Open the file if it exists
    FILE* in_file;
    if ((in_file = fopen(filename, "rb")) == 0L) {
      fprintf(stderr, "can't open %s\n", filename);
      assert(0);
      return -1;
    }

    //initialize error handling
    cinfo.err = jpeg_std_error(&jerr);
    
    // Now we can initialize the JPEG decompression object.
    jpeg_create_decompress(&cinfo);
    
    // Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, in_file);
    
    /* Step 3: read file parameters with jpeg_read_header() */
    
    (void) jpeg_read_header(&cinfo, TRUE);
    width = cinfo.image_width;
    height = cinfo.image_height;
    if (orientation ==8 || orientation == 6) {
      int tmp = width;
      width = height;
      height = tmp;
    }

    fclose(in_file);
    jpeg_destroy_decompress(&cinfo);
    
    return 0;
  }
  
  std::vector<SiteSection> ReadSite(const char* filename)
  {
    CsvParser parser;
    std::vector<SiteSection> sections;
    
    // Read the header line
    std::ifstream ifs(filename);
    
    char path[512];
    path[0] = 0;
    size_t path_length;
    for (path_length = strlen(filename); path_length != 0; --path_length) {
      if (filename[path_length] == '/') {
        strncpy(path, filename, ++path_length);
        break;
      }
    }
    
    
    
    
    int i;
    std::vector<std::string> rec = parser.ParseLine(ifs);
    size_t column_count = rec.size();
    
    // Read the field map
    std::vector<SiteSectionField> field_map(column_count, kSiteSectionFieldUnknown);
    //SiteSectionField* field_map = new SiteSectionField[column_count];
    
    if (column_count > 0) {
      for (i = 0; i < column_count; ++i) {
        field_map[i] = GetField(rec[i]);
      }
      
      int row = 0;
      while (true) {
        rec = parser.ParseLine(ifs);
        if (rec.empty() || rec[0].empty()) break;
        
        SiteSection section;
        // Set some defaults
        section.row = row++;
        section.group = -1;
        section.layout = -1;
        section.importance = 0.5;

        std::istringstream ss;
        size_t extension_pos;
        bool disabled = false;
        
        for (i = 0; i < column_count; ++i) {
          if (rec[i].empty()) continue;
          
          switch (field_map[i]) {
            case kSiteSectionFieldName:
              section.name = rec[i];
              break;
              
            case kSiteSectionFieldDisabled:
              disabled = true;
              break;
              
            case kSiteSectionFieldDescription:
              section.description = rec[i];
              break;
              
            case kSiteSectionFieldPoster:
              section.poster = rec[i];
              
              // Grab extension
              extension_pos = section.poster.find_last_of(".");
              if (strcmp(&section.poster.c_str()[extension_pos + 1], "jpg") == 0 ||
                  strcmp(&section.poster.c_str()[extension_pos + 1], "jpeg") == 0) {
                strcpy(&path[path_length], section.poster.c_str());
                ReadJpegDimensions(path, section.width, section.height);
              } else {
                fprintf(stdout, "Non-jpeg poster: %s.\n", section.poster.c_str());
                // Its probably a video so store the name
                size_t path_pos = section.poster.find_last_of("/");
                section.video.assign(section.poster.begin(), section.poster.begin() + extension_pos);
              }
              break;
              
            case kSiteSectionFieldImportance:
              ss.clear();
              ss.str(rec[i]);
              if (!(ss >> section.importance)) {
                fprintf(stderr, "Failed to extract importance!\n");
              }
              break;
              
            case kSiteSectionFieldGroup:
              ss.clear();
              ss.str(rec[i]);
              if (!(ss >> section.group)) {
                fprintf(stderr, "Failed to extract group!\n");
              }
              break;
              
            case kSiteSectionFieldLayout:
              ss.clear();
              ss.str(rec[i]);
              if (!(ss >> section.layout)) {
                fprintf(stderr, "Failed to extract layout!\n");
              }
              break;
              
            case kSiteSectionFieldDimensions:
              ss.clear();
              ss.str(rec[i]);
              if (!(ss >> section.width) || !(ss >> section.height)) {
                fprintf(stderr, "Failed to extract dimensions!\n");
              }
              break;
              
            case kSiteSectionFieldUnknown:
              break;
          }
        }
        
        if (!disabled) {
          sections.push_back(section);
        
          std::cout << section << std::endl;
          std::cout << std::string(20, '*') << std::endl;
        }
        
      }
    }
    
    return sections;
  }

};

  
#endif
