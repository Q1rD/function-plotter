#include "mainwindow.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Инициализируем набор цветов по умолчанию
    defaultColors = {
        QColor("#2196F3"), // Blue
        QColor("#F44336"), // Red
        QColor("#4CAF50"), // Green
        QColor("#FF9800"), // Orange
        QColor("#9C27B0"), // Purple
        QColor("#795548"), // Brown
        QColor("#009688"), // Teal
        QColor("#673AB7"), // Deep Purple
        QColor("#FF5722"), // Deep Orange
        QColor("#607D8B")  // Blue Grey
    };

    setupMainLayout();
    setupSidePanel();
    
    resize(1200, 800);
    setWindowTitle("Построение графиков функций");
}

void MainWindow::setupMainLayout()
{
    centralWidget = new QWidget(this);
    mainLayout = new QHBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Создаем виджет для отображения графика
    plotWidget = new PlotWidget(this);
    mainLayout->addWidget(plotWidget, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::setupSidePanel()
{
    // Создаем боковую панель
    sidePanel = new QWidget(this);
    sidePanel->setFixedWidth(300);
    sidePanel->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border-left: 1px solid #E0E0E0;"
        "}"
    );
    
    QVBoxLayout *sidePanelLayout = new QVBoxLayout(sidePanel);
    sidePanelLayout->setSpacing(10);
    sidePanelLayout->setContentsMargins(15, 15, 15, 15);

    // Добавляем заголовок
    QLabel *titleLabel = new QLabel("Список функций", sidePanel);
    titleLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 18px;"
        "    color: #1976D2;"
        "    font-weight: bold;"
        "    padding-bottom: 10px;"
        "}"
    );
    sidePanelLayout->addWidget(titleLabel);

    // Создаем область прокрутки для списка функций
    scrollArea = new QScrollArea(sidePanel);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    
    QWidget *scrollContent = new QWidget(scrollArea);
    functionsList = new QVBoxLayout(scrollContent);
    functionsList->setSpacing(10);
    functionsList->setContentsMargins(0, 0, 0, 0);
    functionsList->addStretch();
    
    scrollArea->setWidget(scrollContent);
    sidePanelLayout->addWidget(scrollArea, 1);

    // Добавляем кнопку добавления функции
    addFunctionButton = new QPushButton("Добавить функцию", sidePanel);
    addFunctionButton->setStyleSheet(
        "QPushButton {"
        "    padding: 8px;"
        "    background-color: #2196F3;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 5px;"
        "    font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #1976D2;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #0D47A1;"
        "}"
    );
    sidePanelLayout->addWidget(addFunctionButton);

    // Добавляем панель в главный layout
    mainLayout->addWidget(sidePanel);

    // Подключаем сигнал кнопки
    connect(addFunctionButton, &QPushButton::clicked, this, &MainWindow::onAddFunctionClicked);
}

QColor MainWindow::getNextColor()
{
    if (defaultColors.isEmpty()) {
        return QColor(rand() % 256, rand() % 256, rand() % 256);
    }
    QColor color = defaultColors[nextColorIndex];
    nextColorIndex = (nextColorIndex + 1) % defaultColors.size();
    return color;
}

void MainWindow::onAddFunctionClicked()
{
    QColor color = getNextColor();
    FunctionItem *item = new FunctionItem(QString(), color);
    
    // Вставляем новый элемент перед растягивающим спейсером
    functionsList->insertWidget(functionsList->count() - 1, item);

    // Подключаем сигналы
    connect(item, &FunctionItem::functionChanged, this, &MainWindow::onFunctionChanged);
    connect(item, &FunctionItem::colorChanged, this, &MainWindow::onColorChanged);
    connect(item, &FunctionItem::removeClicked, this, &MainWindow::onRemoveFunction);
}

void MainWindow::onFunctionChanged(FunctionItem *item)
{
    QString oldFunction = item->property("lastFunction").toString();
    QString newFunction = item->function();
    QColor color = item->color();
    
    if (!oldFunction.isEmpty()) {
        plotWidget->updateFunction(oldFunction, newFunction, color);
    } else {
        plotWidget->addFunction(newFunction, color);
    }
    
    item->setProperty("lastFunction", newFunction);
}

void MainWindow::onColorChanged(FunctionItem *item)
{
    QString function = item->function();
    QColor color = item->color();
    if (!function.isEmpty()) {
        plotWidget->updateFunction(function, function, color);
    }
}

void MainWindow::onRemoveFunction(FunctionItem *item)
{
    QString function = item->function();
    if (!function.isEmpty()) {
        plotWidget->removeFunction(function);
    }
    functionsList->removeWidget(item);
    item->deleteLater();
    }

MainWindow::~MainWindow()
{
} 