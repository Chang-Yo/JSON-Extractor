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
#include <vector>
using namespace std;
using json = nlohmann::json;
// 四种处理
void HandleSingleFile(string &file_path, int down_realm, int up_realm);
void HandleDirectory(string &directory_path, int down_realm, int up_realm);
// 考虑到是否含有需要忽略的属性
void HandleSingleFile(string &file_path, int down_realm, int up_realm,
                      char **argv);
void HandleDirectory(string &directory_path, int down_realm, int up_realm,
                     char **argv);

// 数据结构化
vector<list<string>> Graphicalize(json &j);
vector<list<string>> Graphicalize(json &j, vector<string> &ignored_properties);
// 寻找强连通分量
vector<string> FindSubgraph(vector<list<string>> &);