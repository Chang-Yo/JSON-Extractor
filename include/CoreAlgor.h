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
FindSelfContainedSubgraphs(const vector<list<string>> &graph_str,
                           int target_size);
bool IsSelfContained(const unordered_set<int> &subset,
                     const vector<list<int>> &graph);
vector<list<int>> ConvertGraphToIndices(const vector<list<string>> &graph_str);
void GenerateCombinations(int n, int k, const vector<list<int>> &graph,
                          vector<vector<string>> &result);

// 检查子图连通性
bool IsConnected(const vector<string> &subgraph_nodes,
                 const vector<list<string>> &graph_str);
void CheckConnected(vector<vector<string>> &result,
                    const vector<list<string>> &graph_str);

/*<======== 辅助函数声明 ========>*/
vector<string> ExtractNodeIdsFromJsonValue(const json &value);
void ExtractNodeIdsFromString(const string &str, vector<string> &ids);
void ExtractNodeIdsFromArray(const json &arr, vector<string> &ids);
void ExtractNodeIdsFromObject(const json &obj, vector<string> &ids);