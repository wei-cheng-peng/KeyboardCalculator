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
    , m_waitingForOperand(false)
    , m_resultShown(false)
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
    if (m_resultShown) {
        // 结果态按数字：开始全新计算
        startNewExpression();
        m_currentInput = QString::number(digit);
        refreshDisplay();
        return;
    }
    if (m_waitingForOperand) {
        m_currentInput = QString::number(digit);   // 运算符后输入新数字
        m_waitingForOperand = false;
    } else if (m_currentInput == "0") {
        m_currentInput = QString::number(digit);   // 去掉前导 0
    } else {
        if (m_currentInput.length() >= 16) {
            return;   // 单个操作数最多 16 位
        }
        m_currentInput += QString::number(digit);  // 正常追加
    }
    refreshDisplay();
}

void Calculator::inputPoint()
{
    if (m_hasError) {
        return;
    }
    if (m_resultShown) {
        startNewExpression();
        m_currentInput = "0.";
        refreshDisplay();
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

    if (m_resultShown) {
        // 用当前结果作为新表达式的第一个操作数，再挂起新运算符
        m_operands.clear();
        m_operators.clear();
        m_operands.append(m_currentInput.toDouble());
        m_operators.append(op);
        m_expression = formatNumber(m_currentInput.toDouble()) + op;
        m_resultShown = false;
        m_waitingForOperand = true;
        ui->labelHistory->setText(m_expression);
        return;
    }

    if (m_waitingForOperand) {
        // 第二个操作数还没输入就换运算符：替换最后一个运算符
        m_operators.last() = op;
        m_expression.chop(1);
        m_expression += op;
    } else {
        // 确认当前数字进入表达式，只累积不计算
        const double value = m_currentInput.toDouble();
        m_operands.append(value);
        m_expression += formatNumber(value);
        m_operators.append(op);
        m_expression += op;
        m_waitingForOperand = true;
    }

    ui->labelHistory->setText(m_expression);
}

void Calculator::inputEquals()
{
    if (m_hasError) {
        return;
    }
    if (m_operators.isEmpty()) {
        return;   // 没有运算符，忽略
    }

    // 补上最后一个操作数
    if (!m_waitingForOperand) {
        m_operands.append(m_currentInput.toDouble());
        m_expression += formatNumber(m_currentInput.toDouble());
    }

    double result = 0.0;
    if (!evaluateExpression(result)) {
        return;   // 触发除零等错误，状态已锁定
    }

    m_currentInput = formatNumber(result);
    m_resultShown = true;
    m_waitingForOperand = false;
    ui->labelHistory->setText(m_expression + "=");
    refreshDisplay();
}

void Calculator::inputBackspace()
{
    if (m_hasError) {
        return;
    }

    if (m_resultShown) {
        // 结果态：把结果当成普通数字逐位删除
        m_resultShown = false;
        m_expression.clear();
        m_operands.clear();
        m_operators.clear();
        ui->labelHistory->clear();
        m_currentInput.chop(1);
        if (m_currentInput.isEmpty()) {
            m_currentInput = "0";
        }
        refreshDisplay();
        return;
    }

    if (m_waitingForOperand) {
        // 刚按完运算符，退格删掉最后一个运算符，数字退回为当前输入
        if (!m_operators.isEmpty()) {
            m_operators.removeLast();
            if (!m_operands.isEmpty()) {
                const double confirmed = m_operands.last();
                m_operands.removeLast();
                // 表达式先移除数字串，再删末尾运算符
                m_expression.chop(formatNumber(confirmed).length());
            }
            m_expression.chop(1);
            m_waitingForOperand = false;
        }
        refreshDisplay();
        return;
    }

    m_currentInput.chop(1);
    if (m_currentInput.isEmpty()) {
        m_currentInput = "0";   // 删空后回到初始的 0
    }
    refreshDisplay();
}

void Calculator::resetAll()
{
    m_hasError = false;
    startNewExpression();
    refreshDisplay();
}

void Calculator::startNewExpression()
{
    m_currentInput = "0";
    m_expression.clear();
    m_operands.clear();
    m_operators.clear();
    m_waitingForOperand = false;
    m_resultShown = false;
    ui->labelHistory->clear();
}

void Calculator::clearEntry()
{
    if (m_hasError || m_resultShown) {
        resetAll();
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
    if (m_resultShown) {
        // 在结果上取反，转为普通编辑并清掉旧表达式
        m_resultShown = false;
        m_expression.clear();
        m_operands.clear();
        m_operators.clear();
        ui->labelHistory->clear();
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
    if (m_resultShown) {
        m_resultShown = false;
        m_expression.clear();
        m_operands.clear();
        m_operators.clear();
        ui->labelHistory->clear();
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
    if (m_resultShown) {
        m_resultShown = false;
        m_expression.clear();
        m_operands.clear();
        m_operators.clear();
        ui->labelHistory->clear();
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

bool Calculator::evaluateExpression(double &result)
{
    QList<double> nums = m_operands;
    QStringList ops = m_operators;

    // 第一遍：从左到右处理所有乘除，保证先乘除后加减
    int i = 0;
    while (i < ops.size()) {
        if (ops[i] == "×" || ops[i] == "÷") {
            const double left = nums[i];
            const double right = nums[i + 1];
            double value = 0.0;
            if (ops[i] == "×") {
                value = left * right;
            } else {
                if (right == 0.0) {
                    enterErrorState("错误：除数不能为 0");
                    return false;
                }
                value = left / right;
            }
            nums[i] = value;
            nums.removeAt(i + 1);
            ops.removeAt(i);   // 不递增 i，继续检查同一位置
        } else {
            ++i;
        }
    }

    // 第二遍：从左到右处理加减
    double value = nums[0];
    for (int j = 0; j < ops.size(); ++j) {
        if (ops[j] == "+") {
            value += nums[j + 1];
        } else {
            value -= nums[j + 1];
        }
    }

    result = value;
    return true;
}

void Calculator::enterErrorState(const QString &message)
{
    m_hasError = true;
    m_operators.clear();
    m_waitingForOperand = true;
    m_resultShown = false;
    m_expression.clear();
    m_currentInput = message;
    refreshDisplay();
}

QString Calculator::formatNumber(double value)
{
    // 'g' 格式自动去掉多余的尾零，整数不显示小数点，最多保留 15 位有效数字
    return QString::number(value, 'g', 15);
}

void Calculator::refreshDisplay()
{
    QLineEdit *display = ui->lineEditDisplay;

    // 大结果框只在出错或按等号出结果时显示，输入过程中留空
    const QString showText = (m_hasError || m_resultShown) ? m_currentInput : QString();
    display->setText(showText);

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
    while (metrics.horizontalAdvance(showText) > available && pointSize > 12) {
        --pointSize;
        font.setPointSize(pointSize);
        metrics = QFontMetrics(font);
    }
    display->setFont(font);

    // 同步历史表达式：当前数字实时追加，等号后补“=”
    if (m_hasError) {
        ui->labelHistory->clear();
    } else if (m_resultShown) {
        ui->labelHistory->setText(m_expression + "=");
    } else if (m_waitingForOperand) {
        ui->labelHistory->setText(m_expression);
    } else {
        // 输入过程中完整式子（含当前数字）在历史区回显，初始的 0 不显示
        const QString editingText = m_expression + m_currentInput;
        if (editingText == "0") {
            ui->labelHistory->clear();
        } else {
            ui->labelHistory->setText(editingText);
        }
    }
}

void Calculator::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshDisplay();   // 窗口缩放后重新计算字号
}
