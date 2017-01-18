//
//  main.cpp
//  GoldenTest
//
//  Created by Daniel Riley on 1/9/13.
//
//

#include <iostream>
#include "golden/grouper.h"

#include "golden/postscript_writer.h"
#include "golden/json_writer.h"

int main(int argc, const char * argv[])
{

  // insert code here...
  std::cout << "Hello, World!\n";

  golden::Ratios ratios = golden::GoldenRatios::ratios();
  golden::RatioGroup ratio_indexes;
  std::vector<golden::LabelNode*> trees;
  srand(time(NULL));
  
  while (!trees.size()) {
    // How many possibilities with two squares
    for (int i = 0; i < 10; ++i) {
      double num = 0.5 + rand() / (double)RAND_MAX * 2;
      std::cout << "num" << num;
      ratio_indexes.push_back(ratios.FindClosestIndex(num));
    }
    /*
    ratio_indexes.push_back(ratios.FindClosestIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestInverseIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestInverseIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestInverseIndex(golden_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestInverseIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestInverseIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestInverseIndex(root_five_ratio));
    ratio_indexes.push_back(ratios.FindClosestIndex(1.0));
    ratio_indexes.push_back(ratios.FindClosestIndex(1.0));
    ratio_indexes.push_back(ratios.FindClosestIndex(1.0));
    ratio_indexes.push_back(ratios.FindClosestIndex(1.0));
  */

    
    golden::Grouper grouper(ratio_indexes, ratios);
    trees = grouper.Group();
  }
  
  golden::PostscriptWriter writer(ratios);
  writer.Open("/Users/danielriley/Desktop/golden1.ps");
  
  golden::JsonWriter json_writer(ratios);
  json_writer.Open("/Users/danielriley/Desktop/golden1.json");

  for (int i = 0; i < trees.size(); ++i) {
    writer.WriteTree(trees[i]);
    json_writer.WriteTree(trees[i]);
  }
  
  writer.Close();
  json_writer.Close();
  
  for (int i = 0; i < trees.size(); ++i) {
    delete trees[i];
  }

  return 0;
}


