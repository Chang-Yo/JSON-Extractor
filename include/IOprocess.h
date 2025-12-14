/*
这个头文件用来处理所有的输出输出相关操作，包括：
==> 读取调用参数，实现分类器功能
==> 生成需要输出的文件名称
==> 生成输出文件
*/
#pragma once
#include "json.hpp"
#include <list>
#include <string>
#include <vector>
using namespace std;
using json = nlohmann::json;

// 定义五种类别
enum class classfication {
  single_file,
  single_file_with_ignored_property,
  directory,
  directory_with_ignored_property,
  other
};
// 分类器
classfication classify(int argc, char *argv[], const string path);
// 生成输出文件名
string GenerateModuleFilename(const string &origin_filename, int node_count,
                              int module_index);
string GenerateModuleFilename(const string &origin_filename, int node_count,
                              int module_index,
                              vector<string> &ignored_properties);
// 创建输出文件

void CreateModuleFile(vector<string> &, json &, string &);
void PrintGraph(vector<list<string>> &graph);