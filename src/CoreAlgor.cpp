#include "CoreAlgor.h"
#include "IOprocess.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <queue>
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
    cerr << "-------------------------------------" << endl
         << " 🔴 🔴 🔴 Cannot open the file: " << file_path << endl
         << "Please check the path!" << endl
         << "-------------------------------------" << endl;
    return;
  }
  json j;
  ifs >> j;

  // 收集所有节点ID，建立ID到索引的映射
  SetMapping(j);

  // 检查一下json文件是否为空
  if (!id_map.size()) {
    cerr << "-------------------------------------" << endl
         << " 🟨 🟨 ==> ATTENTION: The json file is empty !!" << endl
         << "-------------------------------------" << endl;
    return;
  }
  // 最好检查一下是否存在对应的属性，但是我们可以将这一步直接放进
  // Graphicalize()函数中，如果不存在,就直接cerr报错,返回空数组就是了
  vector<list<string>> graph = Graphicalize(j, ignored_properties);

  if (graph.empty()) {
    cerr << "-------------------------------------" << endl
         << " 🔴 🔴 🔴 Graphicalize the json file \"" << file_path
         << "\" failed!!!" << endl
         << "please check the file contents：" << file_path << "!" << endl
         << "-------------------------------------" << endl;
    return;
  }

  // PrintGraph(graph); // 输出邻结表调试检查

  // 为每个目标节点数寻找所有可能的自包含子图
  for (int target_size = down_realm; target_size <= up_realm; target_size++) {
    if (down_realm <= 0 || up_realm > id_map.size()) { // 边界检查
      cerr << "-------------------------------------" << endl
           << " 🔴 🔴 🔴 ERROR: The realm is wrong !!" << endl
           << "-------------------------------------" << endl;
      break;
      return;
    }
    // PrintProgressBar(target_size - down_realm + 1, up_realm - down_realm +
    // 1);
    vector<vector<string>> subgraphs_of_target_size =
        FindSelfContainedSubgraphs(graph, target_size);
    cout << endl;
    int index = 0;
    for (auto &subgraph : subgraphs_of_target_size) {
      string output_file_name = GenerateModuleFilename(
          file_path, target_size, ++index, ignored_properties);
      CreateModuleFile(subgraph, j, output_file_name);
    }

    if (!index) { // 如果没有找到对应大小的子图
      cout << " 🟨 🟨 ===>ATTENTION: We couldn't find thesubgraphs with size "
              "of [ "
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
// 这个函数实现用图状结构来存储JSON文件中的结点，但同时需要考虑忽略的属性
// 如果ignored_properties为空，说明没有需要忽略的属性，这样可以将两种情况合并处理
vector<list<string>> Graphicalize(json &j, vector<string> &ignored_properties) {
  vector<list<string>> graph(id_to_index.size());

  cout << "We have found the " << id_map.size() << " nodes!" << endl;
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

      // 使用新的辅助函数从任意JSON值中提取节点ID
      vector<string> found_ids = ExtractNodeIdsFromJsonValue(it.value());
      for (const string &found_id : found_ids) {
        // 避免自环和重复添加相同节点
        if (found_id != current_id) {
          graph[current_index].push_back(found_id);
        }
      }
    }
  }

  return graph;
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

/*<======== 寻找指定大小的自包含子图 ========>*/
vector<vector<string>>
FindSelfContainedSubgraphs(const vector<list<string>> &graph_str,
                           int target_size) {

  int n = id_to_index.size();

  // Quick Fail: If target size is larger than total nodes
  if (target_size > n)
    return {};

  vector<list<int>> graph = ConvertGraphToIndices(graph_str);
  vector<vector<string>> result;

  // CALL THE NEW ITERATIVE FUNCTION
  GenerateCombinations(n, target_size, graph, result);

  // 此处可以保留result中连通的子图，如果想要获得可以不连通的自包含子图，请注释这一行
  CheckConnected(result, graph_str);
  return result;
}

/*<======== 生成所有组合并检查自包含性 (Iterative & Optimized) ========>*/
// Optimized to avoid recursion and reduce overhead of set creation
void GenerateCombinations(int n, int k, const vector<list<int>> &graph,
                          vector<vector<string>> &result) {
  if (k <= 0 || k > n)
    return;

  // 1. Initialize the first combination: {0, 1, ..., k-1}
  vector<int> indices(k);
  std::iota(indices.begin(), indices.end(), 0);

  // Optimization: Use a bool vector for O(1) lookup instead of unordered_set
  // This avoids the overhead of hashing for every combination check
  vector<bool> is_in_subset(n, false);

  while (true) {
    // A. Mark current nodes in the lookup table
    for (int idx : indices) {
      is_in_subset[idx] = true;
    }

    // B. Check Self-Contained logic inline for performance
    bool self_contained = true;
    for (int u : indices) {
      for (int v : graph[u]) {
        // If neighbor v is NOT in the current subset, it fails
        if (!is_in_subset[v]) {
          self_contained = false;
          break;
        }
      }
      if (!self_contained)
        break;
    }

    // C. If valid, construct the result
    if (self_contained) {
      vector<string> subgraph;
      subgraph.reserve(k);
      for (int idx : indices) {
        subgraph.push_back(id_to_index[idx]);
      }
      result.push_back(std::move(subgraph));
    }

    // D. Cleanup lookup table for next iteration
    // (Only reset the ones we set to true to keep it O(K) instead of O(N))
    for (int idx : indices) {
      is_in_subset[idx] = false;
    }

    // E. Generate Next Combination (Lexicographical)
    // Find the rightmost element that can be incremented
    int i = k - 1;
    while (i >= 0 && indices[i] == n - k + i) {
      i--;
    }

    if (i < 0)
      break; // All combinations generated

    indices[i]++;
    // Reset all subsequent indices
    for (int j = i + 1; j < k; ++j) {
      indices[j] = indices[j - 1] + 1;
    }
  }
}

/*<======== 检查子图连通性并过滤result ========>*/
void CheckConnected(vector<vector<string>> &result,
                    const vector<list<string>> &graph_str) {
  vector<vector<string>> connected_subgraphs;
  for (auto &subgraph : result) {
    if (IsConnected(subgraph, graph_str)) {
      connected_subgraphs.push_back(subgraph);
    }
  }
  result = connected_subgraphs;
}

/*<======== 辅助函数：检查单个子图是否连通（弱连通） ========>*/
bool IsConnected(const vector<string> &subgraph_nodes,
                 const vector<list<string>> &graph_str) {
  if (subgraph_nodes.empty())
    return false;
  int n = subgraph_nodes.size();

  // 构建局部ID到索引的映射
  unordered_map<string, int> local_map;
  for (int i = 0; i < n; ++i) {
    local_map[subgraph_nodes[i]] = i;
  }

  // 构建局部有向图
  vector<list<int>> local_graph(n);
  for (const string &node : subgraph_nodes) {
    int local_idx = local_map[node];
    int global_idx = id_map[node]; // 假设id_map全局可用
    for (const string &neigh : graph_str[global_idx]) {
      auto it = local_map.find(neigh);
      if (it != local_map.end()) {
        local_graph[local_idx].push_back(it->second);
      }
    }
  }

  // 转换为无向图以检查弱连通
  vector<unordered_set<int>> undir(n);
  for (int i = 0; i < n; ++i) {
    for (int j : local_graph[i]) {
      undir[i].insert(j);
      undir[j].insert(i); // 添加反向边
    }
  }

  // BFS检查连通组件
  vector<bool> visited(n, false);
  queue<int> q;
  q.push(0); // 从第一个节点开始
  visited[0] = true;
  int count = 1;

  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int v : undir[u]) {
      if (!visited[v]) {
        visited[v] = true;
        q.push(v);
        count++;
      }
    }
  }

  return count == n; // 如果访问了所有节点，则连通
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
      if (filename.find("module") == 0) {
        continue;
      }

      // 只处理.json文件，防止文件夹中含有其他类型文件
      if (filename.length() >= 5 &&
          filename.substr(filename.length() - 5) == ".json") {
        string file_path = entry.path().string();
        cout << "Processing file: " << filename << endl;
        HandleSingleFile(file_path, down_realm, up_realm, ignored_properties);
      }
    }
  }
}

/*<======== 辅助函数：从JSON值中提取节点ID ========>*/
vector<string> ExtractNodeIdsFromJsonValue(const json &value) {
  vector<string> ids;

  if (value.is_string()) {
    ExtractNodeIdsFromString(value.get<string>(), ids);
  } else if (value.is_array()) {
    ExtractNodeIdsFromArray(value, ids);
  } else if (value.is_object()) {
    ExtractNodeIdsFromObject(value, ids);
  }

  return ids;
}

void ExtractNodeIdsFromString(const string &str, vector<string> &ids) {
  for (const string &node_id : id_to_index) {
    if (str.find(node_id) != string::npos) {
      ids.push_back(node_id);
    }
  }
}

void ExtractNodeIdsFromArray(const json &arr, vector<string> &ids) {
  for (const auto &element : arr) {
    vector<string> element_ids = ExtractNodeIdsFromJsonValue(element);
    ids.insert(ids.end(), element_ids.begin(), element_ids.end());
  }
}

void ExtractNodeIdsFromObject(const json &obj, vector<string> &ids) {
  for (auto it = obj.begin(); it != obj.end(); ++it) {
    vector<string> value_ids = ExtractNodeIdsFromJsonValue(it.value());
    ids.insert(ids.end(), value_ids.begin(), value_ids.end());
  }
}