#include "CoreAlgor.h"
#include "IOprocess.h"
#include <iostream>
#include <string>
#include <vector>
using namespace std;
// 由于tool是在build文件夹下调用的，所以我们需要加上路径前缀
const string path_base_name = "../";

int main(int argc, char *argv[]) {
  if (argc <= 3) {
    cerr << "Please input the right argument!!" << endl;
    cerr << "<======Example======>" << endl;
    cerr << "./tool FileOrDirectory down_realm up_realm [Optional: several "
            "properties]"
         << endl;
    return 1;
  }

  // 调用分类器，根据处理目标和是否含有忽略参数来将处理分成4类
  string full_path = path_base_name + argv[1];
  classfication category = classify(argc, argv, full_path);

  // 读取共同含有基本信息：文件/目录路径，上界，下届
  string path = full_path;
  int down_realm = stoi(argv[2]);
  int up_realm = stoi(argv[3]);
  vector<string> ignored_properties = GetIgnoredProperties(argc, argv);

  // 根据分类结果来分别处理
  switch (category) {
  case classfication::directory: {
    cout << "✅ ====>Detect the directory: \"" << path << "\"" << endl;
    HandleDirectory(path, down_realm, up_realm, ignored_properties);
    break;
  }
  case classfication::single_file: {
    cout << "✅ ====>Detect the single file: \"" << path << "\"" << endl;
    HandleSingleFile(path, down_realm, up_realm, ignored_properties);
    break;
  }
  case classfication::directory_with_ignored_property: {
    cout << "✅ ====>Detect the directory: \"" << path << "\"" << endl;
    cout << "✅ ====>Also find the extra arguments!" << endl;
    HandleDirectory(path, down_realm, up_realm, ignored_properties);
    break;
  }
  case classfication::single_file_with_ignored_property: {
    cout << "✅ ====>Detect the single file: \"" << path << "\"" << endl;
    cout << "✅ ====>Also find the extra arguments!" << endl;
    HandleSingleFile(path, down_realm, up_realm, ignored_properties);
    break;
  }

  default:
    cerr << "🔴 🔴 🔴 ERROR： Invalid arguments！ 🔴 🔴 🔴 " << endl
         << "====>No file or directory was found named as: " << path << " "
         << endl;
    break;
  }
  return 0;
}
