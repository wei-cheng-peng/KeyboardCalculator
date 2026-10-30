#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
class Calculator;
}
QT_END_NAMESPACE

// 带键盘事件的计算器主窗口
class Calculator : public QWidget
{
    Q_OBJECT

public:
    explicit Calculator(QWidget *parent = nullptr);
    ~Calculator() override;

protected:
    // 重写键盘事件：把物理按键映射到与鼠标按钮相同的处理函数
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onDigitClicked();          // 数字按钮 0-9 的统一槽函数
    void onPointClicked();          // 小数点按钮
    void onBackspaceClicked();      // 退格按钮
    void onClearClicked();          // 清除按钮
    void onEqualsClicked();         // 等号按钮

private:
    void inputDigit(int digit);     // 数字输入的核心处理
    void inputPoint();              // 小数点处理
    void inputBackspace();          // 退格处理
    void inputOperator(const QString &op);  // 四则运算符处理
    void inputEquals();             // 执行计算
    void resetAll();                // 全部清除
    bool applyOperator(double secondOperand); // 按挂起的运算符计算，结果写回 m_storedValue
    void enterErrorState();         // 进入除零错误状态并锁定输入
    void refreshDisplay();          // 用 m_currentInput 刷新显示框
    static QString formatNumber(double value); // 统一数字格式，去掉浮点尾零

    Ui::Calculator *ui;

    // —— 计算状态机 ——
    QString m_currentInput;         // 当前操作数字符串，显示与输入的数据源
    double m_storedValue = 0.0;     // 第一个操作数 / 连续运算的中间结果
    QString m_pendingOperator;      // 待执行的运算符，为空表示没有挂起运算
    bool m_waitingForOperand = false; // 按过运算符后为 true，下一个数字开启新输入
    bool m_hasError = false;        // 除零错误标志，为 true 时仅响应清除
};

#endif // CALCULATOR_H
