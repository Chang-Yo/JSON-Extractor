# JSON-Extractor
任务要求见`request.md`文档，建议安装**mermaid解析拓展**再预览，可以获得更好的可视化体验。

**注意**：输出的`module_XXX.json`文件是**连通**的自包含子图！你可以在`CoreAlgor.cpp`第**182**行添加注释，以此获得包括不连通的自包含子图。

# 项目结构树🌳
```plaintext
workspace
|
|-- request.md           //题目详细要求
|
|-- README.md            //项目介绍
|
|-- CMakeLists.txt       // 环境编译设置
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

# 构建项目并编译

建议使用Linux环境来编译文件，如果你使用的是Windows环境，请确保你的编译环境功能和组件完整，包括cmake,ninja,Windows SDKs等等。Mac环境并没有经过编译测试，请自行处理。

## Linux环境
编译器要求：GNU toolchain / llvm toolchain

1. 创建`build`文件夹并生成`makefile`
    ```bash
    mkdir build && cd build # 如果build文件夹已存在可以直接 cd build
    cmake ..
    ```
2. 开始编译
    ```bash
    make
    ```

## Windows环境

编译器推荐：MinGW / Clang / MVSC

### 使用 MinGW （推荐）
MinGW 的使用体验和Linux环境基本一致，因为调用的是GCC.
```shell
mkdir build && cd build && cmake .. 
```
同样可以发现生成了`Makefile`，直接编译
```shell
make
```

### 使用 Clang
依然是创建并进入build文件夹
```shell
mkdir build && cd build && cmake .. 
```
会发现build下含有`build.ninja`文件，我们通过ninja来构建（请确保系统含有ninja）
```shell
ninja
```
编译结束即可发现`tool.exe`可执行文件

### 使用 MVSC
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

# 运行项目:

！！！请确保终端环境当前在`build/`目录下！！！

如果不在`build/`下执行，请`cd build`进入build环境。

`tool`调用规则及可接收参数：

```bash
./tool path/to/or/directory down_realm up_realm [Optional:Serveral-Ignored-Properties]
```

如果使用的是VSCode，你可以在 Run and Debug 中来调试，项目在`launch.json`中提供了`LLDB`和`GDB`两种调试器配置。

调试默认终端输入路径为`WorkspaceFolder/build/`，默认输入参数为：

```bash
./tool test/test.json 5 6
```

# 如何调试

你可以直接设置断点，然后按下`F5`，即可开始调试；或者你也可以通过 CMake Tools 的 Debug 功能来调试来调试。

联系作者：[Zhixi Hu](mailto:running_stream@sjtu.edu.cn)