# JSON-Extractor
任务要求见`request.md`文档，建议安装**mermaid解析拓展**再预览，可以获得更好的可视化体验。

请在运行前**认真阅读**项目目录下的`Note.md`文档，里面补充了项目的许多**重要提示**！！！尤其注释调用 tool 工具时的路径问题！


# 项目结构树🌳
```plaintext
workspace
|
|-- request.md           // 题目详细要求
|
|-- README.md            // 项目介绍
|
|-- Note.md              // ！！对项目的重要补充信息！！
|
|-- CMakeLists.txt       // 编译环境设置
|
|-- src/                 // 存放cpp源代码
|
|-- example/             // 用AI构建的测试集和测试报告
|
|-- include/             // 存放头文件
|
|
|-- test/                // 存放测试用例
|
|-- .gitignore
```

# 快速开始

## 构建项目并编译

建议使用Linux环境来编译文件.如果你使用的是Windows环境，请确保你的编译环境功能和组件完整，包括cmake,ninja,Windows SDKs等等。Mac环境并没有参与编译测试，请自行妥善处理。

如果出现编译问题，请优先检查你的**环境组件**是否确认完整！

### Linux环境
编译器要求：GNU toolchain / llvm toolchain

创建`build`文件夹并生成`makefile`
    
```bash
mkdir build && cd build # 如果build文件夹已存在可以直接 cd build
cmake ..
```
开始编译
```bash
make
```

### Windows环境

编译器要求：MinGW / Clang / MVSC

#### 使用 MinGW （推荐）
MinGW 的使用体验和Linux环境基本一致，因为调用的是GCC.
```shell
mkdir build && cd build && cmake .. 
```
同样可以发现生成了`Makefile`，直接编译
```shell
make
```

#### 使用 Clang
依然是创建并进入build文件夹
```shell
mkdir build && cd build && cmake .. 
```
会发现build下含有`build.ninja`文件，我们通过ninja来构建（请确保系统含有ninja）
```shell
ninja
```
编译结束即可发现`tool.exe`可执行文件

#### 使用 MVSC
由于MVSC并不会自动读取环境变量，我们推荐在**x64 Native Tools Command Prompt for VS**中来编译项目。

首先找到电脑上的**x64 Native Tools Command Prompt for VS**程序，然后进入所在项目目录：
```bash
cd /D path/to/your/project/workspace
```
然后指定使用Ninja来构建：
```bash
# rmdir /s /q build  # 删除已存在的文件夹（可选）
cmake -G Ninja -B build .
```
然后同样启动ninja来编译：
```bash
cd build && ninja
```

## 运行项目:

！！！请确保终端环境当前在`build/`目录下！！！

如果不在`build/`下执行，请`cd build`进入build环境。

`tool`调用规则及可接收参数：

```bash
./tool path/to/file/or/directory down_realm up_realm [Optional:Serveral-Ignored-Properties]
```

如果使用的是VSCode，你可以在 Run and Debug 中来调试，项目在`launch.json`中提供了`LLDB`和`GDB`两种调试器配置。

调试默认终端输入路径为`WorkspaceFolder/build/`，默认输入参数为：

```bash
./tool test/test.json 5 6
```

## 如何调试

你可以设置断点开始调试；或者你也可以通过 CMake Tools 的 Debug 功能来调试来调试。

请在调试前将`.vscode/`目录下的配置文件自行妥善完善。

# 核心工作流程

![核心工作流程](/illustration.png)


---

作者：[Zhixi Hu](https://github.com/Chang-Yo)

邮箱：[running_stream@sjtu.edu.cn](mailto:running_stream@sjtu.edu.cn)

如果有问题欢迎随时联系。

本项目主要代码托管在[GitHub平台](https://github.com/Chang-Yo/JSON-Extractor)，遵循 [Apache-2.0](/LICENSE) 开源协议。

（为什么文档写这么多？只是想把项目做完美一些，而且，代码跑不了真的不一定是我的代码有BUG....）