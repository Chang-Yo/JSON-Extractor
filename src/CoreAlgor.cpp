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
void HandleDirectory(string &directory_path, int down_realm, int up_realm) {
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

      // 只处理.json文件，防止文件夹中含有其他类型文件----------------->但是有点问题
      if (filename.find(".json") == filename.length() - 5) {
        string file_path = entry.path().string();
        cout << "Processing file: " << filename << endl;
        HandleSingleFile(file_path, down_realm, up_realm);
      }
    }
  }
}

void HandleDirectory(string &directory_path, int down_realm, int up_realm,
                     int argc, char **argv) {
  // 原理同上，循环读取每个文件
  // 然后调用 HandleSingleFile(input_file_path,down_realm,up_realm,int
  // argc,char**argv)
  filesystem::path dir_path(directory_path);

  for (const auto &entry : filesystem::directory_iterator(dir_path)) {
    if (entry.is_regular_file()) {
      string filename = entry.path().filename().string();
      // 跳过以'module'开头的文件
      if (filename.find("module") == 0) {
        continue;
      }

      // 只处理.json文件
      if (filename.find(".json") == filename.length() - 5) {
        string file_path = entry.path().string();
        cout << "Processing file: " << filename << endl;
        HandleSingleFile(file_path, down_realm, up_realm, argc, argv);
      }
    }
  }
}

void HandleSingleFile(string &file_path, int down_realm, int up_realm) {
  // 读取对应的JSON文件并解析
  ifstream ifs(file_path);
  if (!ifs.is_open()) {
    cerr << "Cannot open the file: " << file_path << endl
         << "Please check the path!" << endl;
    return;
  }

  json j;
  ifs >> j;

  // 收集所有节点ID，建立ID到索引的映射
  vector<string> id_to_index;
  unordered_map<string, int> id_map;
  for (const auto &node : j) {
    if (node.contains("id") && node["id"].is_string()) {
      string id = node["id"];
      id_map[id] = id_to_index.size();
      id_to_index.push_back(id);
    }
  }

  // 我们设计一个算法来解析这个j
  // 将文件中各个结点的关系以图状结构存储下来，我们只存储结点id和位置关系
  // 这个函数姑且为 vector<list<string>>& Graphicalize(j)
  vector<list<string>> graph = Graphicalize(j);
  // 检查一下是否成功，如果数组为空，那就是有问题的数据
  if (graph.empty()) {
    cerr << "Graphicalize the json file \"" << file_path << "\" error!!" << endl
         << "please check the file contents!" << endl;
    return;
  }

  // 然后我们在下届到上界之间循环，来寻找符合符合的子图
  for (int target_size = down_realm; target_size <= up_realm; target_size++) {
    // 为每个目标节点数寻找所有可能的子图
    vector<vector<string>> all_subgraphs =
        FindAllSubgraphs(graph, id_to_index, target_size);

    for (int index = 0; index < all_subgraphs.size(); index++) {
      string output_file_name =
          GenerateModuleFilename(file_path, target_size, index + 1);

      CreateModuleFile(all_subgraphs[index], j, output_file_name);
    }
  }
}

void HandleSingleFile(string &file_path, int down_realm, int up_realm, int argc,
                      char **argv) {
  ifstream ifs(file_path);
  if (!ifs.is_open()) {
    cerr << "Cannot open the file: " << file_path << endl
         << "Please check the path!" << endl;
    return;
  }
  json j;
  ifs >> j;

  // 我们将需要忽略的参数存储下来
  vector<string> ignored_properties;
  for (int i = 4; i < argc; i++)
    ignored_properties.push_back(argv[i]);

  // 收集所有节点ID，建立ID到索引的映射
  vector<string> id_to_index;
  unordered_map<string, int> id_map;
  for (const auto &node : j) {
    if (node.contains("id") && node["id"].is_string()) {
      string id = node["id"];
      id_map[id] = id_to_index.size();
      id_to_index.push_back(id);
    }
  }

  // 最好检查一下是否存在对应的属性，但是我们可以将这一步直接放进
  // Graphicalize()函数中，如果不存在,就直接cerr报错,返回空数组就是了
  vector<list<string>> graph = Graphicalize(j, ignored_properties);
  if (graph.empty()) {
    cerr << "Graphicalize the json file \"" << file_path << "\" error!!" << endl
         << "please check the file contents!" << endl;
    return;
  }

  // 然后我们在下届到上界之间循环，来寻找符合符合的子图
  for (int target_size = down_realm; target_size <= up_realm; target_size++) {
    // 为每个目标节点数寻找所有可能的子图
    vector<vector<string>> all_subgraphs =
        FindAllSubgraphs(graph, id_to_index, target_size);

    for (int index = 0; index < all_subgraphs.size(); index++) {
      string output_file_name = GenerateModuleFilename(
          file_path, target_size, index + 1, ignored_properties);
      CreateModuleFile(all_subgraphs[index], j, output_file_name);
    }
  }
}

// DFS辅助函数：从指定节点开始遍历连通分量
void DfsTraverse(const vector<list<string>> &graph,
                 const unordered_map<string, int> &id_map,
                 const string &node_id, vector<bool> &visited,
                 vector<int> &component) {
  if (id_map.find(node_id) == id_map.end())
    return;

  int node_idx = id_map.at(node_id);
  if (visited[node_idx])
    return;

  visited[node_idx] = true;
  component.push_back(node_idx);

  for (const auto &neighbor_id : graph[node_idx]) {
    if (id_map.find(neighbor_id) != id_map.end()) {
      DfsTraverse(graph, id_map, neighbor_id, visited, component);
    }
  }
}

// 查找所有连通分量
vector<vector<int>>
FindConnectedComponents(const vector<list<string>> &graph,
                        const unordered_map<string, int> &id_map) {
  int n = graph.size();
  vector<bool> visited(n, false);
  vector<vector<int>> components;

  // 需要建立索引到ID的反向映射
  vector<string> index_to_id(n);
  for (const auto &pair : id_map) {
    index_to_id[pair.second] = pair.first;
  }

  for (int i = 0; i < n; i++) {
    if (!visited[i]) {
      vector<int> component;
      DfsTraverse(graph, id_map, index_to_id[i], visited, component);
      if (!component.empty()) {
        components.push_back(component);
      }
    }
  }

  return components;
}

// 从连通分量中提取指定大小的子图
vector<string>
extract_subgraph_from_component(const vector<int> &component,
                                const vector<string> &id_to_index,
                                int target_size) {
  if (component.size() < target_size) {
    return {};
  }

  vector<string> result;
  for (int i = 0; i < target_size && i < component.size(); i++) {
    int node_idx = component[i];
    if (node_idx < id_to_index.size()) {
      result.push_back(id_to_index[node_idx]);
    }
  }

  return result;
}

// 这个函数用来寻找符合要求的子图，并且将结点以字符串数组的形式保留下来
vector<string> FindSubgraph(vector<list<string>> &graph, int target_size) {
  // 如果graph转化失败或target_size有问题，则返回空数组
  if (graph.empty() || target_size <= 0 || target_size > graph.size()) {
    cerr << "The graph has something wrong!! Please check the JSON file!"
         << endl;
    return {};
  }

  // 建立ID到索引的映射（简化版本）
  unordered_map<string, int> id_map;
  for (int i = 0; i < graph.size(); i++) {
    id_map["node_" + to_string(i)] = i;
  }

  // 由于节点数量较少，使用连通分量分析
  auto components = FindConnectedComponents(graph, id_map);

  // 在每个连通分量中查找满足大小要求的子图
  for (const auto &component : components) {
    if (component.size() >= target_size) {
      // 从图中提取实际的节点ID
      vector<string> result;
      for (int i = 0; i < target_size && i < component.size(); i++) {
        int node_index = component[i];
        result.push_back("node_" + to_string(node_index));
      }

      return result;
    }
  }

  return {};
}

// 查找所有满足指定节点数量的子图
vector<vector<string>> FindAllSubgraphs(vector<list<string>> &graph,
                                        vector<string> &id_to_index,
                                        int target_size) {
  vector<vector<string>> result;

  if (graph.empty() || target_size <= 0 || target_size > graph.size()) {
    return result;
  }

  // 建立ID到索引的映射
  unordered_map<string, int> id_map;
  for (int i = 0; i < id_to_index.size(); i++) {
    id_map[id_to_index[i]] = i;
  }

  // 查找所有连通分量
  auto components = FindConnectedComponents(graph, id_map);

  // 对每个连通分量，提取满足大小的子图
  for (const auto &component : components) {
    if (component.size() >= target_size) {
      vector<string> subgraph;
      for (int i = 0; i < target_size && i < component.size(); i++) {
        int node_idx = component[i];
        if (node_idx < id_to_index.size()) {
          subgraph.push_back(id_to_index[node_idx]);
        }
      }
      if (subgraph.size() == target_size) {
        result.push_back(subgraph);
      }
    }
  }

  return result;
}

vector<list<string>> Graphicalize(json &j) {
  // 这个函数实现用图状结构来存储JSON文件中的结点
  if (!j.is_array() || j.empty()) {
    cerr << "Invalid JSON format: not an array or empty" << endl;
    return {};
  }

  // 收集所有节点ID，建立ID到索引的映射
  vector<string> id_to_index;
  unordered_map<string, int> id_map;

  for (const auto &node : j) {
    if (!node.contains("id") || !node["id"].is_string()) {
      cerr << "Node missing id or id is not string" << endl;
      return {};
    }
    string id = node["id"];
    id_map[id] = id_to_index.size();
    id_to_index.push_back(id);
  }
  cout << "We have find the " << id_to_index.size() << " nodes!" << endl;

  vector<list<string>> graph(id_to_index.size());

  // 处理每个节点的连接关系
  for (const auto &node : j) {
    string current_id = node["id"];
    int current_index = id_map[current_id];

    // 处理wires单向连接
    if (node.contains("wires") && node["wires"].is_array() &&
        !node["wires"].empty() && node["wires"][0].is_array()) {
      for (const auto &target : node["wires"][0]) {
        if (target.is_string() &&
            id_map.find(target.get<string>()) != id_map.end()) {
          graph[current_index].push_back(target.get<string>());
        }
      }
    }

    // 处理其他属性 双向连接
    for (auto it = node.begin(); it != node.end(); ++it) {
      string key = it.key();
      if (key == "id" || key == "wires") // 跳过id和wires属性
        continue;

      if (it.value().is_string()) {
        string value = it.value().get<string>();
        if (id_map.find(value) != id_map.end()) {
          // 双向连接：当前节点 <-> 目标节点
          graph[current_index].push_back(value);
          int target_index = id_map[value];
          graph[target_index].push_back(current_id);
        }
      }
    }
  }
  return graph;
}

vector<list<string>> Graphicalize(json &j, vector<string> &ignored_properties) {
  // 这个函数实现用图状结构来存储JSON文件中的结点，但同时需要考虑忽略的属性
  if (!j.is_array() || j.empty()) {
    cerr << "Invalid JSON format: not an array or empty" << endl;
    return {};
  }

  // 收集所有节点ID，建立ID到索引的映射
  vector<string> id_to_index;
  unordered_map<string, int> id_map;

  for (const auto &node : j) {
    if (!node.contains("id") || !node["id"].is_string()) {
      cerr << "Node missing id or id is not string" << endl;
      return {};
    }
    string id = node["id"];
    id_map[id] = id_to_index.size();
    id_to_index.push_back(id);
  }

  // 初始化邻接表
  vector<list<string>> graph(id_to_index.size());

  // 将忽略属性转换为集合以便快速查找
  unordered_set<string> ignored_set(ignored_properties.begin(),
                                    ignored_properties.end());

  // 处理每个节点的连接关系
  for (const auto &node : j) {
    if (!node.contains("id"))
      continue;

    string current_id = node["id"];
    int current_idx = id_map[current_id];

    // 处理wires[0] - 单向连接
    if (node.contains("wires") && node["wires"].is_array() &&
        !node["wires"].empty() && node["wires"][0].is_array()) {
      for (const auto &target : node["wires"][0]) {
        if (target.is_string() &&
            id_map.find(target.get<string>()) != id_map.end()) {
          graph[current_idx].push_back(target.get<string>());
        }
      }
    }

    // 处理其他属性 - 双向连接（忽略指定属性）
    for (auto it = node.begin(); it != node.end(); ++it) {
      string key = it.key();
      if (key == "id" || key == "wires" ||
          ignored_set.find(key) != ignored_set.end())
        continue;

      if (it.value().is_string()) {
        string value = it.value().get<string>();
        if (id_map.find(value) != id_map.end()) {
          // 双向连接：当前节点 <-> 目标节点
          graph[current_idx].push_back(value);
          int target_idx = id_map[value];
          graph[target_idx].push_back(current_id);
        }
      }
    }
  }

  return graph;
}
