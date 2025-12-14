#include "CoreAlgor.h"
#include "IOprocess.h"
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <vector>

void HandleDirectory(string &directory_path, int down_realm, int up_realm) {
  // 在目录下读取每个文件，记录为input_file_path
  // 然后循环调用 HandleSingleFile(input_path,down_realm,up_realm)
}

void HandleDirectory(string &directory_path, int down_realm, int up_realm,
                     char **argv) {
  // 原理同上，循环读取每个文件
  // 然后调用 HandleSingleFile(input_file_path,down_realm,up_realm,char**argv)
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
  for (int i = down_realm, index = 1; i <= up_realm; i++, index++) {

    // 此处我们可以将寻找子图的算法封装为一个函数，让他返回一个包含对应结点id的数组
    vector<string> target_id_set = FindSubgraph(graph);
    string output_file_name = GenerateModuleFilename(file_path, i, index);
    CreateModuleFile(target_id_set, j, output_file_name);
  }
}

void HandleSingleFile(string &file_path, int down_realm, int up_realm,
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
  int size = sizeof(argv);
  for (int i = 4; i < size; i++)
    ignored_properties.push_back(argv[i]);

  // 最好检查一下是否存在对应的属性，但是我们可以将这一步直接放进
  // Graphicalize()函数中，如果不存在,就直接cerr报错,返回空数组就是了
  vector<list<string>> graph = Graphicalize(j, ignored_properties);
  if (graph.empty()) {
    cerr << "Graphicalize the json file \"" << file_path << "\" error!!" << endl
         << "please check the file contents!" << endl;
    return;
  }

  for (int i = down_realm, index = 1; i < up_realm; i++, index++) {
    // 然后同样是寻找子图，函数还是一样的
    vector<string> target_id_set = FindSubgraph(graph);
    // 然后就可以构建模块了
    string output_file_name = GenerateModuleFilename(file_path, i, index);
    CreateModuleFile(target_id_set, j, output_file_name);
  }
}

vector<string> FindSubgraph(vector<list<string>> &) {
  // 这个函数用来寻找符合要求的子图，并且将结点以字符串数组的形式保留下来
  vector<string> None;
  return None;
}

vector<list<string>> Graphicalize(json &j) {
  // 这个函数实现用图状结构来存储JSON文件中的结点
  vector<list<string>> None;
  return None;
}

vector<list<string>> Graphicalize(json &j, vector<string> &ignored_properties) {
  // 这个函数实现用图状结构来存储JSON文件中的结点，但同时需要考虑忽略的属性
  vector<list<string>> None;
  return None;
}
