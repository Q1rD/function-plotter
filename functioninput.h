#ifndef FUNCTIONINPUT_H
#define FUNCTIONINPUT_H

#include <QLineEdit>
#include <QWidget>
#include <QPainter>
#include <QFocusEvent>

class FunctionInput : public QLineEdit
{
    Q_OBJECT

public:
    explicit FunctionInput(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;

private:
    bool hasFocus = false;
    void updateStyleSheet();
};

#endif // FUNCTIONINPUT_H 