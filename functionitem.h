#ifndef FUNCTIONITEM_H
#define FUNCTIONITEM_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QColor>

class FunctionItem : public QWidget
{
    Q_OBJECT

public:
    FunctionItem(const QString &function = QString(), const QColor &color = Qt::blue, QWidget *parent = nullptr);
    QString function() const;
    QColor color() const;
    void setFunction(const QString &function);
    void setColor(const QColor &color);

signals:
    void functionChanged(FunctionItem *item);
    void colorChanged(FunctionItem *item);
    void removeClicked(FunctionItem *item);

private slots:
    void onColorButtonClicked();

private:
    QLineEdit *functionInput;
    QPushButton *colorButton;
    QPushButton *removeButton;
    QColor currentColor;

    void updateColorButtonStyle();
};

#endif // FUNCTIONITEM_H 