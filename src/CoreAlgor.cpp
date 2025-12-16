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
    // PrintProgressBar(target_size-down_realm + 1, up_realm - down_realm+1);
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
  for (int i = 0; i < graph_str.size(); i++) {
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
/**
 * 采用非递归写法生成所有可能的节点组合，并检查自包含性
 * 该算法使用组合数学中的字典序生成方法，逐步生成所有k个元素的组合
 * 对于每个组合，检查是否为自包含子图（所有节点引用都在子图内部）
 *
 * @param n 总节点数量
 * @param k 目标子图大小（节点数量）
 * @param graph 邻接表表示的图结构（使用整数索引）
 * @param result 存储所有有效自包含子图的结果集合
 */
void GenerateCombinations(int n, int k, const vector<list<int>> &graph,
                          vector<vector<string>> &result) {
  if (k <= 0 || k > n)
    return;

  // 初始化集合: {0, 1, ..., k-1}
  vector<int> indices(k);
  std::iota(indices.begin(), indices.end(), 0);

  // 使用bool属性的哈希表，加快查找速率
  vector<bool> is_in_subset(n, false);

  while (true) {
    // 标记当前组合中的所有节点
    for (int idx : indices) {
      is_in_subset[idx] = true;
    }

    // 检查自包含性：所有节点的引用必须在子图内部
    bool self_contained = true;
    for (int u : indices) {
      for (int v : graph[u]) {
        // 如果邻居节点v不在当前子集中，则不满足自包含条件
        if (!is_in_subset[v]) {
          self_contained = false;
          break;
        }
      }
      if (!self_contained)
        break;
    }

    // 如果满足自包含条件，构建子图并添加到结果集
    if (self_contained) {
      vector<string> subgraph;
      subgraph.reserve(k);
      for (int idx : indices) {
        subgraph.push_back(id_to_index[idx]);
      }
      result.push_back(std::move(subgraph));
    }

    // 重置哈希表，只重置被修改的节点
    // 优化：不必每次都重置所有节点，只重置当前组合中的节点
    for (int idx : indices) {
      is_in_subset[idx] = false;
    }

    // 生成下一个组合（字典序）
    // 找到最右边可以递增的元素
    int i = k - 1;
    while (i >= 0 && indices[i] == n - k + i) {
      i--;
    }

    if (i < 0)
      break; // 所有组合已生成完毕

    indices[i]++; // 递增当前位置
    // 重置所有后续索引
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
  // 如果考虑字符串中包含节点ID作为独立单词的话，请将函数定义替换为以下函数
  /*for (const string &node_id : id_to_index) {
    int pos = 0;
    while ((pos = str.find(node_id, pos)) != string::npos) {
      // 检查单词边界：前一个字符不是字母数字，或者在字符串开头
      bool start_boundary = (pos == 0) || !isalnum(str[pos - 1]);
      // 后一个字符不是字母数字，或者在字符串结尾
      bool end_boundary = (pos + node_id.length() == str.length()) ||
                          !isalnum(str[pos + node_id.length()]);

      if (start_boundary && end_boundary) {
        ids.push_back(node_id);
        break; // 每个节点ID只添加一次
      }
      pos += node_id.length(); // 继续搜索下一个匹配
    }
  }*/

  // 如果只对属性的值做精确匹配，不考虑字符串中包含节点ID作为独立单词的话，请将函数定义替换为以下函数
  for (const string &node_id : id_to_index) {
    if (str == node_id) {
      ids.push_back(node_id);
      break;
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
