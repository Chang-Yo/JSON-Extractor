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

// 专门处理wires属性的连接关系
void WiresConnect(const json &node, vector<list<string>> &graph,
                  int current_index) {
  if (!node.contains("wires") || !node["wires"].is_array()) {
    return;
  }

  for (const auto &wire_group : node["wires"]) {
    if (wire_group.is_array()) {
      // 处理数组格式：["id1", "id2"] 或 [{"id": "id1"}, {"id": "id2"}]
      for (const auto &target : wire_group) {
        string target_id;

        if (target.is_string()) {
          // 字符串格式："target_id"
          target_id = target.get<string>();
        } else if (target.is_object() && target.contains("id")) {
          // 对象格式：{"id": "target_id", ...}
          target_id = target["id"];
        } else {
          continue; // 跳过不支持格式
        }

        // 添加到图中
        if (id_map.find(target_id) != id_map.end()) {
          graph[current_index].push_back(target_id);
        }
      }
    } else if (wire_group.is_object() && wire_group.contains("id")) {
      // 直接对象格式：{"id": "target_id", ...}
      string target_id = wire_group["id"];
      if (id_map.find(target_id) != id_map.end()) {
        graph[current_index].push_back(target_id);
      }
    }
  }
}

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
  SetMapping(j);

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

  PrintGraph(graph);
  // 然后我们在下届到上界之间循环，来寻找符合的子图
  for (int target_size = down_realm; target_size <= up_realm; target_size++) {
    if (target_size == 1) {
      int i = 1;
      for (auto &node : j) {
        vector<string> subgraph{node["id"]};
        string output_file_name =
            GenerateModuleFilename(file_path, target_size, i++);

        CreateModuleFile(subgraph, j, output_file_name);
      }
      continue;
    }
    // 为每个目标节点数寻找所有可能的子图
    vector<vector<string>> all_subgraphs =
        FindAllSubgraphs(graph, id_to_index, target_size);

    if (all_subgraphs.size() == 0) {
      cout << "###===### We couldn't find the proper module for size:"
           << target_size << " ###===###" << endl;
      continue;
    }

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
  SetMapping(j);

  // 最好检查一下是否存在对应的属性，但是我们可以将这一步直接放进
  // Graphicalize()函数中，如果不存在,就直接cerr报错,返回空数组就是了
  vector<list<string>> graph = Graphicalize(j, ignored_properties);
  if (graph.empty()) {
    cerr << "Graphicalize the json file \"" << file_path << "\" error!!" << endl
         << "please check the file contents：" << file_path << "!" << endl;
    return;
  }
  PrintGraph(graph);
  // 然后我们在下届到上界之间循环，来寻找符合符合的子图
  for (int target_size = down_realm; target_size <= up_realm; target_size++) {
    if (target_size == 1) {
      int i = 1;
      for (auto &node : j) {
        vector<string> subgraph{node["id"]};
        string output_file_name =
            GenerateModuleFilename(file_path, target_size, i++);

        CreateModuleFile(subgraph, j, output_file_name);
      }
      continue;
    }
    // 为每个目标节点数寻找所有可能的子图
    vector<vector<string>> all_subgraphs =
        FindAllSubgraphs(graph, id_to_index, target_size);

    if (all_subgraphs.size() == 0) {
      cout << "###===### We couldn't find the proper module for size:"
           << target_size << " ###===###" << endl;
      continue;
    }

    for (int index = 0; index < all_subgraphs.size(); index++) {
      string output_file_name = GenerateModuleFilename(
          file_path, target_size, index + 1, ignored_properties);
      CreateModuleFile(all_subgraphs[index], j, output_file_name);
    }
  }
}

// 第一次DFS：记录完成顺序（Kosaraju算法第一步）
void DfsForOrder(int u, const vector<list<string>> &graph,
                 vector<bool> &visited, vector<int> &order) {
  visited[u] = true;
  for (const string &neighbor_id : graph[u]) {
    if (id_map.find(neighbor_id) != id_map.end()) {
      int v = id_map[neighbor_id];
      if (!visited[v]) {
        DfsForOrder(v, graph, visited, order);
      }
    }
  }
  order.push_back(u); // 完成时加入顺序
}

// 构建转置图（Kosaraju算法第二步）
vector<list<string>> BuildTransposeGraph(const vector<list<string>> &graph) {
  int n = graph.size();
  vector<list<string>> transpose(n);

  for (int u = 0; u < n; u++) {
    for (const string &neighbor_id : graph[u]) {
      if (id_map.find(neighbor_id) != id_map.end()) {
        int v = id_map[neighbor_id];
        transpose[v].push_back(id_to_index[u]); // 反向边
      }
    }
  }
  return transpose;
}

// 第二次DFS：在转置图上找强连通分量（Kosaraju算法第三步）
void DfsOnTranspose(int u, const vector<list<string>> &transpose,
                    vector<bool> &visited, vector<int> &component) {
  visited[u] = true;
  component.push_back(u);

  for (const string &neighbor_id : transpose[u]) {
    if (id_map.find(neighbor_id) != id_map.end()) {
      int v = id_map[neighbor_id];
      if (!visited[v]) {
        DfsOnTranspose(v, transpose, visited, component);
      }
    }
  }
}

// 查找所有强连通分量（使用Kosaraju算法）
vector<vector<int>> FindConnectedComponents(const vector<list<string>> &graph) {
  int n = graph.size();
  vector<bool> visited(n, false);
  vector<int> order;
  vector<vector<int>> sccs; // 强连通分量

  // 第一步：第一次DFS记录完成顺序
  for (int i = 0; i < n; i++) {
    if (!visited[i]) {
      DfsForOrder(i, graph, visited, order);
    }
  }

  // 第二步：构建转置图
  vector<list<string>> transpose = BuildTransposeGraph(graph);

  // 第三步：重置visited，按完成顺序逆序进行第二次DFS
  fill(visited.begin(), visited.end(), false);
  for (int i = order.size() - 1; i >= 0; i--) {
    int u = order[i];
    if (!visited[u]) {
      vector<int> component;
      DfsOnTranspose(u, transpose, visited, component);
      if (!component.empty()) {
        sccs.push_back(component);
      }
    }
  }

  return sccs;
}

// 查找所有满足指定节点数量的子图
vector<vector<string>> FindAllSubgraphs(vector<list<string>> &graph,
                                        vector<string> &id_to_index,
                                        int target_size) {
  vector<vector<string>> result;

  if (graph.empty() || target_size <= 0 || target_size > graph.size()) {
    return result;
  }

  // 查找所有强连通分量
  auto components = FindConnectedComponents(graph);

  // 对每个强连通分量，提取满足大小的子图
  for (const auto &component : components) {
    if (component.size() == target_size) {
      vector<string> subgraph;
      for (int node_index : component) { // 遍历整个component，而不是截取
        if (node_index < id_to_index.size()) {
          subgraph.push_back(id_to_index[node_index]);
        }
      }
      if (subgraph.size() ==
          target_size) { // 将节点数量符合要求的强连通分量保留
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

  cout << "We have find the " << id_to_index.size() << " nodes!" << endl;

  vector<list<string>> graph(id_to_index.size());

  // 处理每个节点的连接关系
  for (const auto &node : j) {
    string current_id = node["id"];
    int current_index = id_map[current_id];

    // 调用WiresConnect处理wires连接
    WiresConnect(node, graph, current_index);

    // 处理其他属性 双向连接
    for (auto it = node.begin(); it != node.end(); it++) {
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

  cout << "We have found the " << id_map.size() << "nodes!" << endl;
  // 初始化邻接表
  vector<list<string>> graph(id_to_index.size());

  // 将忽略属性转换为集合以便快速查找
  unordered_set<string> ignored_set(ignored_properties.begin(),
                                    ignored_properties.end());

  // 处理每个节点的连接关系
  for (const auto &node : j) {
    string current_id = node["id"];
    int current_index = id_map[current_id];

    // 调用WiresConnect处理wires连接
    WiresConnect(node, graph, current_index);

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
          graph[current_index].push_back(value);
          int target_index = id_map[value];
          graph[target_index].push_back(current_id);
        }
      }
    }
  }

  return graph;
}
