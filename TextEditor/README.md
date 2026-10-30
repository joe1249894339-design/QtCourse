# 简易文本编辑器（Qt Widgets）

Qt 应用程序开发实验 2：多窗口文本编辑器。工程使用 Qt Widgets / C++，界面由
`.ui` 文件描述，菜单和工具栏由 Qt Designer 的 **Action Editor** 创建，编辑器由
`QPlainTextEdit` 派生并 **promote** 为 `CodeEditor`，从而支持行号。

## 功能

- 多顶层窗口：新建/打开文档时每个文档一个独立窗口；每个窗口有独立的标题、修改状态、
  撤销栈和查找/替换对话框。
- 文件：新建、打开、保存、另存为；UTF-8 优先、系统编码（中文 Windows 下通常为 GBK）
  回退；识别 UTF-8/UTF-16 BOM；拒绝二进制文件；`QSaveFile` 原子保存。
- 编辑：撤销、恢复、剪切、复制、粘贴、全选；Action 使能由编辑器信号和剪贴板状态驱动。
- 格式：字体、字号、字体颜色、字体背景色、编辑器背景色、自动换行。
- 视图：显示/隐藏行号、工具栏、状态栏；状态栏显示 length / lines / Ln / Col 和姓名。
- 查找/替换：向上/向下、区分大小写、连续查找（循环）、未找到提示、替换当前、全部替换
  （统计处数，合并为一次撤销）。
- 关闭未保存提示（保存 / 不保存 / 取消）；标题随修改状态显示 `*`。
- 定时自动保存（默认 60 秒，可在“文件”菜单关闭；只对已有路径的文件生效）。
- 关于对话框；程序图标；浅色 Fusion 主题（避免系统深色主题把状态栏画黑）。

## 环境

- 实际开发/验证：**Qt 6.11.2 (MinGW 64-bit)**、Qt Creator、Windows 11。
- 编译工具：MinGW GCC 13.1.0、qmake。
- 实验文档写的是 Qt 5.12.11；源码在编码处理处保留了 Qt 5/Qt 6 条件编译，
  但未在 Qt 5.12 实机复验。

## 目录

```text
TextEditor/
├─ TextEditor.pro              # 主工程（qmake）
├─ main.cpp                    # 入口，创建第一个窗口
├─ mainwindow.h/.cpp/.ui       # 主窗口、菜单/工具栏/Action、文件与编辑逻辑
├─ codeeditor.h/.cpp           # QPlainTextEdit 派生类 + 行号区 + 当前行高亮
├─ documentio.h/.cpp           # 文件读写、编码识别、二进制检测
├─ finddialog.h/.cpp/.ui       # 查找对话框
├─ replacedialog.h/.cpp/.ui    # 替换对话框
├─ aboutdialog.h/.cpp/.ui      # 关于对话框
├─ resources.qrc               # 图标资源
├─ resources/icons/            # 工具栏/菜单图标、app.png、app.ico
├─ demo/                       # 演示用文本文件
├─ tests/                      # Qt Test 测试工程
├─ report/                     # 填好的实验报告、测试输出、截图
├─ bin/                        # 已编译的 TextEditor.exe 和 Qt 运行库
├─ README.md
├─ GIT_COMMIT_PLAN.md
└─ DEMO_SCRIPT.md
```

## 编译

### Qt Creator

1. 打开 `TextEditor.pro`。
2. 选择 `Desktop Qt 6.11.2 MinGW 64-bit` 套件。
3. Build / Run。

### 命令行（PowerShell）

```powershell
$env:PATH = 'D:\Qt\6.11.2\mingw_64\bin;D:\Qt\Tools\mingw1310_64\bin;' + $env:PATH
Set-Location 'D:\deepseek工作区\TextEditor'
New-Item -ItemType Directory -Force build | Out-Null
Set-Location build
& 'D:\Qt\6.11.2\mingw_64\bin\qmake.exe' '..\TextEditor.pro' -spec win32-g++
& 'D:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe' -j4
```

生成的可执行文件在 `bin\TextEditor.exe`。**完整交付包**的 `bin` 已带 `platforms`、
`styles`、`imageformats` 和 MinGW 运行库，可直接双击运行；**GitHub 源码仓库**
只提交源码，不提交 `bin/`、`build/`，需要先按上面的步骤构建。

如果换一台机器构建，运行前可在 `bin` 目录执行：

```powershell
& 'D:\Qt\6.11.2\mingw_64\bin\windeployqt.exe' --release --compiler-runtime .\TextEditor.exe
```

> 说明：工程路径含中文时 `windres` 无法读取 `RC_ICONS` 的 `.ico` 绝对路径，
> 所以主工程移除了 `RC_ICONS`；窗口/任务栏图标由 `resources.qrc` 中的
> `app.png` + `QApplication::setWindowIcon` 提供。

## 运行

```powershell
.\bin\TextEditor.exe
.\bin\TextEditor.exe .\demo\sample.txt
```

多窗口行为：

- “新建”总是创建一个新的顶层编辑窗口。
- “打开”时，如果当前窗口是空白且未修改，就在当前窗口打开；否则新开一个窗口，
  不覆盖当前文档。
- “退出”依次关闭所有编辑窗口，每个有未保存修改的窗口都会提示。

## 测试

```powershell
$env:PATH = 'D:\Qt\6.11.2\mingw_64\bin;D:\Qt\Tools\mingw1310_64\bin;' + $env:PATH
Set-Location 'D:\deepseek工作区\TextEditor\tests'
New-Item -ItemType Directory -Force build | Out-Null
Set-Location build
& 'D:\Qt\6.11.2\mingw_64\bin\qmake.exe' '..\tests.pro' -spec win32-g++
& 'D:\Qt\Tools\mingw1310_64\bin\mingw32-make.exe' -j4
& '..\bin\tst_texteditor.exe' -v1
```

当前结果：**15 passed, 0 failed**。完整输出见 `report/test_output.txt`。

测试覆盖：

- UTF-8 文件读写、二进制文件拒绝、不存在文件提示
- 打开后的标题/修改状态、保存后 modified 清除
- Action 使能（无选中/有选中、剪贴板、撤销/恢复、打开编辑菜单）
- 查找连续命中/循环/未找到/区分大小写
- 替换当前、全部替换、一次撤销恢复
- 自动保存、行号宽度随行数变化

## 报告与提交

- 实验报告：`report/实验2-简单文本编辑器-黄俊豪-定稿.docx`
- 测试输出：`report/test_output.txt`
- 运行截图：`report/screenshots/`
- GitHub 提交顺序：`GIT_COMMIT_PLAN.md`
- 录屏分镜：`DEMO_SCRIPT.md`

实验要求“GitHub 提交记录”和“测试视频”由本人提交/录制；报告和脚本已准备好。
