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

# 环境准备

建议使用Linux环境，如果你使用的是Windows环境，请自行配置`launch.json`文件。

推荐环境：GNU toolchain / llvm toolchain 

# 如何开始
1. 将仓库克隆到本地，在项目目录下运行以下命令:

    ```bash
    mkdir build && cd build 
    ```
2. 开始构建并编译项目:

    ```bash
    cmake .. && make
    ```

    如果你想使用llvm工具链，请输入：

    ```bash
    cmake -D CMAKE_CXX_COMPILER=clang++ && make
    ```

3. 运行项目:

    ！！！请确保您当前在`build/`目录下！！！

    如果不在`build/`下执行，请**自行确定**文件路径。

    ```bash
    ./tool path/to/or/directory down_realm up_realm [Optional:Serveral-Ignored-Properties]
    ```

    或者你可以直接按下`F5`来运行，该调试默认调用`tool`的指令为：
    ```bash
    ./tool test/test.json 5 6
    ```

4. 如何调试
   
    你可以直接设置断点，然后按下`F5`，即可开始调试。

**注意**：在调试之前请先选择好您的调试器，在"Run and Debug"下拉菜单中选择GDB调试器或LLDB调试器。如果你没有安装LLDB,请选择GDB调试，或者你也可以通过 CMake Tools 来调试。

联系作者：[Zhixi Hu](mailto:running_stream@sjtu.edu.cn)
