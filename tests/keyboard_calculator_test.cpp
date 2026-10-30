#include <QtTest/QtTest>

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include "calculator.h"

// 计算器功能与边界用例测试，使用 offscreen 平台无头运行
class KeyboardCalculatorTest : public QObject
{
    Q_OBJECT

private:
    // 读取主显示框文本
    static QString displayText(Calculator &window)
    {
        return window.findChild<QLineEdit *>("lineEditDisplay")->text();
    }

    // 读取历史表达式文本
    static QString historyText(Calculator &window)
    {
        return window.findChild<QLabel *>("labelHistory")->text();
    }

    // 模拟鼠标点击指定按钮
    static void clickButton(Calculator &window, const QString &objectName)
    {
        QPushButton *button = window.findChild<QPushButton *>(objectName);
        QTest::mouseClick(button, Qt::LeftButton);
    }

private slots:
    // 数字输入与前导零处理
    void testDigitInputAndLeadingZero()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit1");
        clickButton(window, "btnDigit2");
        clickButton(window, "btnDigit3");
        QCOMPARE(displayText(window), QString("123"));

        Calculator another;
        another.show();
        clickButton(another, "btnDigit0");
        clickButton(another, "btnDigit5");
        QCOMPARE(displayText(another), QString("5"));
    }

    // 同一操作数重复小数点只保留一个
    void testDuplicatePointIgnored()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit1");
        clickButton(window, "btnPoint");
        clickButton(window, "btnPoint");
        clickButton(window, "btnDigit2");
        QCOMPARE(displayText(window), QString("1.2"));
    }

    // 运算符后直接按小数点，自动补整数部分 0
    void testPointAfterOperator()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit5");
        clickButton(window, "btnAdd");
        clickButton(window, "btnPoint");
        clickButton(window, "btnDigit5");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("5.5"));
    }

    // 退格逐位删除，删空回到 0
    void testBackspace()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit1");
        clickButton(window, "btnDigit2");
        clickButton(window, "btnDigit3");
        clickButton(window, "btnBackspace");
        QCOMPARE(displayText(window), QString("12"));
        clickButton(window, "btnBackspace");
        clickButton(window, "btnBackspace");
        QCOMPARE(displayText(window), QString("0"));
    }

    // 清除后显示与历史全部复位
    void testClear()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit1");
        clickButton(window, "btnDigit2");
        clickButton(window, "btnAdd");
        clickButton(window, "btnDigit3");
        clickButton(window, "btnClear");
        QCOMPARE(displayText(window), QString("0"));
        QVERIFY(historyText(window).isEmpty());
    }

    // 带小数的加法
    void testAddition()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit1");
        clickButton(window, "btnPoint");
        clickButton(window, "btnDigit5");
        clickButton(window, "btnAdd");
        clickButton(window, "btnDigit2");
        clickButton(window, "btnPoint");
        clickButton(window, "btnDigit2");
        clickButton(window, "btnDigit5");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("3.75"));
    }

    // 减法、乘法、除法
    void testBasicArithmetic()
    {
        Calculator subtraction;
        subtraction.show();
        clickButton(subtraction, "btnDigit5");
        clickButton(subtraction, "btnSubtract");
        clickButton(subtraction, "btnDigit3");
        clickButton(subtraction, "btnEquals");
        QCOMPARE(displayText(subtraction), QString("2"));

        Calculator multiplication;
        multiplication.show();
        clickButton(multiplication, "btnDigit4");
        clickButton(multiplication, "btnMultiply");
        clickButton(multiplication, "btnDigit2");
        clickButton(multiplication, "btnPoint");
        clickButton(multiplication, "btnDigit5");
        clickButton(multiplication, "btnEquals");
        QCOMPARE(displayText(multiplication), QString("10"));

        Calculator division;
        division.show();
        clickButton(division, "btnDigit8");
        clickButton(division, "btnDivide");
        clickButton(division, "btnDigit4");
        clickButton(division, "btnEquals");
        QCOMPARE(displayText(division), QString("2"));
    }

    // 连续操作符：后按的运算符替换前者
    void testContinuousOperatorReplaced()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit5");
        clickButton(window, "btnAdd");
        clickButton(window, "btnSubtract");
        clickButton(window, "btnDigit3");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("2"));
    }

    // 连续运算：中间结果参与下一次运算
    void testChainedCalculation()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit2");
        clickButton(window, "btnAdd");
        clickButton(window, "btnDigit3");
        clickButton(window, "btnMultiply");
        clickButton(window, "btnDigit4");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("20"));
    }

    // 除数为零：进入错误状态，输入被锁定，清除后恢复正常
    void testDivideByZeroAndRecover()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit6");
        clickButton(window, "btnDivide");
        clickButton(window, "btnDigit0");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("错误：除数不能为 0"));

        clickButton(window, "btnDigit5");
        QCOMPARE(displayText(window), QString("错误：除数不能为 0"));

        clickButton(window, "btnClear");
        QCOMPARE(displayText(window), QString("0"));

        clickButton(window, "btnDigit2");
        clickButton(window, "btnAdd");
        clickButton(window, "btnDigit3");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("5"));
    }

    // 计算完成后用结果继续运算
    void testContinueAfterEquals()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit2");
        clickButton(window, "btnAdd");
        clickButton(window, "btnDigit3");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("5"));

        clickButton(window, "btnMultiply");
        clickButton(window, "btnDigit2");
        clickButton(window, "btnEquals");
        QCOMPARE(displayText(window), QString("10"));
    }

    // 纯键盘输入与鼠标结果一致
    void testKeyboardInput()
    {
        Calculator window;
        window.show();

        QTest::keyClick(&window, Qt::Key_7);
        QTest::keyClick(&window, Qt::Key_Plus);
        QTest::keyClick(&window, Qt::Key_8);
        QTest::keyClick(&window, Qt::Key_Return);
        QCOMPARE(displayText(window), QString("15"));

        QTest::keyClick(&window, Qt::Key_C);
        QTest::keyClick(&window, Qt::Key_1);
        QTest::keyClick(&window, Qt::Key_2);
        QTest::keyClick(&window, Qt::Key_3);
        QTest::keyClick(&window, Qt::Key_Backspace);
        QCOMPARE(displayText(window), QString("12"));
    }

    // 鼠标与键盘混合输入
    void testMixedMouseAndKeyboard()
    {
        Calculator window;
        window.show();

        clickButton(window, "btnDigit9");
        QTest::keyClick(&window, Qt::Key_Plus);
        clickButton(window, "btnDigit1");
        QTest::keyClick(&window, Qt::Key_Enter);
        QCOMPARE(displayText(window), QString("10"));
    }
};

QTEST_MAIN(KeyboardCalculatorTest)
#include "keyboard_calculator_test.moc"
