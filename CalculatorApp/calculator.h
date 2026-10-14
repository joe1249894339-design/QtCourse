#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QWidget>
#include <QString>

class QLabel;
class QPushButton;
class QKeyEvent;

/*
 * 计算器主窗口。
 *
 * 设计要点：
 * 1. 用一个显式状态机描述输入流程，把“输入数字 / 小数点 / 操作符 / 等号 / 退格 / 清除”
 *    全部集中到几个处理函数中，边界情况（连续小数点、连续操作符、除零、连续运算等）
 *    都在状态机里统一处理，避免散落的 if-else。
 * 2. 按钮点击与键盘事件调用的是同一套处理函数，因此鼠标与键盘行为完全一致，
 *    也便于混合输入。
 */
class Calculator : public QWidget
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);

protected:
    // 键盘输入入口：把按键映射到与按钮相同的处理函数
    void keyPressEvent(QKeyEvent *event) override;

private:
    // ---- 状态定义（须先于下面的函数声明，因为函数形参要用到它们）----
    enum class Operator { None, Add, Subtract, Multiply, Divide };

    enum class State {
        EnteringFirstNumber,   // 正在输入第一个操作数
        AfterOperator,         // 已按下操作符，等待第二个操作数
        EnteringSecondNumber,  // 正在输入第二个操作数
        ResultShown            // 已显示结果
    };

    // ---- 输入处理（按钮点击与键盘事件共用的唯一逻辑）----
    void inputDigit(int digit);          // 输入数字 0~9
    void inputDecimalPoint();            // 输入小数点
    void inputOperator(Operator op);     // 输入操作符
    void inputEquals();                  // 输入等号
    void backspace();                    // 退格
    void clearAll();                     // 全部清除

    // ---- 内部辅助 ----
    double evaluate(double a, Operator op, double b, bool &ok) const; // 计算 a op b
    QString formatNumber(double value) const;   // 数字格式化，消除浮点尾差
    QString operatorSymbol(Operator op) const;  // 操作符转显示符号
    void updateDisplay();                       // 刷新显示
    void setError();                            // 进入错误状态（除数为 0 等）

    // ---- 界面构建 ----
    void buildUi();
    QPushButton *createButton(const QString &text, const QString &role);

    // ---- 状态变量 ----
    State m_state = State::EnteringFirstNumber;
    bool m_hasError = false;          // 是否处于错误状态（除数为 0）
    QString m_currentInput = "0";     // 当前正在输入的数字串（保留小数点）
    double m_accumulator = 0.0;       // 累积的操作数 / 结果
    Operator m_pendingOperator = Operator::None; // 待执行的操作符
    QString m_expressionText;         // 顶部表达式提示，如 "12 +" / "12 + 5 ="

    // ---- 控件 ----
    QLabel *m_displayLabel = nullptr;     // 主显示：当前数字 / 结果
    QLabel *m_expressionLabel = nullptr;  // 副显示：表达式 / 操作符

    static const int MAX_INPUT_LENGTH = 15; // 输入串长度上限，防止溢出

    // 界面尺寸常量（集中管理，便于统一调整）
    static const int WINDOW_WIDTH  = 360;  // 窗口宽
    static const int WINDOW_HEIGHT = 560;  // 窗口高
    static const int BUTTON_SIZE   = 72;   // 普通按钮边长
    static const int SPAN_SIZE     = 154;  // 跨两格按钮边长（72×2 + 间距 10）
};

#endif // CALCULATOR_H
