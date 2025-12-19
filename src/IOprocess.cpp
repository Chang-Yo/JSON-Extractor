#include "IOprocess.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*<======== 分类器 ========>*/
classfication classify(int argc, char **argv, const string path) {
  if (argc < 4)
    return classfication::other;

  filesystem::path target_path(path);
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

/*<======== 获取忽略属性 ========>*/
vector<string> GetIgnoredProperties(int argc, char **argv) {
  vector<string> ignored_properties;
  for (int index = 4; index < argc; index++)
    ignored_properties.push_back(argv[index]);
  return ignored_properties;
}

/*<======== 生成子模块文件名 ========>*/
string GenerateModuleFilename(const string &origin_filename, int node_count,
                              int module_index,
                              vector<string> &ignored_properties) {
  int dot_pos = origin_filename.find_last_of('.');
  int slash_pos = origin_filename.find_last_of('/');
  string file_base_dir = origin_filename.substr(0, slash_pos + 1);
  string basename =
      origin_filename.substr(slash_pos + 1, dot_pos - slash_pos - 1);
  ostringstream output_filename;
  output_filename << file_base_dir;
  // 如果有忽略属性，在文件名中添加标记
  if (!ignored_properties.empty()) {
    output_filename << "module_" << basename;
    // 添加所有忽略属性，用下划线连接
    for (auto &ignored_property : ignored_properties)
      output_filename << "_" << ignored_property;

    output_filename << "_" << node_count << "_" << module_index << ".json";
  } else {
    output_filename << "module_" << basename << "_" << node_count << "_"
                    << module_index << ".json";
  }

  return output_filename.str();
}

/*<======== 写入文件 ========>*/
void CreateModuleFile(vector<string> &target_id_set, json &j,
                      string &output_file_name) {
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
  cout << "The file was created as: " << output_file_name << endl;
  ofs << setw(4) << output_array;
  cout << "✅ ==>The module file was written successfully!" << endl;
  ofs.close();
  return;
}

/*<======== 打印出邻接表 ========>*/
void PrintGraph(vector<list<string>> &graph) {
  cout << "=== Adj-Graph Sheet ===" << endl;
  for (int i = 0; i < graph.size(); i++) {
    // 获取当前节点的ID
    string current_node_id = id_to_index[i];
    cout << "[" << current_node_id << "] -->";

    // 遍历邻接节点
    for (const auto &neighbor_id : graph[i]) {
      cout << neighbor_id << " -> ";
    }
    cout << "[END]" << endl;
  }
}

void PrintGraph(vector<vector<string>> &all_subgraphs) {
  for (auto &subgraph : all_subgraphs) {
    for (auto &node : subgraph) {
      cout << node << "-->";
    }
  }
  cout << "[END]" << endl;
}

void PrintGraph(vector<vector<int>> &components) {
  for (auto &component : components) {
    for (auto &node_id : component) {
      cout << "[" << id_to_index[node_id] << "]-->";
    }
    cout << "[END]" << endl;
  }
}

void PrintProgressBar(const int current_step, int total_step) {
  double progress = (double)current_step / total_step;
  int complete_blocks = int(progress * 50);
  int remaining_blocks = 50 - complete_blocks;

  string bar;
  bar += "[";
  bar += string(complete_blocks, '#');
  bar += string(remaining_blocks, '.');
  bar += "]";

  cout << "\r" << bar << " Processing Now...";
  cout.flush();
}