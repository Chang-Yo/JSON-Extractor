#include "json.hpp"
#include <exception>
#include <fstream>
#include <iostream>
#include <string>

using json = nlohmann::json;
using namespace std;
const string BASIC_PATH_PREFIX = "../test/";

void parse_file_example() {
  const string FILE_PATH = "../test/1.json";
  ifstream input_file(FILE_PATH);
  if (!input_file.is_open()) {
    cerr << "ERROR: Can't open the file: \"" << FILE_PATH
         << "\". Please check the file path again" << endl;
    return;
  }

  try {
    json data;
    input_file >> data;

    if (!data.is_array()) {
      cerr << "ERROR: JSON file is not a array!" << endl;
      return;
    }

    cout << "The file <" << FILE_PATH << "> contains" << data.size()
         << " nodes." << endl;
    for (const auto &node : data) {
      if (node.is_object()) {
        string id = node.at("id").get<string>();
        cout << "\n -----File Parsing-----\n";
        cout << "Node ID:\t " << id << endl;

        if (node.find("name") != node.end()) {
          string name = node.at("name").get<string>();
          cout << "Node name:\t " << name << endl;
        }
      }
    }
  } catch (const json::parse_error &e) {
    cerr << "JSON Parse ERROR!" << e.what() << " (Location: " << e.byte << ")"
         << endl;
  } catch (const exception &e) {
    cerr << "Running ERROR!" << e.what() << endl;
  }
}

void parse_multi_files();

int main(int argc, char *argv[]) {
  parse_file_example();

  return 0;
}