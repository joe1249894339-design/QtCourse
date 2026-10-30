#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QColor>

class LineNumberArea;

/**
 * QPlainTextEdit 派生类：提供行号栏、当前行高亮和编辑器背景色。
 * 在 mainwindow.ui 中通过 promote 将 QPlainTextEdit 提升为本类。
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit CodeEditor(QWidget *parent = nullptr);

    int lineNumberAreaWidth() const;
    void lineNumberAreaPaintEvent(QPaintEvent *event);

    bool lineNumbersVisible() const;
    void setLineNumbersVisible(bool visible);

    void setEditorBackground(const QColor &color);
    QColor editorBackground() const;

    void refreshLineNumberAreaWidth();

public slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &rect, int dy);
    void highlightCurrentLine();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    static bool isDarkColor(const QColor &color);
    void updateEditorColors();

    LineNumberArea *m_lineNumberArea;
    bool m_lineNumbersVisible;
    QColor m_editorBackground;
    QColor m_currentLineColor;
    QColor m_lineNumberBackground;
    QColor m_lineNumberTextColor;
    QColor m_currentLineNumberColor;
};

#endif // CODEEDITOR_H
