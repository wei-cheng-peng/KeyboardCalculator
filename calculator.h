#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QList>
#include <QResizeEvent>
#include <QString>
#include <QStringList>
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
    // 窗口缩放时重新适配显示字号
    void resizeEvent(QResizeEvent *event) override;

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

    bool evaluateExpression(double &result);              // 对完整表达式求值，先乘除后加减
    void enterErrorState(const QString &message);         // 进入错误状态并锁定输入
    void startNewExpression();                            // 清空表达式，准备重新开始
    void refreshDisplay();
    static QString formatNumber(double value);

    Ui::Calculator *ui;

    // —— 表达式状态 ——
    QString m_currentInput;         // 当前正在输入的数字字符串，显示框数据源
    QString m_expression;           // 完整表达式字符串，历史区显示，如 5+5+9
    QList<double> m_operands;       // 已确认的操作数
    QStringList m_operators;        // 已确认的运算符，数量比操作数少 1
    bool m_waitingForOperand;       // 按过运算符后为 true，下一个数字开启新输入
    bool m_resultShown;             // 当前显示的是等号结果
    bool m_hasError;                // 错误标志，为 true 时仅响应清除
};

#endif // CALCULATOR_H
