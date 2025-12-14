#include "IOprocess.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

classfication classify(int argc, char **argv) {
  if (argc < 4)
    return classfication::other;

  filesystem::path target_path(argv[1]);
  bool with_ignored_property =
      (argc >= 5); // 通过参数的个数来判断是否有需要忽略的属性

  if (filesystem::is_regular_file(target_path))
    return with_ignored_property
               ? classfication::single_file_with_ignored_property
               : classfication::single_file;
  else if (filesystem::is_directory(target_path))
    return with_ignored_property
               ? classfication::directory_with_ignored_property
               : classfication::directory;
  else
    return classfication::other;
}

string GenerateModuleFilename(const string &origin_filename, int node_count,
                              int module_index) {
  int dot_pos = origin_filename.find_last_of('.');
  string basename = origin_filename.substr(0, dot_pos);
  ostringstream output_filename;
  output_filename << "module_" << basename << "_" << node_count << "_"
                  << module_index << ".json";
  return output_filename.str();
}

void CreateModuleFile(vector<string> &target_id_set, json &j,
                      const string output_file_name) {
  if (target_id_set.empty()) {
    cerr << "There is no qualified subgraph" << endl;
    return;
  }
  json output_array = json::array();
  for (int i = 0; i < target_id_set.size(); i++) {
    string target_id = target_id_set[i];
    for (const auto &node : j) {
      if (node["id"] == target_id) {
        output_array.push_back(node);
        break;
      }
    }
  }
  ofstream ofs(output_file_name);
  if (!ofs.is_open()) {
    cerr << "cannot open the file: " << output_file_name << endl;
    return;
  }
  ofs << setw(4) << output_array;
  ofs.close();
  return;
}