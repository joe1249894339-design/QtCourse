#include "mainwindow.h"

#include <QApplication>
#include <QColor>
#include <QFileInfo>
#include <QIcon>
#include <QLibraryInfo>
#include <QPalette>
#include <QStringList>
#include <QStyleFactory>
#include <QTranslator>

int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("简易文本编辑器"));
    app.setOrganizationName(QStringLiteral("DGUT"));
    app.setWindowIcon(QIcon(QStringLiteral(":/icons/app.png")));

    // 实验参考界面是浅色风格；固定 Fusion + 浅色调色板，
    // 避免演示机器启用 Windows 深色主题时菜单/工具栏变成黑底。
    QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, QColor(32, 32, 32));
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(248, 248, 248));
    lightPalette.setColor(QPalette::ToolTipBase, QColor(255, 255, 220));
    lightPalette.setColor(QPalette::ToolTipText, QColor(32, 32, 32));
    lightPalette.setColor(QPalette::Text, QColor(32, 32, 32));
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, QColor(32, 32, 32));
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(0, 102, 204));
    lightPalette.setColor(QPalette::Highlight, QColor(0, 120, 215));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);
    lightPalette.setColor(QPalette::Disabled, QPalette::Text, QColor(120, 120, 120));
    lightPalette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(120, 120, 120));
    lightPalette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(120, 120, 120));
    QApplication::setPalette(lightPalette);

    // 加载 Qt 自带的中文翻译，让 QFontDialog/QColorDialog 等标准对话框的
    // “字体、字号、确定、取消”等文字显示为中文。
    QTranslator qtTranslator;
    QStringList translationDirs;
    translationDirs << QCoreApplication::applicationDirPath() + QStringLiteral("/translations");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    translationDirs << QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    translationDirs << QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif
    for (const QString &dir : translationDirs) {
        if (qtTranslator.load(QStringLiteral("qtbase_zh_CN"), dir)) {
            app.installTranslator(&qtTranslator);
            break;
        }
    }

    MainWindow *window = new MainWindow;
    window->show();

    // 支持通过命令行直接打开文件：TextEditor.exe D:\path\demo.txt
    const QStringList args = app.arguments();
    if (args.size() > 1) {
        const QString path = args.at(1);
        if (QFileInfo::exists(path))
            window->openPath(path);
    }

    return app.exec();
}
