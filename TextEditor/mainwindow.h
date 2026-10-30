#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "documentio.h"

class QLabel;
class QTextCharFormat;
class QTimer;
class QColor;
class CodeEditor;
class FindDialog;
class ReplaceDialog;

namespace Ui {
class MainWindow;
}

/**
 * 一个 MainWindow 实例对应一个文档窗口。
 * “新建/打开”会创建新的 MainWindow，实现多顶层窗口的文本编辑器。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    // 供启动参数、测试和截图工具使用
    bool openPath(const QString &path, bool showError = true);
    bool saveToPath(const QString &path);
    void applyDocument(const DocumentData &doc);

    bool findText(const QString &text, bool caseSensitive, bool forward,
                  bool showNotFound = false);
    int replaceAll(const QString &text, const QString &replacement,
                   bool caseSensitive);

    bool isPristine() const;
    bool isModified() const;
    QString currentFilePath() const;
    QString displayName() const;
    CodeEditor *editor() const;

    void setAutoSaveEnabled(bool enabled);
    bool autoSaveEnabled() const;
    void setAutoSaveInterval(int milliseconds);

public slots:
    void newFile();
    void openFile();
    bool saveFile();
    bool saveFileAs();
    void exitApplication();

    void showFindDialog();
    void showReplaceDialog();
    void showAbout();

    void autoSaveNow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void chooseFont();
    void chooseFontColor();
    void chooseTextBackground();
    void chooseEditorBackground();

    void onWordWrapToggled(bool checked);
    void onLineNumbersToggled(bool checked);
    void onToolBarToggled(bool checked);
    void onStatusBarToggled(bool checked);
    void onAutoSaveToggled(bool checked);

    void onFindRequested(const QString &text, bool caseSensitive, bool forward);
    void onReplaceRequested(const QString &text, const QString &replacement,
                            bool caseSensitive, bool forward);
    void onReplaceAllRequested(const QString &text, const QString &replacement,
                               bool caseSensitive);

    void updateStatusBar();
    void updateWindowTitle();
    void updatePasteAction();
    void updateEditActions();
    void autoSave();

private:
    void setupStatusBar();
    void setupConnections();
    void showStatusMessage(const QString &text, int timeout = 3000);
    bool maybeSave();
    void applyCharFormatToSelectionOrAll(const QTextCharFormat &format,
                                         const QString &what);
    QString selectedText() const;
    QString defaultSaveDir() const;
    QString suggestedSavePath() const;

    Ui::MainWindow *ui;
    QString m_filePath;
    TextEncoding m_encoding;
    FindDialog *m_findDialog;
    ReplaceDialog *m_replaceDialog;
    QLabel *m_statusLabel;
    QLabel *m_messageLabel;
    QLabel *m_nameLabel;
    QTimer *m_messageTimer;
    QTimer *m_autoSaveTimer;
    bool m_autoSaveEnabled;
    int m_untitledNumber;

    static int s_untitledCounter;
};

#endif // MAINWINDOW_H
