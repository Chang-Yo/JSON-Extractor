/*
这个头文件包含了处理JSON文件的核心算法，包括：
==> 四种分类处理器
==> 使用图状结构来解析并存储JSON文件
==> 寻找符合要求的强连通子图
*/
#pragma once
#include "IOprocess.h"
#include "json.hpp"
#include <list>
#include <string>
#include <unordered_set>
#include <vector>
using namespace std;
using json = nlohmann::json;
// 四种分类其实可以合并为两类
void HandleSingleFile(string &file_path, int down_realm, int up_realm,
                      vector<string> &);
void HandleDirectory(string &directory_path, int down_realm, int up_realm,
                     vector<string> &);

// 用于构建邻接表
void SetMapping(const json &j);
vector<list<string>> Graphicalize(json &j, vector<string> &ignored_properties);

// 寻找自包含子图时需要的主函数和辅助函数
vector<vector<string>>
FindConnectedGraph(const vector<list<string>> &graph_str);
void DFS(int start, const vector<list<int>> &graph,
         unordered_set<int> &reachable, vector<bool> &visited);
bool IsSelfContained(const unordered_set<int> &subset,
                     const vector<list<int>> &graph);
vector<list<int>> ConvertGraphToIndices(const vector<list<string>> &graph_str);