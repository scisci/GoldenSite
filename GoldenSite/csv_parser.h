/*! \file csv_parser.h
    \brief File overview.
    
    Details.
    
    by Dan Riley, 9/2/12.
    Copyright (c) 2012 Scientific Sciences. All rights reserved.
*/

#ifndef GoldenSite_csv_parser_h
#define GoldenSite_csv_parser_h


#include <istream>
#include <string>
#include <vector>

class CsvParser {
public:
  
  /// <summary>loads a CSV record from the stream is</summary>
  /// <remarks>
  /// * leading and trailing white space is removed outside of
  //    quoted sections when trimWhiteSpace is true
  /// * line breaks are preserved in quoted sections
  /// * quote literals consist of two adjacent quote characters
  /// * quote literals must be in quoted sections
  /// </remarks>
  /// <param name=is>input stream for CSV records</param>
  /// <param name=trimWhiteSpace>trims white space on unquoted fields</param>
  /// <param name=fieldDelim>field delimiter. defaults to ',' for CSV</param>
  /// <param name=recordDelim>record delimiter. defaults to '\n' for CSV</param>
  /// <param name=quote>delimiter for quoted fields. defaults to '"'</param>
  /// <returns>a list of fields in the record</returns>
  std::vector<std::string> ParseLine(std::istream& is,
                                        bool trim_whitespace=true,
                                        const char field_delimiter=',',
                                        const char record_delimiter='\n',
                                        const char quote_character='"')
  {
    std::vector<std::string> record; // result record list. default empty
    std::string field;          // temporary field construction zone
    int start = -1,        // start of a quoted section for trimming
    end = -1;          // end of a quoted section for trimming
    char ch;
    while (is.get(ch))
    {
      if (ch == field_delimiter || ch == record_delimiter) {
        // fieldDelim and recordDelim mark the end of a
        // field. save the field, reset for the next field,
        // and break if there are no more fields
        if (trim_whitespace) {
          // trim all external white space
          // exclude chars between start and end
          const std::string whitespace_list = " \t\n\f\v\r";
          int end_pos, start_pos;
          // order dependency: right trim before let trim
          // left trim will invalidate end's index value
          if ((end_pos = field.find_last_not_of(whitespace_list)) != std::string::npos) {
            // ePos+1 because find_last_not_of stops on white space
            field.erase((end > end_pos) ? end : end_pos + 1);
          }
          
          if ((start_pos = field.find_first_not_of(whitespace_list)) != std::string::npos) {
            field.erase(0, (start != -1 && start < start_pos) ? start : start_pos);
          }
          // reset the quoted section
          start = end = -1;
        }
        // save the new field and reset the temporary
        record.push_back(field);
        field.clear();
        // exit case 1: !is, managed by loop condition
        // exit case 2: recordDelim, managed here
        if (ch == record_delimiter) break;
      } else if (ch == quote_character) {
        // save the start of the quoted section
        start = field.length();
        while (is.get(ch)) {
          if (ch == quote_character) {
            // consecutive quotes are an escaped quote literal
            // only applies in quoted fields
            // 'a""b""c' becomes 'abc'
            // 'a"""b"""c' becomes 'a"b"c'
            // '"a""b""c"' becomes 'a"b"c'
            if (is.peek() != quote_character) {
              // save the end of the quoted section
              end = field.length();
              break;
            } else {
              field.push_back(is.get());
            }
          } else {
            field.push_back(ch);
          }
        }
      } else {
        field.push_back(ch);
      }
    }
    
    return record;
  }

};


#endif
