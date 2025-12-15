/*
这个头文件用来处理所有的输出输出相关操作，包括：
==> 读取调用参数，实现分类器功能
==> 生成需要输出的文件名称
==> 生成输出文件
==> 输出图状结构方便调试
==> （测试）在终端输出进度条
*/
#pragma once
#include "json.hpp"
#include <list>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;
using json = nlohmann::json;

// 调用CoreAlgor.cpp中定义的全局变量
extern vector<string> id_to_index;
extern unordered_map<string, int> id_map;

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
// 获取需要忽略的属性
vector<string> GetIgnoredProperties(int argc, char **argv);
// 生成输出文件名和创建输出文件
string GenerateModuleFilename(const string &origin_filename, int node_count,
                              int module_index,
                              vector<string> &ignored_properties);
void CreateModuleFile(vector<string> &, json &, string &);

// 输出图状结构
void PrintGraph(vector<list<string>> &graph);
void PrintGraph(vector<vector<string>> &graph);
void PrintGraph(vector<vector<int>> &graph);

void PrintProgressBar(const int, int);