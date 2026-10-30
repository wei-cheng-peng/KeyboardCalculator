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
    void onDigitClicked();           // 数字按钮 0-9
    void onPointClicked();           // 小数点
    void onOperatorClicked();        // 四则运算符
    void onEqualsClicked();          // 等号
    void onBackspaceClicked();       // 退格
    void onClearClicked();           // 全部清除 C
    void onClearEntryClicked();      // 清除当前输入 CE
    void onNegateClicked();          // 正负号 ±
    void onPercentClicked();         // 百分比 %
    void onUnaryClicked();           // 单目运算：1/x、x²、√

private:
    // —— 核心处理逻辑：鼠标和键盘最终都调用这一套函数 ——
    void inputDigit(int digit);
    void inputPoint();
    void inputOperator(const QString &op);
    void inputEquals();
    void inputBackspace();
    void resetAll();
    void clearEntry();
    void negateCurrent();
    void applyPercent();
    void applyUnary(const QString &op);

    bool applyBinary(double secondOperand);                 // 四则运算，结果写回 m_storedValue
    void enterErrorState(const QString &message);           // 进入错误状态并锁定输入
    void refreshDisplay();
    static QString formatNumber(double value);

    Ui::Calculator *ui;

    // —— 计算状态机 ——
    QString m_currentInput;         // 当前操作数字符串，显示与输入的数据源
    double m_storedValue;           // 第一个操作数 / 连续运算的中间结果
    QString m_pendingOperator;      // 待执行的运算符，为空表示没有挂起运算
    bool m_waitingForOperand;       // 按过运算符后为 true，下一个数字开启新输入
    bool m_hasError;                // 错误标志，为 true 时仅响应清除
};

#endif // CALCULATOR_H
