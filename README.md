# JSON-Extractor
任务要求见`request.md`文档，建议使用能够**支持mermaid解析**的markdown预览工具来预览

# 项目结构树🌳
```plaintext
workspace
|
|-- request.md           //题目详细要求
|
|-- README.md            //项目介绍
|
|-- CMakeLists.txt
|
|-- src/                 //存放cpp源代码
|
|
|-- include/             //存放头文件
|
|
|-- test/                //存放测试用例
|
|-- .gitignore
```

# 环境准备
llvm toolchain >=20.0 / GNU toolchain

# 如何开始
1. 将仓库克隆到本地，在项目目录下运行以下命令:

```bash
mkdir build && cd build 
```
2. 开始构建并编译项目:

```bash
cmake .. && make
```

或者如果你使用的是llvm工具链，请输入：

```bash
cmake -D CMAKE_CXX_COMPILER=clang++ && make
```
3. 运行项目:

```bash
./executable
```
4. 如何调试
   
你可以直接设置断点，然后按下`F5`，即可开始调试。

**注意：**在调试之前请先选择好您的调试器，在"Run and Debug"下拉菜单中选择GDB调试器或LLDB调试器。如果你没有安装LLDB,请选择GDB调试

或者你也可以通过 CMake Tools 来调试。


联系作者：[running_stream@sjtu.edu.cn](mailto:running_stream@sjtu.edu.cn)
