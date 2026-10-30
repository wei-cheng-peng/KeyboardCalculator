#include "calculator.h"
#include "ui_calculator.h"

#include <QPushButton>

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculator)
{
    ui->setupUi(this);

    m_currentInput = "0";

    // 10 个数字按钮统一连接到同一个槽，避免重复代码
    for (int i = 0; i <= 9; ++i) {
        QPushButton *button = findChild<QPushButton *>(QString("btnDigit%1").arg(i));
        connect(button, &QPushButton::clicked, this, &Calculator::onDigitClicked);
    }

    connect(ui->btnPoint, &QPushButton::clicked, this, &Calculator::onPointClicked);
    connect(ui->btnBackspace, &QPushButton::clicked, this, &Calculator::onBackspaceClicked);
    connect(ui->btnClear, &QPushButton::clicked, this, &Calculator::onClearClicked);

    // 运算符按钮统一转调 inputOperator，符号与界面显示保持一致
    connect(ui->btnAdd, &QPushButton::clicked, this, [this] { inputOperator("+"); });
    connect(ui->btnSubtract, &QPushButton::clicked, this, [this] { inputOperator("−"); });
    connect(ui->btnMultiply, &QPushButton::clicked, this, [this] { inputOperator("×"); });
    connect(ui->btnDivide, &QPushButton::clicked, this, [this] { inputOperator("÷"); });
    connect(ui->btnEquals, &QPushButton::clicked, this, &Calculator::onEqualsClicked);

    refreshDisplay();
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::onDigitClicked()
{
    // 从发出信号的按钮名 btnDigitX 中解析出数字 X
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    const int digit = button->objectName().mid(8).toInt();
    inputDigit(digit);
}

void Calculator::inputDigit(int digit)
{
    if (m_waitingForOperand) {
        m_currentInput = QString::number(digit);   // 运算符后输入新数字，覆盖旧显示
        m_waitingForOperand = false;
    } else if (m_currentInput == "0") {
        m_currentInput = QString::number(digit);   // 去掉前导 0
    } else {
        m_currentInput += QString::number(digit);  // 正常追加
    }
    refreshDisplay();
}

void Calculator::onPointClicked()
{
    inputPoint();
}

void Calculator::onBackspaceClicked()
{
    inputBackspace();
}

void Calculator::onClearClicked()
{
    resetAll();
}

void Calculator::inputPoint()
{
    if (m_waitingForOperand) {
        m_currentInput = "0.";               // 运算符后直接按点，补出整数部分 0
        m_waitingForOperand = false;
        refreshDisplay();
        return;
    }
    // 同一个操作数只允许出现一个小数点，重复点击直接忽略
    if (m_currentInput.contains('.')) {
        return;
    }
    m_currentInput += '.';
    refreshDisplay();
}

void Calculator::inputBackspace()
{
    if (m_waitingForOperand) {
        return;   // 运算符等待状态下退格没有意义，忽略
    }
    m_currentInput.chop(1);
    if (m_currentInput.isEmpty()) {
        m_currentInput = "0";   // 删空后回到初始的 0
    }
    refreshDisplay();
}

void Calculator::resetAll()
{
    m_currentInput = "0";
    m_storedValue = 0.0;
    m_pendingOperator.clear();
    m_waitingForOperand = false;
    ui->labelHistory->clear();
    refreshDisplay();
}

void Calculator::onEqualsClicked()
{
    inputEquals();
}

void Calculator::inputOperator(const QString &op)
{
    const double currentValue = m_currentInput.toDouble();

    if (m_waitingForOperand) {
        // 第二个操作数还没开始输入就换运算符：直接替换，处理连续操作符
        m_pendingOperator = op;
    } else {
        if (!m_pendingOperator.isEmpty()) {
            // 已有挂起运算且第二个操作数已输入：先算中间结果，支持连续运算
            applyOperator(currentValue);
        } else {
            m_storedValue = currentValue;
        }
        m_pendingOperator = op;
        m_waitingForOperand = true;
    }

    ui->labelHistory->setText(formatNumber(m_storedValue) + " " + op);
}

void Calculator::inputEquals()
{
    if (m_pendingOperator.isEmpty()) {
        return;   // 没有挂起的运算符，忽略
    }

    const double secondOperand = m_currentInput.toDouble();
    ui->labelHistory->setText(formatNumber(m_storedValue) + " " + m_pendingOperator + " "
                              + formatNumber(secondOperand) + " =");
    applyOperator(secondOperand);

    m_pendingOperator.clear();
    m_waitingForOperand = true;   // 计算完成：按数字开始新计算，按运算符则用结果继续
}

bool Calculator::applyOperator(double secondOperand)
{
    double result = 0.0;

    if (m_pendingOperator == "+") {
        result = m_storedValue + secondOperand;
    } else if (m_pendingOperator == "−") {
        result = m_storedValue - secondOperand;
    } else if (m_pendingOperator == "×") {
        result = m_storedValue * secondOperand;
    } else if (m_pendingOperator == "÷") {
        // 注意：除数为 0 的情况暂未处理，后续提交修复
        result = m_storedValue / secondOperand;
    }

    m_storedValue = result;
    m_currentInput = formatNumber(result);
    refreshDisplay();
    return true;
}

QString Calculator::formatNumber(double value)
{
    // 'g' 格式自动去掉多余的尾零，整数不显示小数点，最多保留 15 位有效数字
    return QString::number(value, 'g', 15);
}

void Calculator::refreshDisplay()
{
    ui->lineEditDisplay->setText(m_currentInput);
}
