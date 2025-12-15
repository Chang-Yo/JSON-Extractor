#include "CoreAlgor.h"
#include "IOprocess.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
using json = nlohmann::json;

vector<string> id_to_index;
unordered_map<string, int> id_map;

/*<======== 核心操作：处理单文件 ========>*/
void HandleSingleFile(string &file_path, int down_realm, int up_realm,
                      vector<string> &ignored_properties) {
  ifstream ifs(file_path);
  if (!ifs.is_open()) {
    cerr << "Cannot open the file: " << file_path << endl
         << "Please check the path!" << endl;
    return;
  }
  json j;
  ifs >> j;

  // 收集所有节点ID，建立ID到索引的映射
  SetMapping(j);

  // 最好检查一下是否存在对应的属性，但是我们可以将这一步直接放进
  // Graphicalize()函数中，如果不存在,就直接cerr报错,返回空数组就是了
  vector<list<string>> graph = Graphicalize(j, ignored_properties);
  if (graph.empty()) {
    cerr << "Graphicalize the json file \"" << file_path << "\" failed!!!"
         << endl
         << "please check the file contents：" << file_path << "!" << endl;
    return;
  }

  // 输出邻结表调试检查
  PrintGraph(graph);

  // 为每个目标节点数寻找所有可能的子图，all_subgraphs包含了所有的**自包含**的子图
  vector<vector<string>> all_subgraphs = FindConnectedGraph(graph);

  for (int target_size = down_realm; target_size <= up_realm; target_size++) {
    if (down_realm <= 0) { // 边界检查
      cerr << "ERROR: The down realm cannot be lower than 1!!" << endl;
      break;
      return;
    }

    int index = 0;
    for (auto &subgraph : all_subgraphs) {
      if (subgraph.size() == target_size) {
        string output_file_name = GenerateModuleFilename(
            file_path, target_size, index++, ignored_properties);
        CreateModuleFile(subgraph, j, output_file_name);
      }
    }

    if (!index) { // 如果没有找到对应大小的子图
      cout << "======>ATTENTION: We couldn't find thesubgraphs with size of [ "
           << target_size << " ] !" << endl;
    }
  }
}

/*<======== 建立 id 映射表 ========>*/
void SetMapping(const json &j) {
  id_to_index.clear();
  id_map.clear();
  // 收集所有节点ID，建立ID到索引的映射
  for (const auto &node : j) {
    if (node.contains("id") && node["id"].is_string()) {
      string id = node["id"];
      id_map[id] = id_to_index.size();
      id_to_index.push_back(id);
    }
  }
}

/*<======== 建立邻接表表 ========>*/
vector<list<string>> Graphicalize(json &j, vector<string> &ignored_properties) {
  // 这个函数实现用图状结构来存储JSON文件中的结点，但同时需要考虑忽略的属性
  // 如果ignored_properties为空，说明没有需要忽略的属性，这样可以将两种情况合并处理
  cout << "We have found the " << id_map.size() << "nodes!" << endl;

  vector<list<string>> graph(id_to_index.size());

  // 将忽略属性转换为集合以便快速查找
  unordered_set<string> ignored_set(ignored_properties.begin(),
                                    ignored_properties.end());

  // 处理每个节点的连接关系
  for (const auto &node : j) {
    string current_id = node["id"];
    int current_index = id_map[current_id];

    for (auto it = node.begin(); it != node.end(); ++it) { // it 代表每一种属性
      string key = it.key();
      if (key == "id" ||
          ignored_set.find(key) !=
              ignored_set.end()) // 如果属性在ignored_set中找到则跳过
        continue;

      if (it.value().is_string()) {
        string value = it.value().get<string>();
        for (auto &other_id : id_to_index) {
          if (value.find(other_id)) {
            // 如果在其他属性中找到了调用的其他节点，就建立联系
            graph[current_index].push_back(other_id);
          }
        }
      }
    }
  }

  return graph;
}

/*<======== DFS遍历 ========>*/
// 计算从节点start可达的所有节点（包括自己）
void DFS(int start, const vector<list<int>> &graph,
         unordered_set<int> &reachable, vector<bool> &visited) {
  if (visited[start])
    return;
  visited[start] = true;
  reachable.insert(start);

  for (int neighbor : graph[start]) {
    DFS(neighbor, graph, reachable, visited);
  }
}

/*<======== 检查是否自包含 ========>*/
bool IsSelfContained(const unordered_set<int> &subset,
                     const vector<list<int>> &graph) {
  for (int node : subset) {
    for (int neighbor : graph[node]) {
      if (subset.find(neighbor) == subset.end()) {
        return false; // 调用了外部节点
      }
    }
  }
  return true;
}

/*<======== 将字符串邻接表转化为整数邻接表 ========>*/
// 化为整数可以提高运行效率
vector<list<int>> ConvertGraphToIndices(const vector<list<string>> &graph_str) {
  vector<list<int>> graph(graph_str.size());
  for (size_t i = 0; i < graph_str.size(); i++) {
    for (const string &neighbor_id : graph_str[i]) {
      auto it = id_map.find(neighbor_id);
      if (it != id_map.end()) {
        graph[i].push_back(it->second);
      }
    }
  }
  return graph;
}

/*<======== 寻找所有自包含子图 ========>*/
vector<vector<string>>
FindConnectedGraph(const vector<list<string>> &graph_str) {
  int n = id_to_index.size();
  vector<list<int>> graph = ConvertGraphToIndices(graph_str);

  // 预计算每个节点的可达集合
  vector<unordered_set<int>> reachable_from(n);
  for (int i = 0; i < n; i++) {
    vector<bool> visited(n, false);
    DFS(i, graph, reachable_from[i], visited);
  }

  // 使用位掩码枚举所有非空子集
  vector<vector<string>> result;
  int total_subsets = (1 << n);

  for (int mask = 1; mask < total_subsets; mask++) {
    unordered_set<int> subset;
    for (int i = 0; i < n; i++) {
      if (mask & (1 << i)) { // 利用汇编的原理
        subset.insert(i);
      }
    }

    // 检查是否自包含
    if (IsSelfContained(subset, graph)) {
      vector<string> subgraph;
      for (int idx : subset) {
        subgraph.push_back(id_to_index[idx]);
      }
      // sort(subgraph.begin(), subgraph.end()); // 可选：排序以便于比较
      result.push_back(subgraph);
    }
  }

  return result;
}

/*<======== 处理文件夹 ========>*/
// 只需要循环调用HandleSingleFile()即可
void HandleDirectory(string &directory_path, int down_realm, int up_realm,
                     vector<string> &ignored_properties) {
  // 在目录下读取每个文件，记录为input_file_path
  // 然后循环调用 HandleSingleFile(input_path,down_realm,up_realm)
  filesystem::path dir_path(directory_path);

  for (const auto &entry : filesystem::directory_iterator(dir_path)) {
    if (entry.is_regular_file()) {
      string filename = entry.path().filename().string();
      // 跳过以'module'开头的文件
      if (filename.find("module")) {
        continue;
      }

      // 只处理.json文件，防止文件夹中含有其他类型文件----------------->但是有点问题
      if (filename.find(".json") == filename.length() - 5) {
        string file_path = entry.path().string();
        cout << "Processing file: " << filename << endl;
        HandleSingleFile(file_path, down_realm, up_realm, ignored_properties);
      }
    }
  }
}
