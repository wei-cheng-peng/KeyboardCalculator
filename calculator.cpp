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

void Calculator::refreshDisplay()
{
    ui->lineEditDisplay->setText(m_currentInput);
}
