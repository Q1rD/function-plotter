#include "functioninput.h"
#include <QPainter>
#include <QStyleOption>

FunctionInput::FunctionInput(QWidget *parent)
    : QLineEdit(parent)
{
    setMinimumHeight(40);
    setAttribute(Qt::WA_MacShowFocusRect, false);
    setClearButtonEnabled(true);
    setPlaceholderText("Введите функцию (например: 2*x^2 + sin(x))");
    updateStyleSheet();
}

void FunctionInput::updateStyleSheet()
{
    QString style = QString(
        "FunctionInput {"
        "    padding: 8px 15px;"
        "    border: 2px solid %1;"
        "    border-radius: 10px;"
        "    background-color: white;"
        "    font-size: 14px;"
        "    selection-background-color: #e3f2fd;"
        "}"
        "FunctionInput:hover {"
        "    border-color: #2196F3;"
        "}"
    ).arg(hasFocus ? "#2196F3" : "#B0BEC5");

    setStyleSheet(style);
}

void FunctionInput::paintEvent(QPaintEvent *event)
{
    QLineEdit::paintEvent(event);

    // Добавляем эффект тени
    if (!hasFocus) {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        QColor shadowColor(0, 0, 0, 20);
        for(int i = 0; i < 5; ++i) {
            painter.setPen(QPen(shadowColor, 1));
            painter.drawRoundedRect(rect().adjusted(i, i, -i, -i), 10, 10);
        }
    }
}

void FunctionInput::focusInEvent(QFocusEvent *event)
{
    hasFocus = true;
    updateStyleSheet();
    QLineEdit::focusInEvent(event);
}

void FunctionInput::focusOutEvent(QFocusEvent *event)
{
    hasFocus = false;
    updateStyleSheet();
    QLineEdit::focusOutEvent(event);
} 