#include "calculator.h"
#include "ui_calculator.h"

#include <cmath>

#include <QFontMetrics>
#include <QHash>
#include <QKeyEvent>
#include <QPushButton>
#include <QResizeEvent>

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Calculator)
    , m_storedValue(0.0)
    , m_waitingForOperand(false)
    , m_hasError(false)
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
    connect(ui->btnClearEntry, &QPushButton::clicked, this, &Calculator::onClearEntryClicked);
    connect(ui->btnNegate, &QPushButton::clicked, this, &Calculator::onNegateClicked);
    connect(ui->btnPercent, &QPushButton::clicked, this, &Calculator::onPercentClicked);

    // 单目运算三个按钮共用一个槽
    connect(ui->btnReciprocal, &QPushButton::clicked, this, &Calculator::onUnaryClicked);
    connect(ui->btnSquare, &QPushButton::clicked, this, &Calculator::onUnaryClicked);
    connect(ui->btnSquareRoot, &QPushButton::clicked, this, &Calculator::onUnaryClicked);

    // 四则运算符四个按钮共用一个槽
    connect(ui->btnAdd, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->btnSubtract, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->btnMultiply, &QPushButton::clicked, this, &Calculator::onOperatorClicked);
    connect(ui->btnDivide, &QPushButton::clicked, this, &Calculator::onOperatorClicked);

    connect(ui->btnEquals, &QPushButton::clicked, this, &Calculator::onEqualsClicked);

    refreshDisplay();
}

Calculator::~Calculator()
{
    delete ui;
}

void Calculator::keyPressEvent(QKeyEvent *event)
{
    // 键盘只做“按键 -> 统一处理函数”的映射，不写独立计算逻辑，
    // 因此鼠标与键盘天然复用同一套代码，行为保持一致
    switch (event->key()) {
    case Qt::Key_0:
    case Qt::Key_1:
    case Qt::Key_2:
    case Qt::Key_3:
    case Qt::Key_4:
    case Qt::Key_5:
    case Qt::Key_6:
    case Qt::Key_7:
    case Qt::Key_8:
    case Qt::Key_9:
        inputDigit(event->key() - Qt::Key_0);
        break;
    case Qt::Key_Period:
    case Qt::Key_Comma:
        inputPoint();
        break;
    case Qt::Key_Plus:
        inputOperator("+");
        break;
    case Qt::Key_Minus:
        inputOperator("−");
        break;
    case Qt::Key_Asterisk:
        inputOperator("×");
        break;
    case Qt::Key_Slash:
        inputOperator("÷");
        break;
    case Qt::Key_Percent:
        applyPercent();
        break;
    case Qt::Key_Equal:
    case Qt::Key_Return:
    case Qt::Key_Enter:
        inputEquals();
        break;
    case Qt::Key_Backspace:
        inputBackspace();
        break;
    case Qt::Key_Escape:
    case Qt::Key_Delete:
    case Qt::Key_C:
        resetAll();
        break;
    default:
        QWidget::keyPressEvent(event);   // 不处理的按键交给父类
        return;
    }
    event->accept();
}

// ---------- 槽函数：只负责把信号转成统一调用 ----------

void Calculator::onDigitClicked()
{
    // 从按钮名 btnDigitX 中解析出数字 X
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    const int digit = button->objectName().mid(8).toInt();
    inputDigit(digit);
}

void Calculator::onPointClicked()
{
    inputPoint();
}

void Calculator::onOperatorClicked()
{
    static const QHash<QString, QString> symbolMap = {
        {"btnAdd", "+"},
        {"btnSubtract", "−"},
        {"btnMultiply", "×"},
        {"btnDivide", "÷"},
    };
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    inputOperator(symbolMap.value(button->objectName()));
}

void Calculator::onUnaryClicked()
{
    static const QHash<QString, QString> operationMap = {
        {"btnReciprocal", "reciprocal"},
        {"btnSquare", "square"},
        {"btnSquareRoot", "sqrt"},
    };
    const QPushButton *button = qobject_cast<QPushButton *>(sender());
    applyUnary(operationMap.value(button->objectName()));
}

void Calculator::onEqualsClicked()
{
    inputEquals();
}

void Calculator::onBackspaceClicked()
{
    inputBackspace();
}

void Calculator::onClearClicked()
{
    resetAll();
}

void Calculator::onClearEntryClicked()
{
    clearEntry();
}

void Calculator::onNegateClicked()
{
    negateCurrent();
}

void Calculator::onPercentClicked()
{
    applyPercent();
}

// ---------- 核心处理逻辑 ----------

void Calculator::inputDigit(int digit)
{
    if (m_hasError) {
        return;   // 错误状态下锁定输入，只能按 C 清除
    }
    // 单个操作数最多 16 位，防止无限输入把字号压得太小
    if (!m_waitingForOperand && m_currentInput.length() >= 16) {
        return;
    }
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

void Calculator::inputPoint()
{
    if (m_hasError) {
        return;
    }
    if (m_waitingForOperand) {
        m_currentInput = "0.";               // 运算符后直接按点，补出整数部分 0
        m_waitingForOperand = false;
        refreshDisplay();
        return;
    }
    // 同一个操作数只允许一个小数点，重复点击直接忽略
    if (m_currentInput.contains('.')) {
        return;
    }
    m_currentInput += '.';
    refreshDisplay();
}

void Calculator::inputOperator(const QString &op)
{
    if (m_hasError) {
        return;
    }

    const double currentValue = m_currentInput.toDouble();

    if (m_waitingForOperand) {
        // 第二个操作数还没开始输入就换运算符：直接替换，处理连续操作符
        m_pendingOperator = op;
    } else {
        if (!m_pendingOperator.isEmpty()) {
            // 已有挂起运算且第二个操作数已输入：先算中间结果，支持连续运算
            if (!applyBinary(currentValue)) {
                return;   // 触发除零错误，状态已被锁定
            }
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
    if (m_hasError) {
        return;
    }
    if (m_pendingOperator.isEmpty()) {
        return;   // 没有挂起的运算符，忽略
    }

    const double secondOperand = m_currentInput.toDouble();
    ui->labelHistory->setText(formatNumber(m_storedValue) + " " + m_pendingOperator + " "
                              + formatNumber(secondOperand) + " =");
    applyBinary(secondOperand);

    m_pendingOperator.clear();
    m_waitingForOperand = true;   // 计算完成：按数字开始新计算，按运算符则用结果继续
}

void Calculator::inputBackspace()
{
    if (m_hasError) {
        return;
    }
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
    m_hasError = false;
    ui->labelHistory->clear();
    refreshDisplay();
}

void Calculator::clearEntry()
{
    if (m_hasError) {
        resetAll();   // 错误状态下 CE 也可恢复
        return;
    }
    m_currentInput = "0";
    m_waitingForOperand = false;
    refreshDisplay();
}

void Calculator::negateCurrent()
{
    if (m_hasError || m_waitingForOperand) {
        return;
    }
    if (m_currentInput == "0") {
        return;   // 0 取反没有意义
    }
    if (m_currentInput.startsWith('-')) {
        m_currentInput.remove(0, 1);
    } else {
        m_currentInput.prepend('-');
    }
    refreshDisplay();
}

void Calculator::applyPercent()
{
    if (m_hasError || m_waitingForOperand) {
        return;
    }
    const double result = m_currentInput.toDouble() / 100.0;
    m_currentInput = formatNumber(result);
    refreshDisplay();
}

void Calculator::applyUnary(const QString &op)
{
    if (m_hasError || m_waitingForOperand) {
        return;
    }

    const double value = m_currentInput.toDouble();
    double result = 0.0;

    if (op == "reciprocal") {
        if (value == 0.0) {
            enterErrorState("错误：除数不能为 0");
            return;
        }
        result = 1.0 / value;
    } else if (op == "square") {
        result = value * value;
    } else {  // 开平方
        if (value < 0.0) {
            enterErrorState("错误：负数不能开平方");
            return;
        }
        result = std::sqrt(value);
    }

    m_currentInput = formatNumber(result);
    refreshDisplay();
}

bool Calculator::applyBinary(double secondOperand)
{
    double result = 0.0;

    if (m_pendingOperator == "+") {
        result = m_storedValue + secondOperand;
    } else if (m_pendingOperator == "−") {
        result = m_storedValue - secondOperand;
    } else if (m_pendingOperator == "×") {
        result = m_storedValue * secondOperand;
    } else if (m_pendingOperator == "÷") {
        if (secondOperand == 0.0) {
            enterErrorState("错误：除数不能为 0");
            return false;   // 拦截本次计算，等待按 C 清除后重新开始
        }
        result = m_storedValue / secondOperand;
    }

    m_storedValue = result;
    m_currentInput = formatNumber(result);
    refreshDisplay();
    return true;
}

void Calculator::enterErrorState(const QString &message)
{
    m_hasError = true;
    m_pendingOperator.clear();
    m_waitingForOperand = true;
    ui->labelHistory->clear();
    ui->lineEditDisplay->setText(message);
}

QString Calculator::formatNumber(double value)
{
    // 'g' 格式自动去掉多余的尾零，整数不显示小数点，最多保留 15 位有效数字
    return QString::number(value, 'g', 15);
}

void Calculator::refreshDisplay()
{
    QLineEdit *display = ui->lineEditDisplay;
    display->setText(m_currentInput);

    // 根据数字长度自适应字号：文字超宽就逐步缩小，保证完整显示在框内
    int widgetWidth = display->width();
    if (widgetWidth <= 1) {
        widgetWidth = 320;   // 构造期布局尚未确定，用兜底宽度
    }
    const int available = widgetWidth - 32;   // 预留 QSS 左右各 12px 内边距

    QFont font = display->font();
    int pointSize = 36;
    font.setPointSize(pointSize);
    QFontMetrics metrics(font);
    while (metrics.horizontalAdvance(m_currentInput) > available && pointSize > 12) {
        --pointSize;
        font.setPointSize(pointSize);
        metrics = QFontMetrics(font);
    }
    display->setFont(font);
}

void Calculator::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshDisplay();   // 窗口缩放后重新计算字号
}
