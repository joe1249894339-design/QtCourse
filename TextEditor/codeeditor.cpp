#include "codeeditor.h"

#include <QPainter>
#include <QTextBlock>
#include <QTextEdit>
#include <QTextFormat>
#include <QResizeEvent>
#include <QScrollBar>

/**
 * 行号栏是编辑器内部的一个子控件，不参与外层布局。
 * 这样滚动、换行、字体变化时都能和文本保持同步。
 */
class LineNumberArea : public QWidget
{
public:
    explicit LineNumberArea(CodeEditor *editor)
        : QWidget(editor), m_codeEditor(editor)
    {
        setCursor(Qt::ArrowCursor);
    }

    QSize sizeHint() const override
    {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    CodeEditor *m_codeEditor;
};

CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent)
    , m_lineNumberArea(nullptr)
    , m_lineNumbersVisible(true)
    , m_editorBackground(Qt::white)
{
    setFrameShape(QFrame::NoFrame);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setTabStopDistance(fontMetrics().horizontalAdvance(QLatin1Char(' ')) * 4);
    setCursorWidth(2);

    m_lineNumberArea = new LineNumberArea(this);

    connect(this, &CodeEditor::blockCountChanged,
            this, &CodeEditor::updateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest,
            this, &CodeEditor::updateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged,
            this, &CodeEditor::highlightCurrentLine);

    updateEditorColors();
    updateLineNumberAreaWidth(0);
    highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() const
{
    if (!m_lineNumbersVisible)
        return 0;

    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        ++digits;
    }

    // 左右留白 + 数字宽度，保证 1、10、100 行都不会挤在一起
    return 12 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (!m_lineNumberArea)
        return;

    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), m_lineNumberBackground);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());
    const int currentBlock = textCursor().blockNumber();

    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            const bool isCurrent = (blockNumber == currentBlock);
            QFont numberFont = font();
            numberFont.setBold(isCurrent);
            painter.setFont(numberFont);
            painter.setPen(isCurrent ? m_currentLineNumberColor : m_lineNumberTextColor);
            painter.drawText(0, top,
                             m_lineNumberArea->width() - 6,
                             fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter,
                             QString::number(blockNumber + 1));
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

void CodeEditor::updateLineNumberAreaWidth(int newBlockCount)
{
    Q_UNUSED(newBlockCount);
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (!m_lineNumberArea || !m_lineNumbersVisible)
        return;

    if (dy != 0)
        m_lineNumberArea->scroll(0, dy);
    else
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        updateLineNumberAreaWidth(0);
}

void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);

    if (!m_lineNumberArea)
        return;

    const QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(),
                                        lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::highlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> selections;

    if (!isReadOnly()) {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(m_currentLineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }

    setExtraSelections(selections);
    if (m_lineNumberArea)
        m_lineNumberArea->update();
}

void CodeEditor::setLineNumbersVisible(bool visible)
{
    m_lineNumbersVisible = visible;
    if (m_lineNumberArea)
        m_lineNumberArea->setVisible(visible);
    updateLineNumberAreaWidth(0);
}

bool CodeEditor::lineNumbersVisible() const
{
    return m_lineNumbersVisible;
}

void CodeEditor::setEditorBackground(const QColor &color)
{
    if (!color.isValid())
        return;

    m_editorBackground = color;
    updateEditorColors();
    highlightCurrentLine();
    if (m_lineNumberArea)
        m_lineNumberArea->update();
    viewport()->update();
}

QColor CodeEditor::editorBackground() const
{
    return m_editorBackground;
}

void CodeEditor::refreshLineNumberAreaWidth()
{
    updateLineNumberAreaWidth(0);
    if (m_lineNumberArea)
        m_lineNumberArea->update();
}

bool CodeEditor::isDarkColor(const QColor &color)
{
    return color.lightness() < 128;
}

void CodeEditor::updateEditorColors()
{
    const bool dark = isDarkColor(m_editorBackground);

    QPalette pal = palette();
    pal.setColor(QPalette::Base, m_editorBackground);
    pal.setColor(QPalette::Text, dark ? QColor(236, 238, 240) : QColor(24, 26, 30));
    setPalette(pal);

    m_lineNumberBackground = dark ? m_editorBackground.lighter(135)
                                  : QColor(243, 244, 246);
    m_lineNumberTextColor = dark ? QColor(176, 182, 190)
                                 : QColor(122, 128, 136);
    m_currentLineNumberColor = dark ? QColor(255, 255, 255)
                                    : QColor(20, 22, 26);
    m_currentLineColor = dark ? QColor(255, 255, 0, 55)
                              : QColor(255, 246, 170);
}
