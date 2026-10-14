#include "calculator.h"

#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
{
    buildUi();

    setWindowTitle(QString::fromUtf8("计算器"));
    setFixedSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    setFocusPolicy(Qt::StrongFocus); // 让窗口本身能接收键盘事件
    setFocus();
}

// ---------------------------------------------------------------------------
// 界面构建
// ---------------------------------------------------------------------------
void Calculator::buildUi()
{
    // ---- 顶部显示区：表达式提示 + 主显示 ----
    m_expressionLabel = new QLabel(this);
    m_expressionLabel->setObjectName("expressionLabel");
    m_expressionLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_displayLabel = new QLabel("0", this);
    m_displayLabel->setObjectName("displayLabel");
    m_displayLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    // ---- 创建按钮 ----
    QPushButton *btnAC   = createButton("AC", "function");                        // 全部清除
    QPushButton *btnBack = createButton(QString(QChar(0x232B)), "function");       // 退格 ⌫
    QPushButton *btnDiv  = createButton(operatorSymbol(Operator::Divide),   "operator");   // ÷
    QPushButton *btnMul  = createButton(operatorSymbol(Operator::Multiply), "operator");   // ×
    QPushButton *btnSub  = createButton("-", "operator");
    QPushButton *btnAdd  = createButton("+", "operator");
    QPushButton *btnDot  = createButton(".", "number");
    QPushButton *btnEq   = createButton("=", "operator");

    // 数字按钮 0~9：循环创建并连接，避免逐个重复
    QPushButton *btnDigits[10];
    for (int i = 0; i <= 9; ++i) {
        btnDigits[i] = createButton(QString::number(i), "number");
        const int digit = i; // 复制一份供 lambda 按值捕获
        connect(btnDigits[i], &QPushButton::clicked, this,
                [this, digit] { inputDigit(digit); });
    }

    // ---- 布局：5 行 × 4 列（参考 iPhone）----
    // 布局示意：
    //   AC   ⌫   ÷   ×
    //   7    8   9   -
    //   4    5   6   +
    //   1    2   3   =(跨两行)
    //   0(跨两列) .  =(续)
    QGridLayout *grid = new QGridLayout;
    grid->setSpacing(10);
    grid->addWidget(btnAC,        0, 0);
    grid->addWidget(btnBack,      0, 1);
    grid->addWidget(btnDiv,       0, 2);
    grid->addWidget(btnMul,       0, 3);
    grid->addWidget(btnDigits[7], 1, 0);
    grid->addWidget(btnDigits[8], 1, 1);
    grid->addWidget(btnDigits[9], 1, 2);
    grid->addWidget(btnSub,       1, 3);
    grid->addWidget(btnDigits[4], 2, 0);
    grid->addWidget(btnDigits[5], 2, 1);
    grid->addWidget(btnDigits[6], 2, 2);
    grid->addWidget(btnAdd,       2, 3);
    grid->addWidget(btnDigits[1], 3, 0);
    grid->addWidget(btnDigits[2], 3, 1);
    grid->addWidget(btnDigits[3], 3, 2);
    grid->addWidget(btnEq,        3, 3, 2, 1); // “=”竖跨两行
    grid->addWidget(btnDigits[0], 4, 0, 1, 2); // “0”横跨两列
    grid->addWidget(btnDot,       4, 2);

    // 跨格按钮需要更大的尺寸
    btnDigits[0]->setFixedSize(SPAN_SIZE, BUTTON_SIZE);
    btnEq->setFixedSize(BUTTON_SIZE, SPAN_SIZE);

    // ---- 信号连接：鼠标点击 → 与键盘共用同一套处理函数 ----
    connect(btnAC,   &QPushButton::clicked, this, [this] { clearAll(); });
    connect(btnBack, &QPushButton::clicked, this, [this] { backspace(); });
    connect(btnDiv,  &QPushButton::clicked, this, [this] { inputOperator(Operator::Divide); });
    connect(btnMul,  &QPushButton::clicked, this, [this] { inputOperator(Operator::Multiply); });
    connect(btnSub,  &QPushButton::clicked, this, [this] { inputOperator(Operator::Subtract); });
    connect(btnAdd,  &QPushButton::clicked, this, [this] { inputOperator(Operator::Add); });
    connect(btnDot,  &QPushButton::clicked, this, [this] { inputDecimalPoint(); });
    connect(btnEq,   &QPushButton::clicked, this, [this] { inputEquals(); });

    // ---- 整体布局 ----
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 24, 16, 16);
    mainLayout->setSpacing(12);
    mainLayout->addWidget(m_expressionLabel);
    mainLayout->addWidget(m_displayLabel);
    mainLayout->addStretch(); // 把按钮区推到窗口底部
    mainLayout->addLayout(grid);

    // ---- 全局样式（iPhone 深色风格）----
    setStyleSheet(
        "QWidget { background-color: #000000; }"
        "QLabel#expressionLabel { color: #8E8E93; font-size: 20px; padding-right: 4px; }"
        "QLabel#displayLabel { color: #FFFFFF; font-size: 48px; padding-right: 4px; }"
        "QPushButton { border: none; border-radius: 36px; font-size: 24px; font-weight: bold; }"
        "QPushButton[role=\"number\"] { background-color: #333333; color: #FFFFFF; }"
        "QPushButton[role=\"number\"]:pressed { background-color: #737373; }"
        "QPushButton[role=\"operator\"] { background-color: #FF9500; color: #FFFFFF; }"
        "QPushButton[role=\"operator\"]:pressed { background-color: #FBC58C; }"
        "QPushButton[role=\"function\"] { background-color: #A5A5A5; color: #000000; }"
        "QPushButton[role=\"function\"]:pressed { background-color: #D9D9D9; }"
    );
}

QPushButton *Calculator::createButton(const QString &text, const QString &role)
{
    QPushButton *btn = new QPushButton(text, this);
    btn->setProperty("role", role);   // 动态属性，供样式表选择器匹配
    btn->setFocusPolicy(Qt::NoFocus); // 关键：防止按钮抢走焦点，保证键盘事件始终到达窗口
    btn->setCursor(Qt::PointingHandCursor);
    btn->setFixedSize(BUTTON_SIZE, BUTTON_SIZE);        // 默认方形按钮（跨格按钮随后调整）
    return btn;
}

// ---------------------------------------------------------------------------
// 输入处理：按钮与键盘共用的统一逻辑
// ---------------------------------------------------------------------------
void Calculator::inputDigit(int digit)
{
    if (m_hasError)
        clearAll(); // 错误状态下输入数字：先清零，重新开始

    switch (m_state) {
    case State::EnteringFirstNumber:
    case State::EnteringSecondNumber:
        if (m_currentInput.length() >= MAX_INPUT_LENGTH)
            return; // 防止输入串过长撑爆显示
        if (m_currentInput == "0")
            m_currentInput = QString::number(digit); // 前导 0 直接替换
        else
            m_currentInput += QString::number(digit);
        break;

    case State::AfterOperator:
        m_currentInput = QString::number(digit); // 操作符后开始输入第二个操作数
        m_state = State::EnteringSecondNumber;
        break;

    case State::ResultShown:
        clearAll();                             // 结果后输入数字 → 开始全新计算
        m_currentInput = QString::number(digit);
        break;
    }
    updateDisplay();
}

void Calculator::inputDecimalPoint()
{
    if (m_hasError)
        clearAll();

    switch (m_state) {
    case State::EnteringFirstNumber:
    case State::EnteringSecondNumber:
        if (!m_currentInput.contains('.')) // 连续小数点：忽略重复输入
            m_currentInput += '.';
        break;

    case State::AfterOperator:
        m_currentInput = "0.";
        m_state = State::EnteringSecondNumber;
        break;

    case State::ResultShown:
        clearAll();
        m_currentInput = "0.";
        break;
    }
    updateDisplay();
}

void Calculator::inputOperator(Operator op)
{
    if (m_hasError)
        return; // 错误状态下必须先按 AC 清除

    switch (m_state) {
    case State::EnteringFirstNumber:
        m_accumulator = m_currentInput.toDouble();
        m_pendingOperator = op;
        m_state = State::AfterOperator;
        break;

    case State::AfterOperator:
        m_pendingOperator = op; // 连续操作符：后输入的覆盖前一个
        break;

    case State::EnteringSecondNumber: {
        // 连续运算：先结算上一步，再以中间结果作为新的左操作数
        bool ok = false;
        double result = evaluate(m_accumulator, m_pendingOperator,
                                 m_currentInput.toDouble(), ok);
        if (!ok) {
            setError();
            return;
        }
        m_accumulator = result;
        m_currentInput = formatNumber(result);
        m_pendingOperator = op;
        m_state = State::AfterOperator;
        break;
    }

    case State::ResultShown:
        m_pendingOperator = op; // 结果后继续运算：以结果作为左操作数
        m_state = State::AfterOperator;
        break;
    }

    // 刷新表达式提示：左操作数 + 操作符
    m_expressionText = QString("%1 %2")
                           .arg(formatNumber(m_accumulator))
                           .arg(operatorSymbol(op));
    updateDisplay();
}

void Calculator::inputEquals()
{
    if (m_hasError)
        return;

    switch (m_state) {
    case State::EnteringFirstNumber:
        // 只有一个操作数时，等号直接把它作为结果
        m_accumulator = m_currentInput.toDouble();
        m_expressionText = formatNumber(m_accumulator) + " =";
        m_state = State::ResultShown;
        m_pendingOperator = Operator::None;
        break;

    case State::AfterOperator:
        return; // 操作符后直接按等号（缺少第二个操作数）：忽略

    case State::EnteringSecondNumber: {
        bool ok = false;
        double a = m_accumulator;
        double b = m_currentInput.toDouble();
        double result = evaluate(a, m_pendingOperator, b, ok);
        if (!ok) {
            setError(); // 除数为 0 等错误
            return;
        }

        m_expressionText = QString("%1 %2 %3 =")
                               .arg(formatNumber(a))
                               .arg(operatorSymbol(m_pendingOperator))
                               .arg(formatNumber(b));

        m_accumulator = result;
        m_currentInput = formatNumber(result);
        m_state = State::ResultShown;
        m_pendingOperator = Operator::None;
        break;
    }

    case State::ResultShown:
        return; // 重复按等号：忽略
    }
    updateDisplay();
}

void Calculator::backspace()
{
    if (m_hasError)
        return;

    switch (m_state) {
    case State::EnteringFirstNumber:
    case State::EnteringSecondNumber:
        m_currentInput.chop(1);
        if (m_currentInput.isEmpty())
            m_currentInput = "0"; // 退格到空则回到 0
        updateDisplay();
        break;

    default:
        return; // 等待操作符 / 结果状态下不可退格
    }
}

void Calculator::clearAll()
{
    m_accumulator = 0.0;
    m_pendingOperator = Operator::None;
    m_currentInput = "0";
    m_expressionText.clear();
    m_state = State::EnteringFirstNumber;
    m_hasError = false;
    updateDisplay();
}

// ---------------------------------------------------------------------------
// 内部辅助
// ---------------------------------------------------------------------------
double Calculator::evaluate(double a, Operator op, double b, bool &ok) const
{
    ok = true;
    switch (op) {
    case Operator::Add:      return a + b;
    case Operator::Subtract: return a - b;
    case Operator::Multiply: return a * b;
    case Operator::Divide:
        if (b == 0.0) {          // 除数为 0：返回失败
            ok = false;
            return 0.0;
        }
        return a / b;
    default:
        ok = false;
        return 0.0;
    }
}

QString Calculator::formatNumber(double value) const
{
    if (qIsNaN(value))
        return QString::fromUtf8("错误");
    if (qIsInf(value))
        return value > 0 ? QString(QChar(0x221E))          // ∞
                         : QString("-") + QString(QChar(0x221E));

    // 'g' + 15 位有效数字可消除 0.1+0.2 之类的二进制浮点尾差（如 0.30000000000000004 → 0.3）
    return QString::number(value, 'g', 15);
}

QString Calculator::operatorSymbol(Operator op) const
{
    switch (op) {
    case Operator::Add:      return "+";
    case Operator::Subtract: return "-";
    case Operator::Multiply: return QString(QChar(0x00D7)); // ×
    case Operator::Divide:   return QString(QChar(0x00F7)); // ÷
    default:                 return QString();
    }
}

void Calculator::updateDisplay()
{
    m_displayLabel->setText(m_currentInput);
    m_expressionLabel->setText(m_expressionText);
}

void Calculator::setError()
{
    m_hasError = true;
    m_currentInput = QString::fromUtf8("错误");
    m_expressionText.clear();
    m_state = State::ResultShown; // 错误态下其它输入统一由 m_hasError 拦截
    updateDisplay();
}

// ---------------------------------------------------------------------------
// 键盘事件：与按钮复用同一套处理函数
// ---------------------------------------------------------------------------
void Calculator::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_0: inputDigit(0); break;
    case Qt::Key_1: inputDigit(1); break;
    case Qt::Key_2: inputDigit(2); break;
    case Qt::Key_3: inputDigit(3); break;
    case Qt::Key_4: inputDigit(4); break;
    case Qt::Key_5: inputDigit(5); break;
    case Qt::Key_6: inputDigit(6); break;
    case Qt::Key_7: inputDigit(7); break;
    case Qt::Key_8: inputDigit(8); break;
    case Qt::Key_9: inputDigit(9); break;

    case Qt::Key_Plus:     inputOperator(Operator::Add);      break;
    case Qt::Key_Minus:    inputOperator(Operator::Subtract); break;
    case Qt::Key_Asterisk: inputOperator(Operator::Multiply); break; // * 或小键盘 *
    case Qt::Key_Slash:    inputOperator(Operator::Divide);   break; // / 或小键盘 /

    case Qt::Key_Period: // 小数点（主键盘 . 与小键盘 . 均为此键）
    case Qt::Key_Comma:  inputDecimalPoint(); break;

    case Qt::Key_Equal:  // 主键盘 =
    case Qt::Key_Enter:  // 小键盘回车
    case Qt::Key_Return: // 主键盘回车
        inputEquals(); break;

    case Qt::Key_Backspace: backspace(); break;
    case Qt::Key_Delete:    // Delete 键
    case Qt::Key_Escape:    clearAll(); break;

    default:
        QWidget::keyPressEvent(event);
        return;
    }
    event->accept();
}
