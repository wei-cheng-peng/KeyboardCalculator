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

private slots:
    void onDigitClicked();          // 数字按钮 0-9 的统一槽函数
    void onPointClicked();          // 小数点按钮
    void onBackspaceClicked();      // 退格按钮
    void onClearClicked();          // 清除按钮

private:
    void inputDigit(int digit);     // 数字输入的核心处理
    void inputPoint();              // 小数点处理
    void inputBackspace();          // 退格处理
    void resetAll();                // 全部清除
    void refreshDisplay();          // 用 m_currentInput 刷新显示框

    Ui::Calculator *ui;
    QString m_currentInput;         // 当前输入的数字字符串，也是显示的数据源
};

#endif // CALCULATOR_H
