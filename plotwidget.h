#ifndef PLOTWIDGET_H
#define PLOTWIDGET_H

#include <QWidget>
#include <QString>
#include <QVector>
#include <QPair>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <muParser.h>
#include <QMap>
#include <cmath>
#include <memory>
#include <QDebug>
#include <QRegularExpression>

struct Function {
    QString expression;
    QColor color;
    std::shared_ptr<mu::Parser> parser;
    mutable double xValue;

    static double power_wrapper(double v1, double v2) {
        if (v1 < 0 && std::floor(v2) != v2) {
            return std::numeric_limits<double>::quiet_NaN();
        }
        return std::pow(v1, v2);
    }

    // Метод для предварительной обработки выражения
    QString preprocessExpression(const QString &expr) const {
        QString result = expr;
        
        // Проверяем, является ли ввод просто числом
        bool isNumber;
        result.toDouble(&isNumber);
        if (!isNumber) {
            // Заменяем все вхождения числа перед x на число*x
            QRegularExpression numberBeforeX("(\\d+)([xX])");
            result.replace(numberBeforeX, "\\1*\\2");
            
            // Заменяем все вхождения числа с точкой перед x на число*x
            QRegularExpression floatBeforeX("(\\d*\\.\\d+)([xX])");
            result.replace(floatBeforeX, "\\1*\\2");
            
            // Заменяем все вхождения x перед числом на x*число
            QRegularExpression numberAfterX("([xX])(\\d+)");
            result.replace(numberAfterX, "\\1*\\2");
            
            // Заменяем все вхождения x перед числом с точкой на x*число
            QRegularExpression floatAfterX("([xX])(\\d*\\.\\d+)");
            result.replace(floatAfterX, "\\1*\\2");
            
            // Заменяем )x на )*x и x( на x*(
            result.replace(")(", ")*(");
            result.replace(")x", ")*x");
            result.replace(")X", ")*X");
            result.replace("x(", "x*(");
            result.replace("X(", "X*(");
            
            // Приводим x к нижнему регистру для единообразия
            result.replace("X", "x");
            
            // Заменяем "^" на "pow" для корректной работы со степенями
            QRegularExpression powerRegex("([\\d.]+|x|\\([^)]+\\))\\s*\\^\\s*([\\d.]+|x|\\([^)]+\\))");
            int pos = 0;
            QRegularExpressionMatch match;
            while ((match = powerRegex.match(result, pos)).hasMatch()) {
                QString matchStr = match.captured(0);
                QString base = match.captured(1);
                QString exponent = match.captured(2);
                QString replacement = QString("pow(%1,%2)").arg(base, exponent);
                result.replace(match.capturedStart(), match.capturedLength(), replacement);
                pos = match.capturedStart() + replacement.length();
            }
        }
        
        // Добавляем нейтральный член с x в конец любого выражения
        result = QString("(%1)+0*x").arg(result);
        
        qDebug() << "Исходное выражение:" << expr;
        qDebug() << "Обработанное выражение:" << result;
        return result;
    }

    Function(const QString &expr = QString(), const QColor &col = Qt::blue)
        : expression(expr), color(col), xValue(0.0), parser(std::make_shared<mu::Parser>())
    {
        try {
            parser->SetDecSep('.');
            parser->SetThousandsSep(' ');
            
            // Определяем стандартные математические функции
            parser->DefineFun("pow", power_wrapper);
            parser->DefineFun("sin", static_cast<double (*)(double)>(std::sin));
            parser->DefineFun("cos", static_cast<double (*)(double)>(std::cos));
            parser->DefineFun("tan", static_cast<double (*)(double)>(std::tan));
            parser->DefineFun("sqrt", static_cast<double (*)(double)>(std::sqrt));
            parser->DefineFun("abs", static_cast<double (*)(double)>(std::abs));
            parser->DefineFun("exp", static_cast<double (*)(double)>(std::exp));
            parser->DefineFun("log", static_cast<double (*)(double)>(std::log));
            parser->DefineFun("log10", static_cast<double (*)(double)>(std::log10));

            // Определяем переменную до установки выражения
            parser->DefineVar("x", &xValue);

            // Устанавливаем выражение для парсера с предварительной обработкой
            if (!expression.isEmpty()) {
                QString processedExpr = preprocessExpression(expression);
                parser->SetExpr(processedExpr.toStdString());
            }
        }
        catch (const mu::Parser::exception_type &e) {
            qDebug() << "Ошибка при инициализации парсера:" << QString::fromStdString(e.GetMsg());
        }
    }

    Function(const Function &other)
        : expression(other.expression), color(other.color), xValue(other.xValue), 
          parser(std::make_shared<mu::Parser>())
    {
        try {
            parser->SetDecSep('.');
            parser->SetThousandsSep(' ');
            
            // Определяем стандартные математические функции
            parser->DefineFun("pow", power_wrapper);
            parser->DefineFun("sin", static_cast<double (*)(double)>(std::sin));
            parser->DefineFun("cos", static_cast<double (*)(double)>(std::cos));
            parser->DefineFun("tan", static_cast<double (*)(double)>(std::tan));
            parser->DefineFun("sqrt", static_cast<double (*)(double)>(std::sqrt));
            parser->DefineFun("abs", static_cast<double (*)(double)>(std::abs));
            parser->DefineFun("exp", static_cast<double (*)(double)>(std::exp));
            parser->DefineFun("log", static_cast<double (*)(double)>(std::log));
            parser->DefineFun("log10", static_cast<double (*)(double)>(std::log10));

            // Определяем переменную до установки выражения
            parser->DefineVar("x", &xValue);

            // Копируем выражение из другого парсера с предварительной обработкой
            if (!expression.isEmpty()) {
                QString processedExpr = preprocessExpression(expression);
                parser->SetExpr(processedExpr.toStdString());
            }
        }
        catch (const mu::Parser::exception_type &e) {
            qDebug() << "Ошибка при копировании функции:" << QString::fromStdString(e.GetMsg());
        }
    }

    Function& operator=(const Function &other)
    {
        if (this != &other) {
            expression = other.expression;
            color = other.color;
            xValue = other.xValue;
            parser = std::make_shared<mu::Parser>();
            
            try {
                parser->SetDecSep('.');
                parser->SetThousandsSep(' ');
                
                // Определяем стандартные математические функции
                parser->DefineFun("pow", power_wrapper);
                parser->DefineFun("sin", static_cast<double (*)(double)>(std::sin));
                parser->DefineFun("cos", static_cast<double (*)(double)>(std::cos));
                parser->DefineFun("tan", static_cast<double (*)(double)>(std::tan));
                parser->DefineFun("sqrt", static_cast<double (*)(double)>(std::sqrt));
                parser->DefineFun("abs", static_cast<double (*)(double)>(std::abs));
                parser->DefineFun("exp", static_cast<double (*)(double)>(std::exp));
                parser->DefineFun("log", static_cast<double (*)(double)>(std::log));
                parser->DefineFun("log10", static_cast<double (*)(double)>(std::log10));

                // Определяем переменную до установки выражения
                parser->DefineVar("x", &xValue);

                // Копируем выражение с предварительной обработкой
                if (!expression.isEmpty()) {
                    QString processedExpr = preprocessExpression(expression);
                    parser->SetExpr(processedExpr.toStdString());
                }
            }
            catch (const mu::Parser::exception_type &e) {
                qDebug() << "Ошибка присваивания парсера:" << QString::fromStdString(e.GetMsg());
            }
        }
        return *this;
    }
};

class PlotWidget : public QWidget {
    Q_OBJECT

public:
    explicit PlotWidget(QWidget *parent = nullptr);
    ~PlotWidget();
    void addFunction(const QString &func, const QColor &color);
    void updateFunction(const QString &oldFunc, const QString &newFunc, const QColor &color);
    void removeFunction(const QString &func);

protected:
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    QMap<QString, Function> functions;
    double xMin = -10.0;
    double xMax = 10.0;
    double yMin = -10.0;
    double yMax = 10.0;
    double zoomFactor = 1.0;
    
    bool isPanning = false;
    bool isAltPressed = false;
    QPoint lastMousePos;
    QPoint currentMousePos;
    bool isMouseInWidget = false;
    
    QPair<double, double> nearestPoint;
    bool hasNearestPoint = false;
    
    QVector<QPair<double, double>> calculatePoints(const Function &func);
    void drawAxes(QPainter &painter);
    void drawGrid(QPainter &painter);
    void drawFunction(QPainter &painter, const QString &expr, Function &func);
    void drawAxisLabels(QPainter &painter);
    void drawCoordinates(QPainter &painter);
    void drawGraphPoint(QPainter &painter);
    QPoint transformToScreen(double x, double y);
    QPair<double, double> transformToGraph(int screenX, int screenY);
    void zoom(double factor, QPoint center);
    void pan(const QPoint &delta);
    QPair<double, double> findNearestPoint(const QPoint &mousePos);
    double evaluateFunction(double x, const Function &func) const;
};

#endif // PLOTWIDGET_H 