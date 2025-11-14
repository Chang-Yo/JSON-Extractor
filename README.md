# JSON Subgraph Extractor

This tool extracts subgraphs from JSON files representing a graph structure.

## Build Instructions

### Prerequisites
- C++17 compiler (g++, clang++)
- CMake 3.10+

### Build Steps
1. Create a build directory:
   ```bash
   mkdir build
   cd build
   ```
2. Run CMake:
   ```bash
   cmake ..
   ```
3. Build the project:
   ```bash
   make
   ```

## Usage

The executable `mytool` will be created in the `build` directory.

```bash
./mytool <target_path> <m1> <m2> [ignored_attributes...]
```

- `<target_path>`: Path to a JSON file or a directory containing JSON files.
- `<m1>`: Upper bound for the number of nodes in the subgraph.
- `<m2>`: Lower bound for the number of nodes in the subgraph.
- `[ignored_attributes...]`: (Optional) A list of attribute names to ignore when building graph connections.

### Examples

- Process a single file, extracting subgraphs with 5 to 6 nodes:
  ```bash
  ./mytool ../data.json 6 5
  ```

- Process all JSON files in a directory, ignoring the 'z' attribute, extracting subgraphs with 5 to 10 nodes:
  ```bash
  ./mytool ../data_dir 10 5 z
  ```
