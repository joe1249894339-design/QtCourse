# GitHub 提交计划

本实验要求“有体现代码迭代、测试和问题修复的过程”。建议按下面的顺序提交，
每条提交只包含该阶段相关的文件，这样 `git log --oneline --graph` 能看出开发过程。

> 说明：本工程由本地生成，GitHub 仓库的创建和最终 push 由本人完成。
> 如果仓库地址还没有，先在 GitHub 新建空仓库，再执行 `git remote add origin <url>`。

## 提交前检查

```powershell
git status
git add .
git commit -m "..."
```

确认 `.gitignore` 至少忽略：

```text
build/
tests/build/
tools/build/
*.user
*.o
*.exe
bin/*.dll
bin/platforms/
bin/styles/
bin/imageformats/
```

如果希望把可执行程序和 Qt 运行库一起提交，可以保留 `bin/`；否则只提交源码、
`.ui`、`.qrc`、报告和文档，并在 README 说明构建方法。

## 建议的 10 次提交

| # | 提交信息 | 主要文件 |
|---|---|---|
| 1 | `chore: 初始化 Qt 工程与图标资源` | `TextEditor.pro`、`main.cpp`、`resources.qrc`、`resources/icons/` |
| 2 | `feat: 主窗口 UI 与 Action 分组` | `mainwindow.ui`、`mainwindow.h`、`mainwindow.cpp` 的 UI/菜单/工具栏部分 |
| 3 | `feat: CodeEditor 行号与当前行高亮` | `codeeditor.h`、`codeeditor.cpp`、`mainwindow.ui` 的 promote 配置 |
| 4 | `feat: 多窗口与文件新建/打开/保存/另存为` | `documentio.h/.cpp`、`mainwindow.cpp` 的文件操作、`mainwindow.ui` 文件菜单 |
| 5 | `feat: 编辑动作与使能状态` | `mainwindow.cpp` 的 Action 连接、`updateEditActions`、`updatePasteAction` |
| 6 | `feat: 字体/颜色/背景/自动换行` | `mainwindow.cpp` 的格式动作、`codeeditor.cpp` 的背景色/行号颜色 |
| 7 | `feat: 查找与替换` | `finddialog.*`、`replacedialog.*`、`mainwindow.cpp` 的 `findText`/`replaceAll` |
| 8 | `feat: 关于对话框与自动保存` | `aboutdialog.*`、`mainwindow.cpp` 的 `autoSave`、关闭提示 |
| 9 | `test: 边界测试与问题修复` | `tests/`、`README.md` 的测试说明、`report/test_output.txt` |
| 10 | `docs: 实验报告与运行说明` | `report/*.docx`、`README.md`、`GIT_COMMIT_PLAN.md`、`DEMO_SCRIPT.md` |

## 常用命令

```powershell
git init
git add TextEditor.pro main.cpp resources.qrc resources/icons
git commit -m "chore: 初始化 Qt 工程与图标资源"

git add mainwindow.h mainwindow.cpp
git commit -m "feat: 主窗口 UI 与 Action 分组"

git add codeeditor.h codeeditor.cpp
git commit -m "feat: CodeEditor 行号与当前行高亮"

# ……按上表继续
git remote add origin https://github.com/<your-name>/<repo>.git
git branch -M main
git push -u origin main
```

## 提交记录截图

提交完成后，在仓库首页或 `git log --oneline --graph --decorate` 的输出处截图，
放到实验报告“Github日志”一节，或作为作业系统附件提交。
