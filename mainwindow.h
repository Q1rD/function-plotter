#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QScrollArea>
#include "plotwidget.h"
#include "functionitem.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddFunctionClicked();
    void onFunctionChanged(FunctionItem *item);
    void onColorChanged(FunctionItem *item);
    void onRemoveFunction(FunctionItem *item);

private:
    PlotWidget *plotWidget;
    QWidget *sidePanel;
    QVBoxLayout *functionsList;
    QPushButton *addFunctionButton;
    QWidget *centralWidget;
    QHBoxLayout *mainLayout;
    QScrollArea *scrollArea;
    
    QVector<QColor> defaultColors;
    int nextColorIndex = 0;
    
    void setupSidePanel();
    void setupMainLayout();
    QColor getNextColor();
};

#endif // MAINWINDOW_H 