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
    if (m_currentInput == "0") {
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
    // 同一个操作数只允许出现一个小数点，重复点击直接忽略
    if (m_currentInput.contains('.')) {
        return;
    }
    m_currentInput += '.';
    refreshDisplay();
}

void Calculator::inputBackspace()
{
    m_currentInput.chop(1);
    if (m_currentInput.isEmpty()) {
        m_currentInput = "0";   // 删空后回到初始的 0
    }
    refreshDisplay();
}

void Calculator::resetAll()
{
    m_currentInput = "0";
    ui->labelHistory->clear();
    refreshDisplay();
}

void Calculator::refreshDisplay()
{
    ui->lineEditDisplay->setText(m_currentInput);
}
