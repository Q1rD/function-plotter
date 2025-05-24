#include "functionitem.h"
#include <QColorDialog>
#include <QStyle>

FunctionItem::FunctionItem(const QString &function, const QColor &color, QWidget *parent)
    : QWidget(parent)
    , currentColor(color)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(5);

    // Создаем поле ввода функции
    functionInput = new QLineEdit(this);
    functionInput->setPlaceholderText("Введите функцию");
    functionInput->setText(function);
    functionInput->setStyleSheet(
        "QLineEdit {"
        "    padding: 5px 10px;"
        "    border: 2px solid #B0BEC5;"
        "    border-radius: 5px;"
        "    background-color: white;"
        "    font-size: 13px;"
        "    color: black;"
        "}"
        "QLineEdit:hover {"
        "    border-color: #2196F3;"
        "}"
        "QLineEdit:focus {"
        "    border-color: #2196F3;"
        "}"
    );

    // Создаем кнопку выбора цвета
    colorButton = new QPushButton(this);
    colorButton->setFixedSize(30, 30);
    updateColorButtonStyle();

    // Создаем кнопку удаления
    removeButton = new QPushButton(this);
    removeButton->setFixedSize(30, 30);
    removeButton->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
    removeButton->setStyleSheet(
        "QPushButton {"
        "    border: none;"
        "    border-radius: 15px;"
        "    background-color: transparent;"
        "}"
        "QPushButton:hover {"
        "    background-color: #ffebee;"
        "}"
    );

    // Добавляем виджеты в layout
    layout->addWidget(functionInput, 1);
    layout->addWidget(colorButton);
    layout->addWidget(removeButton);

    // Подключаем сигналы
    connect(functionInput, &QLineEdit::textChanged, [this]() {
        emit functionChanged(this);
    });
    connect(colorButton, &QPushButton::clicked, this, &FunctionItem::onColorButtonClicked);
    connect(removeButton, &QPushButton::clicked, [this]() {
        emit removeClicked(this);
    });
}

QString FunctionItem::function() const
{
    return functionInput->text();
}

QColor FunctionItem::color() const
{
    return currentColor;
}

void FunctionItem::setFunction(const QString &function)
{
    functionInput->setText(function);
}

void FunctionItem::setColor(const QColor &color)
{
    currentColor = color;
    updateColorButtonStyle();
    emit colorChanged(this);
}

void FunctionItem::onColorButtonClicked()
{
    QColor newColor = QColorDialog::getColor(currentColor, this, "Выберите цвет функции");
    if (newColor.isValid()) {
        setColor(newColor);
    }
}

void FunctionItem::updateColorButtonStyle()
{
    QString style = QString(
        "QPushButton {"
        "    background-color: %1;"
        "    border: 2px solid #B0BEC5;"
        "    border-radius: 15px;"
        "}"
        "QPushButton:hover {"
        "    border-color: #2196F3;"
        "}"
    ).arg(currentColor.name());
    colorButton->setStyleSheet(style);
} 