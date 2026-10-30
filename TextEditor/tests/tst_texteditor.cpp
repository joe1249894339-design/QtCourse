#include <QtTest>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMenu>
#include <QTextCursor>

#include "mainwindow.h"
#include "codeeditor.h"
#include "documentio.h"

class TestTextEditor : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void cleanupTestCase();
    void documentIoUtf8();
    void documentIoBinary();
    void documentIoMissing();
    void openPathAndTitle();
    void openMissingPath();
    void saveAndModifiedTitle();
    void actionEnableStates();
    void menuActionStates();
    void findWrapAndNotFound();
    void replaceCurrent();
    void replaceAllAndUndo();
    void autoSaveNamedFile();
    void lineNumbers();

private:
    QString writeFile(const QString &name, const QByteArray &bytes);
    QString m_dirPath;
};

QString TestTextEditor::writeFile(const QString &name, const QByteArray &bytes)
{
    const QString path = m_dirPath + QLatin1Char('/') + name;
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return QString();
    file.write(bytes);
    file.close();
    return path;
}

void TestTextEditor::initTestCase()
{
    m_dirPath = QDir::currentPath() + QStringLiteral("/tst_texteditor_data");
    QDir().mkpath(m_dirPath);
    QVERIFY2(QFileInfo(m_dirPath).isDir(), "cannot create test data directory");
}

void TestTextEditor::cleanupTestCase()
{
    QDir(m_dirPath).removeRecursively();
}

void TestTextEditor::documentIoUtf8()
{
    const QString text = QStringLiteral("第一行\nsecond line\n第三行");
    const QString path = writeFile(QStringLiteral("utf8.txt"), text.toUtf8());
    QVERIFY(!path.isEmpty());

    DocumentData data;
    QString error;
    QVERIFY2(DocumentIO::read(path, data, error), qPrintable(error));
    QCOMPARE(data.text, text);
    QVERIFY(data.encoding == TextEncoding::Utf8);
    QCOMPARE(data.path, QFileInfo(path).absoluteFilePath());

    const QString saved = m_dirPath + QStringLiteral("/utf8_saved.txt");
    QVERIFY2(DocumentIO::write(saved, data.text, data.encoding, error), qPrintable(error));

    DocumentData roundTrip;
    QVERIFY2(DocumentIO::read(saved, roundTrip, error), qPrintable(error));
    QCOMPARE(roundTrip.text, text);
}

void TestTextEditor::documentIoBinary()
{
    QByteArray bytes;
    bytes.append('A');
    bytes.append('\0');
    bytes.append('B');
    const QString path = writeFile(QStringLiteral("binary.bin"), bytes);
    QVERIFY(!path.isEmpty());

    DocumentData data;
    QString error;
    QVERIFY(!DocumentIO::read(path, data, error));
    QVERIFY(error.contains(QStringLiteral("二进制")));
}

void TestTextEditor::documentIoMissing()
{
    DocumentData data;
    QString error;
    QVERIFY(!DocumentIO::read(m_dirPath + QStringLiteral("/missing.txt"), data, error));
    QVERIFY(!error.isEmpty());
}

void TestTextEditor::openPathAndTitle()
{
    const QString text = QStringLiteral("open me\n第二行");
    const QString path = writeFile(QStringLiteral("open.txt"), text.toUtf8());
    QVERIFY(!path.isEmpty());

    MainWindow *window = new MainWindow;
    QVERIFY(window->openPath(path, false));
    QCOMPARE(window->editor()->toPlainText(), text);
    QVERIFY(!window->isModified());
    QVERIFY(window->windowTitle().contains(QStringLiteral("open.txt")));
    QVERIFY(!window->windowTitle().startsWith(QStringLiteral("*")));
    delete window;
}

void TestTextEditor::openMissingPath()
{
    MainWindow *window = new MainWindow;
    QVERIFY(!window->openPath(m_dirPath + QStringLiteral("/does_not_exist.txt"), false));
    QVERIFY(window->editor()->toPlainText().isEmpty());
    delete window;
}

void TestTextEditor::saveAndModifiedTitle()
{
    const QString path = m_dirPath + QStringLiteral("/saved.txt");
    MainWindow *window = new MainWindow;
    window->editor()->clear();
    window->editor()->insertPlainText(QStringLiteral("hello 世界"));

    QVERIFY(window->isModified());
    QVERIFY(window->windowTitle().startsWith(QStringLiteral("*")));

    QVERIFY(window->saveToPath(path));
    QVERIFY(!window->isModified());
    QVERIFY(!window->windowTitle().startsWith(QStringLiteral("*")));
    QVERIFY(window->windowTitle().contains(QStringLiteral("saved.txt")));

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(file.readAll()), QStringLiteral("hello 世界"));
    file.close();

    delete window;
}

void TestTextEditor::actionEnableStates()
{
    MainWindow *window = new MainWindow;
    QAction *cut = window->findChild<QAction *>(QStringLiteral("actionCut"));
    QAction *copy = window->findChild<QAction *>(QStringLiteral("actionCopy"));
    QAction *paste = window->findChild<QAction *>(QStringLiteral("actionPaste"));
    QAction *undo = window->findChild<QAction *>(QStringLiteral("actionUndo"));
    QAction *redo = window->findChild<QAction *>(QStringLiteral("actionRedo"));
    QVERIFY(cut && copy && paste && undo && redo);

    QVERIFY(!cut->isEnabled());
    QVERIFY(!copy->isEnabled());
    QVERIFY(!undo->isEnabled());
    QVERIFY(!redo->isEnabled());

    window->editor()->clear();
    window->editor()->insertPlainText(QStringLiteral("abcdef"));
    QTextCursor cursor = window->editor()->textCursor();
    cursor.setPosition(0);
    cursor.setPosition(3, QTextCursor::KeepAnchor);
    window->editor()->setTextCursor(cursor);
    QCoreApplication::processEvents();

    QVERIFY(cut->isEnabled());
    QVERIFY(copy->isEnabled());
    QVERIFY(undo->isEnabled());
    QVERIFY(!redo->isEnabled());

    window->editor()->undo();
    QCoreApplication::processEvents();
    QVERIFY(redo->isEnabled());

    QApplication::clipboard()->setText(QStringLiteral("clipboard"));
    QCoreApplication::processEvents();
    QVERIFY(paste->isEnabled());

    QApplication::clipboard()->clear();
    QCoreApplication::processEvents();
    QVERIFY(!paste->isEnabled());

    delete window;
}

void TestTextEditor::menuActionStates()
{
    MainWindow *window = new MainWindow;
    window->resize(600, 400);
    window->show();
    window->editor()->clear();
    window->editor()->insertPlainText(QStringLiteral("abcdef"));
    QTest::qWait(100);

    QTextCursor cursor = window->editor()->textCursor();
    cursor.setPosition(0);
    cursor.setPosition(3, QTextCursor::KeepAnchor);
    window->editor()->setTextCursor(cursor);
    QTest::qWait(100);

    QAction *cut = window->findChild<QAction *>(QStringLiteral("actionCut"));
    QVERIFY(cut);
    QVERIFY(cut->isEnabled());
    QVERIFY(window->editor()->textCursor().hasSelection());

    QMenu *menu = window->findChild<QMenu *>(QStringLiteral("menuEdit"));
    QVERIFY(menu);
    menu->popup(QPoint(20, 20));
    QTest::qWait(150);

    qInfo() << "after popup: selection=" << window->editor()->textCursor().hasSelection()
            << "cut=" << cut->isEnabled();
    QVERIFY(cut->isEnabled());

    menu->hide();
    QTest::qWait(50);
    delete window;
}

void TestTextEditor::findWrapAndNotFound()
{
    MainWindow *window = new MainWindow;
    window->editor()->setPlainText(QStringLiteral("abc abc\nabc"));

    QVERIFY(window->findText(QStringLiteral("abc"), false, true, false));
    QCOMPARE(window->editor()->textCursor().selectionStart(), 0);

    QVERIFY(window->findText(QStringLiteral("abc"), false, true, false));
    QCOMPARE(window->editor()->textCursor().selectionStart(), 4);

    QVERIFY(window->findText(QStringLiteral("abc"), false, true, false));
    QCOMPARE(window->editor()->textCursor().selectionStart(), 8);

    // 第四次应循环回到文件开头
    QVERIFY(window->findText(QStringLiteral("abc"), false, true, false));
    QCOMPARE(window->editor()->textCursor().selectionStart(), 0);

    QVERIFY(!window->findText(QStringLiteral("zzz"), false, true, false));

    // 区分大小写
    window->editor()->setPlainText(QStringLiteral("Abc abc"));
    QTextCursor start = window->editor()->textCursor();
    start.setPosition(0);
    window->editor()->setTextCursor(start);
    QVERIFY(window->findText(QStringLiteral("abc"), true, true, false));
    QCOMPARE(window->editor()->textCursor().selectionStart(), 4);
    QVERIFY(!window->findText(QStringLiteral("ABC"), true, true, false));

    delete window;
}

void TestTextEditor::replaceCurrent()
{
    MainWindow *window = new MainWindow;
    window->editor()->setPlainText(QStringLiteral("one two one"));

    QTextCursor cursor = window->editor()->textCursor();
    cursor.setPosition(0);
    cursor.setPosition(3, QTextCursor::KeepAnchor);
    window->editor()->setTextCursor(cursor);

    QVERIFY(QMetaObject::invokeMethod(
        window, "onReplaceRequested", Qt::DirectConnection,
        Q_ARG(QString, QStringLiteral("one")),
        Q_ARG(QString, QStringLiteral("1")),
        Q_ARG(bool, false),
        Q_ARG(bool, true)));

    QCOMPARE(window->editor()->toPlainText(), QStringLiteral("1 two one"));
    QCOMPARE(window->editor()->textCursor().selectedText(), QStringLiteral("one"));

    delete window;
}

void TestTextEditor::replaceAllAndUndo()
{
    MainWindow *window = new MainWindow;
    window->editor()->setPlainText(QStringLiteral("one two one two one"));

    QCOMPARE(window->replaceAll(QStringLiteral("one"), QStringLiteral("1"), false), 3);
    QCOMPARE(window->editor()->toPlainText(), QStringLiteral("1 two 1 two 1"));
    QCOMPARE(window->replaceAll(QStringLiteral("zzz"), QStringLiteral("x"), false), 0);
    QCOMPARE(window->replaceAll(QString(), QStringLiteral("x"), false), 0);

    QVERIFY(window->editor()->document()->isUndoAvailable());
    window->editor()->undo();
    QCOMPARE(window->editor()->toPlainText(), QStringLiteral("one two one two one"));

    delete window;
}

void TestTextEditor::autoSaveNamedFile()
{
    const QString path = m_dirPath + QStringLiteral("/autosave.txt");
    MainWindow *window = new MainWindow;
    window->editor()->clear();
    window->editor()->insertPlainText(QStringLiteral("first"));
    QVERIFY(window->saveToPath(path));

    window->editor()->moveCursor(QTextCursor::End);
    window->editor()->insertPlainText(QStringLiteral(" second"));
    QVERIFY(window->isModified());

    window->autoSaveNow();
    QVERIFY(!window->isModified());

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly));
    QCOMPARE(QString::fromUtf8(file.readAll()), QStringLiteral("first second"));
    file.close();

    delete window;
}

void TestTextEditor::lineNumbers()
{
    MainWindow *window = new MainWindow;
    const int oneDigitWidth = window->editor()->lineNumberAreaWidth();
    QVERIFY(oneDigitWidth > 0);

    QString text;
    for (int i = 0; i < 120; ++i)
        text += QStringLiteral("line %1\n").arg(i + 1);
    window->editor()->setPlainText(text);
    QCoreApplication::processEvents();

    const int threeDigitWidth = window->editor()->lineNumberAreaWidth();
    QVERIFY(threeDigitWidth > oneDigitWidth);

    window->editor()->setLineNumbersVisible(false);
    QCOMPARE(window->editor()->lineNumberAreaWidth(), 0);

    window->editor()->setLineNumbersVisible(true);
    QVERIFY(window->editor()->lineNumberAreaWidth() > 0);

    delete window;
}

QTEST_MAIN(TestTextEditor)
#include "tst_texteditor.moc"
