#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "codeeditor.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFont>
#include <QFontDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QPalette>
#include <QPushButton>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStatusBar>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTime>
#include <QTimer>
#include <QToolBar>
#include <QWidget>

int MainWindow::s_untitledCounter = 0;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_encoding(TextEncoding::Utf8)
    , m_findDialog(nullptr)
    , m_replaceDialog(nullptr)
    , m_statusLabel(nullptr)
    , m_messageLabel(nullptr)
    , m_nameLabel(nullptr)
    , m_messageTimer(nullptr)
    , m_autoSaveTimer(nullptr)
    , m_autoSaveEnabled(true)
    , m_untitledNumber(0)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);
    m_untitledNumber = ++s_untitledCounter;

    setupStatusBar();
    setupConnections();

    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(60000);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start();

    updateWindowTitle();
    updateStatusBar();
    updatePasteAction();
    ui->editor->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupStatusBar()
{
    // 用一个浅色内容控件铺满 QStatusBar，避免 Windows 深色主题把状态栏画成黑底。
    QWidget *content = new QWidget(ui->statusbar);
    content->setObjectName(QStringLiteral("statusContent"));
    content->setAutoFillBackground(true);
    content->setStyleSheet(QStringLiteral("background-color: #f3f4f6;"));
    QPalette contentPalette = content->palette();
    contentPalette.setColor(QPalette::Window, QColor(243, 244, 246));
    contentPalette.setColor(QPalette::WindowText, QColor(34, 34, 34));
    content->setPalette(contentPalette);

    QFont statusFont = font();
    statusFont.setPointSize(9);
    content->setFont(statusFont);

    QHBoxLayout *layout = new QHBoxLayout(content);
    layout->setContentsMargins(8, 0, 8, 0);
    layout->setSpacing(12);

    m_statusLabel = new QLabel(content);
    m_statusLabel->setObjectName(QStringLiteral("statusLabel"));
    m_statusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_statusLabel->setStyleSheet(QStringLiteral("color: #333333; background: transparent;"));

    m_messageLabel = new QLabel(content);
    m_messageLabel->setObjectName(QStringLiteral("messageLabel"));
    m_messageLabel->setStyleSheet(QStringLiteral("color: #666666; background: transparent;"));
    m_messageLabel->setVisible(false);

    m_nameLabel = new QLabel(QStringLiteral("黄俊豪"), content);
    m_nameLabel->setObjectName(QStringLiteral("nameLabel"));
    m_nameLabel->setStyleSheet(QStringLiteral("color: #333333; background: transparent; padding-right: 4px;"));

    layout->addWidget(m_statusLabel, 1);
    layout->addWidget(m_messageLabel, 1);
    layout->addWidget(m_nameLabel, 0);

    ui->statusbar->addWidget(content, 1);
    ui->statusbar->setSizeGripEnabled(false);
    ui->statusbar->setStyleSheet(QStringLiteral(
        "QStatusBar { background-color: #f3f4f6; border-top: 1px solid #d9d9d9; }\n"
        "QStatusBar::item { border: none; }"));
    if (ui->statusbar->layout())
        ui->statusbar->layout()->setContentsMargins(0, 0, 0, 0);

    m_messageTimer = new QTimer(this);
    m_messageTimer->setSingleShot(true);
    connect(m_messageTimer, &QTimer::timeout, this, [this]() {
        if (m_messageLabel) {
            m_messageLabel->clear();
            m_messageLabel->setVisible(false);
        }
    });
}

void MainWindow::showStatusMessage(const QString &text, int timeout)
{
    if (!m_messageLabel || !m_messageTimer)
        return;
    m_messageLabel->setText(text);
    m_messageLabel->setVisible(true);
    m_messageTimer->start(qMax(500, timeout));
}

void MainWindow::setupConnections()
{
    connect(ui->actionNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actionOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actionSave, &QAction::triggered, this, [this]() { saveFile(); });
    connect(ui->actionSaveAs, &QAction::triggered, this, [this]() { saveFileAs(); });
    connect(ui->actionExit, &QAction::triggered, this, &MainWindow::exitApplication);

    connect(ui->actionUndo, &QAction::triggered, ui->editor, &QPlainTextEdit::undo);
    connect(ui->actionRedo, &QAction::triggered, ui->editor, &QPlainTextEdit::redo);
    connect(ui->actionCut, &QAction::triggered, ui->editor, &QPlainTextEdit::cut);
    connect(ui->actionCopy, &QAction::triggered, ui->editor, &QPlainTextEdit::copy);
    connect(ui->actionPaste, &QAction::triggered, ui->editor, &QPlainTextEdit::paste);
    connect(ui->actionSelectAll, &QAction::triggered, ui->editor, &QPlainTextEdit::selectAll);

    connect(ui->actionFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actionReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);
    connect(ui->actionFont, &QAction::triggered, this, &MainWindow::chooseFont);
    connect(ui->actionFontColor, &QAction::triggered, this, &MainWindow::chooseFontColor);
    connect(ui->actionTextBackground, &QAction::triggered, this, &MainWindow::chooseTextBackground);
    connect(ui->actionEditorBackground, &QAction::triggered, this, &MainWindow::chooseEditorBackground);

    connect(ui->actionWordWrap, &QAction::toggled, this, &MainWindow::onWordWrapToggled);
    connect(ui->actionLineNumbers, &QAction::toggled, this, &MainWindow::onLineNumbersToggled);
    connect(ui->actionToolBar, &QAction::toggled, this, &MainWindow::onToolBarToggled);
    connect(ui->actionStatusBar, &QAction::toggled, this, &MainWindow::onStatusBarToggled);
    connect(ui->actionAutoSave, &QAction::toggled, this, &MainWindow::onAutoSaveToggled);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::showAbout);
    connect(ui->menuEdit, &QMenu::aboutToShow, this, &MainWindow::updateEditActions);

    connect(ui->editor, &QPlainTextEdit::undoAvailable,
            ui->actionUndo, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::redoAvailable,
            ui->actionRedo, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::copyAvailable,
            ui->actionCut, &QAction::setEnabled);
    connect(ui->editor, &QPlainTextEdit::copyAvailable,
            ui->actionCopy, &QAction::setEnabled);
    connect(QApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updatePasteAction);

    connect(ui->editor->document(), &QTextDocument::modificationChanged,
            this, &MainWindow::updateWindowTitle);
    connect(ui->editor, &QPlainTextEdit::textChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged,
            this, &MainWindow::updateStatusBar);
    connect(ui->editor->document(), &QTextDocument::blockCountChanged,
            this, &MainWindow::updateStatusBar);

    ui->actionUndo->setEnabled(false);
    ui->actionRedo->setEnabled(false);
    ui->actionCut->setEnabled(false);
    ui->actionCopy->setEnabled(false);
    updatePasteAction();
}

void MainWindow::newFile()
{
    // B 方案：每个文档一个独立顶层窗口，新建总是再开一个窗口。
    MainWindow *window = new MainWindow;
    window->move(pos() + QPoint(32, 32));
    window->show();
    window->raise();
    window->activateWindow();
}

void MainWindow::openFile()
{
    const QString path = QFileDialog::getOpenFileName(
        this, tr("打开文件"), defaultSaveDir(),
        tr("文本文件 (*.txt *.md *.log *.ini *.json *.cpp *.h);;所有文件 (*.*)"));
    if (path.isEmpty())
        return;

    DocumentData doc;
    QString error;
    if (!DocumentIO::read(path, doc, error)) {
        QMessageBox::warning(this, tr("打开失败"), error);
        return;
    }

    if (isPristine()) {
        applyDocument(doc);
    } else {
        MainWindow *window = new MainWindow;
        window->applyDocument(doc);
        window->move(pos() + QPoint(32, 32));
        window->show();
        window->raise();
        window->activateWindow();
    }
}

bool MainWindow::openPath(const QString &path, bool showError)
{
    DocumentData doc;
    QString error;
    if (!DocumentIO::read(path, doc, error)) {
        if (showError)
            QMessageBox::warning(this, tr("打开失败"), error);
        return false;
    }

    applyDocument(doc);
    return true;
}

void MainWindow::applyDocument(const DocumentData &doc)
{
    QSignalBlocker blocker(ui->editor->document());

    ui->editor->setPlainText(doc.text);
    m_filePath = doc.path;
    m_encoding = doc.encoding;
    ui->editor->document()->setModified(false);
    ui->editor->document()->clearUndoRedoStacks();
    ui->editor->moveCursor(QTextCursor::Start);

    blocker.unblock();

    updateWindowTitle();
    updateStatusBar();
    updatePasteAction();
    ui->editor->setFocus();
}

bool MainWindow::saveFile()
{
    if (m_filePath.isEmpty())
        return saveFileAs();
    return saveToPath(m_filePath);
}

bool MainWindow::saveFileAs()
{
    QString path = QFileDialog::getSaveFileName(
        this, tr("另存为"), suggestedSavePath(),
        tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (path.isEmpty())
        return false;

    if (QFileInfo(path).suffix().isEmpty())
        path += QStringLiteral(".txt");

    return saveToPath(path);
}

bool MainWindow::saveToPath(const QString &path)
{
    QString error;
    if (!DocumentIO::write(path, ui->editor->toPlainText(), m_encoding, error)) {
        QMessageBox::warning(this, tr("保存失败"), error);
        return false;
    }

    m_filePath = QFileInfo(path).absoluteFilePath();
    ui->editor->document()->setModified(false);
    updateWindowTitle();
    showStatusMessage(
        tr("已保存：%1").arg(QDir::toNativeSeparators(m_filePath)), 3000);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!isModified())
        return true;

    QMessageBox box(this);
    box.setWindowTitle(tr("未保存的更改"));
    box.setIcon(QMessageBox::Warning);
    box.setText(tr("文件 \"%1\" 已被修改，是否保存？").arg(displayName()));
    box.setInformativeText(tr("如果不保存，所做的修改将会丢失。"));

    QPushButton *saveButton = box.addButton(tr("保存"), QMessageBox::AcceptRole);
    QPushButton *discardButton = box.addButton(tr("不保存"), QMessageBox::DestructiveRole);
    box.addButton(tr("取消"), QMessageBox::RejectRole);
    box.setDefaultButton(saveButton);
    box.exec();

    if (box.clickedButton() == saveButton)
        return saveFile();
    if (box.clickedButton() == discardButton)
        return true;

    // 点右上角关闭或按 Esc 也按“取消”处理。
    return false;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

void MainWindow::exitApplication()
{
    const QWidgetList windows = QApplication::topLevelWidgets();
    for (QWidget *widget : windows) {
        if (auto *window = qobject_cast<MainWindow *>(widget)) {
            if (!window->close())
                return;
        }
    }
}

void MainWindow::showFindDialog()
{
    if (!m_findDialog) {
        m_findDialog = new FindDialog(this);
        connect(m_findDialog, &FindDialog::findRequested,
                this, &MainWindow::onFindRequested);
    }

    m_findDialog->setSearchText(selectedText());
    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    if (!m_replaceDialog) {
        m_replaceDialog = new ReplaceDialog(this);
        connect(m_replaceDialog, &ReplaceDialog::findRequested,
                this, &MainWindow::onFindRequested);
        connect(m_replaceDialog, &ReplaceDialog::replaceRequested,
                this, &MainWindow::onReplaceRequested);
        connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested,
                this, &MainWindow::onReplaceAllRequested);
    }

    m_replaceDialog->setSearchText(selectedText());
    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

void MainWindow::showAbout()
{
    AboutDialog dialog(this);
    dialog.exec();
}

void MainWindow::onFindRequested(const QString &text, bool caseSensitive, bool forward)
{
    findText(text, caseSensitive, forward, true);
}

bool MainWindow::findText(const QString &text, bool caseSensitive, bool forward,
                          bool showNotFound)
{
    if (text.isEmpty()) {
        if (showNotFound)
            QMessageBox::information(this, tr("查找"), tr("请输入查找内容。"));
        return false;
    }

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (!forward)
        flags |= QTextDocument::FindBackward;

    QTextCursor from = ui->editor->textCursor();
    if (from.hasSelection())
        from.setPosition(forward ? from.selectionEnd() : from.selectionStart());

    QTextCursor found = ui->editor->document()->find(text, from, flags);
    bool wrapped = false;

    if (found.isNull()) {
        QTextCursor wrapCursor(ui->editor->document());
        if (forward)
            wrapCursor.movePosition(QTextCursor::Start);
        else
            wrapCursor.movePosition(QTextCursor::End);
        found = ui->editor->document()->find(text, wrapCursor, flags);
        wrapped = !found.isNull();
    }

    if (found.isNull()) {
        if (showNotFound) {
            QMessageBox::information(
                this, tr("查找"), tr("未找到 \"%1\"。").arg(text));
        }
        return false;
    }

    ui->editor->setTextCursor(found);
    ui->editor->ensureCursorVisible();

    if (wrapped) {
        showStatusMessage(
            forward ? tr("已从文件开头继续查找") : tr("已从文件末尾继续查找"), 2500);
    }
    return true;
}

void MainWindow::onReplaceRequested(const QString &text, const QString &replacement,
                                    bool caseSensitive, bool forward)
{
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入查找内容。"));
        return;
    }

    QTextCursor cursor = ui->editor->textCursor();
    bool selectionMatches = false;
    if (cursor.hasSelection()) {
        const Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive
                                                     : Qt::CaseInsensitive;
        selectionMatches = (QString::compare(cursor.selectedText(), text, cs) == 0);
    }

    if (selectionMatches) {
        cursor.insertText(replacement);
        ui->editor->setTextCursor(cursor);
    }

    findText(text, caseSensitive, forward, true);
}

int MainWindow::replaceAll(const QString &text, const QString &replacement,
                           bool caseSensitive)
{
    if (text.isEmpty())
        return 0;

    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;

    QTextCursor startCursor(ui->editor->document());
    startCursor.movePosition(QTextCursor::Start);
    QTextCursor found = ui->editor->document()->find(text, startCursor, flags);
    if (found.isNull())
        return 0;

    const int originalPosition = ui->editor->textCursor().position();

    QTextCursor editBlock(ui->editor->document());
    editBlock.beginEditBlock();

    int count = 0;
    while (!found.isNull()) {
        found.insertText(replacement);
        ++count;
        found = ui->editor->document()->find(text, found, flags);
    }

    editBlock.endEditBlock();

    QTextCursor restore(ui->editor->document());
    restore.setPosition(qMin(originalPosition,
                             qMax(0, ui->editor->document()->characterCount() - 1)));
    ui->editor->setTextCursor(restore);

    return count;
}

void MainWindow::onReplaceAllRequested(const QString &text, const QString &replacement,
                                       bool caseSensitive)
{
    if (text.isEmpty()) {
        QMessageBox::information(this, tr("替换"), tr("请输入查找内容。"));
        return;
    }

    const int count = replaceAll(text, replacement, caseSensitive);
    if (count == 0) {
        QMessageBox::information(
            this, tr("替换"), tr("未找到 \"%1\"。").arg(text));
    } else {
        QMessageBox::information(
            this, tr("替换"), tr("共替换 %1 处。").arg(count));
    }
}

void MainWindow::chooseFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(
        &ok, ui->editor->font(), this, tr("选择字体"));
    if (!ok)
        return;

    QTextCharFormat format;
    format.setFont(font);
    applyCharFormatToSelectionOrAll(format, tr("字体"));

    if (!ui->editor->textCursor().hasSelection())
        ui->editor->setFont(font);

    ui->editor->refreshLineNumberAreaWidth();
}

void MainWindow::chooseFontColor()
{
    QColor initial = ui->editor->currentCharFormat().foreground().color();
    if (!initial.isValid())
        initial = Qt::black;

    const QColor color = QColorDialog::getColor(
        initial, this, tr("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setForeground(color);
    applyCharFormatToSelectionOrAll(format, tr("字体颜色"));
}

void MainWindow::chooseTextBackground()
{
    QColor initial = ui->editor->currentCharFormat().background().color();
    if (!initial.isValid())
        initial = Qt::white;

    const QColor color = QColorDialog::getColor(
        initial, this, tr("选择字体背景色"));
    if (!color.isValid())
        return;

    QTextCharFormat format;
    format.setBackground(color);
    applyCharFormatToSelectionOrAll(format, tr("字体背景色"));
}

void MainWindow::chooseEditorBackground()
{
    const QColor color = QColorDialog::getColor(
        ui->editor->editorBackground(), this, tr("选择编辑器背景色"));
    if (!color.isValid())
        return;

    ui->editor->setEditorBackground(color);
}

void MainWindow::applyCharFormatToSelectionOrAll(const QTextCharFormat &format,
                                                 const QString &what)
{
    QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection()) {
        cursor.mergeCharFormat(format);
        ui->editor->mergeCurrentCharFormat(format);
        ui->editor->setTextCursor(cursor);
        return;
    }

    const int position = cursor.position();
    QTextCursor all(ui->editor->document());
    all.select(QTextCursor::Document);
    all.mergeCharFormat(format);

    QTextCursor restore(ui->editor->document());
    restore.setPosition(qMin(position,
                             qMax(0, ui->editor->document()->characterCount() - 1)));
    ui->editor->setTextCursor(restore);
    ui->editor->setCurrentCharFormat(format);

    showStatusMessage(
        tr("未选中文本，已把%1应用到全文").arg(what), 3000);
}

void MainWindow::onWordWrapToggled(bool checked)
{
    ui->editor->setLineWrapMode(checked ? QPlainTextEdit::WidgetWidth
                                        : QPlainTextEdit::NoWrap);
    ui->editor->highlightCurrentLine();
    ui->editor->viewport()->update();
    showStatusMessage(checked ? tr("自动换行：开") : tr("自动换行：关"), 1500);
}

void MainWindow::onLineNumbersToggled(bool checked)
{
    ui->editor->setLineNumbersVisible(checked);
    showStatusMessage(checked ? tr("行号：显示") : tr("行号：隐藏"), 1500);
}

void MainWindow::onToolBarToggled(bool checked)
{
    ui->toolBar->setVisible(checked);
}

void MainWindow::onStatusBarToggled(bool checked)
{
    ui->statusbar->setVisible(checked);
}

void MainWindow::setAutoSaveEnabled(bool enabled)
{
    if (ui->actionAutoSave->isChecked() != enabled) {
        QSignalBlocker blocker(ui->actionAutoSave);
        ui->actionAutoSave->setChecked(enabled);
    }

    m_autoSaveEnabled = enabled;
    if (!m_autoSaveTimer)
        return;
    if (enabled)
        m_autoSaveTimer->start();
    else
        m_autoSaveTimer->stop();
}

bool MainWindow::autoSaveEnabled() const
{
    return m_autoSaveEnabled;
}

void MainWindow::setAutoSaveInterval(int milliseconds)
{
    if (!m_autoSaveTimer)
        return;
    m_autoSaveTimer->setInterval(qMax(1000, milliseconds));
    if (m_autoSaveEnabled)
        m_autoSaveTimer->start();
}

void MainWindow::onAutoSaveToggled(bool checked)
{
    setAutoSaveEnabled(checked);
    showStatusMessage(
        checked ? tr("自动保存：开（每 60 秒）") : tr("自动保存：关"), 2000);
}

void MainWindow::autoSaveNow()
{
    autoSave();
}

void MainWindow::autoSave()
{
    if (!m_autoSaveEnabled || !isModified() || m_filePath.isEmpty())
        return;

    QString error;
    if (!DocumentIO::write(m_filePath, ui->editor->toPlainText(), m_encoding, error)) {
        showStatusMessage(tr("自动保存失败：%1").arg(error), 5000);
        return;
    }

    ui->editor->document()->setModified(false);
    updateWindowTitle();
    showStatusMessage(
        tr("已自动保存 %1").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss"))),
        3000);
}

void MainWindow::updateStatusBar()
{
    if (!m_statusLabel)
        return;

    const QTextCursor cursor = ui->editor->textCursor();
    const int length = ui->editor->toPlainText().size();
    const int lines = ui->editor->document()->blockCount();
    const int line = cursor.blockNumber() + 1;
    const int column = cursor.positionInBlock() + 1;

    m_statusLabel->setText(
        tr("length: %1    lines: %2    Ln: %3   Col: %4")
            .arg(length)
            .arg(lines)
            .arg(line)
            .arg(column));
}

void MainWindow::updateWindowTitle()
{
    // 手动把 * 放在标题最前面，和参考截图保持一致。
    const QString prefix = isModified() ? QStringLiteral("* ") : QString();
    setWindowTitle(prefix + displayName() + QStringLiteral(" - 编辑器"));
}

void MainWindow::updatePasteAction()
{
    const QMimeData *mimeData = QApplication::clipboard()->mimeData();
    ui->actionPaste->setEnabled(mimeData && mimeData->hasText());
}

void MainWindow::updateEditActions()
{
    // 打开“编辑”菜单时再同步一次，避免焦点变化导致使能状态滞后。
    const bool hasSelection = ui->editor->textCursor().hasSelection();
    ui->actionCut->setEnabled(hasSelection);
    ui->actionCopy->setEnabled(hasSelection);
    ui->actionUndo->setEnabled(ui->editor->document()->isUndoAvailable());
    ui->actionRedo->setEnabled(ui->editor->document()->isRedoAvailable());
    updatePasteAction();
}

bool MainWindow::isPristine() const
{
    return m_filePath.isEmpty()
        && !isModified()
        && ui->editor->toPlainText().isEmpty();
}

bool MainWindow::isModified() const
{
    return ui->editor->document()->isModified();
}

QString MainWindow::currentFilePath() const
{
    return m_filePath;
}

QString MainWindow::displayName() const
{
    if (!m_filePath.isEmpty())
        return QFileInfo(m_filePath).fileName();

    if (m_untitledNumber <= 1)
        return tr("新建文本文件");
    return tr("新建文本文件 %1").arg(m_untitledNumber);
}

CodeEditor *MainWindow::editor() const
{
    return ui->editor;
}

QString MainWindow::selectedText() const
{
    const QTextCursor cursor = ui->editor->textCursor();
    return cursor.hasSelection() ? cursor.selectedText() : QString();
}

QString MainWindow::defaultSaveDir() const
{
    if (!m_filePath.isEmpty())
        return QFileInfo(m_filePath).absolutePath();

    QString dir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    if (dir.isEmpty() || !QDir(dir).exists())
        dir = QDir::homePath();
    return dir;
}

QString MainWindow::suggestedSavePath() const
{
    if (!m_filePath.isEmpty())
        return m_filePath;
    return QDir(defaultSaveDir()).filePath(QStringLiteral("新建文本文件.txt"));
}
