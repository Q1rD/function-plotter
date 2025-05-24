#include "plotwidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <cmath>
#include <QRegularExpression>
#include <QToolTip>

// Функция для вычисления котангенса
static double cot(double x) {
    double tanVal = tan(x);
    if (tanVal == 0) {
        return std::numeric_limits<double>::infinity();
    }
    return 1.0 / tanVal;
}

// Функция для вычисления степени с поддержкой отрицательных чисел
static double power(double base, double exponent) {
    if (base < 0) {
        // Для отрицательного основания проверяем, является ли показатель целым числом
        if (std::abs(exponent - std::round(exponent)) < 1e-10) {
            return std::pow(base, exponent);
        }
        return std::numeric_limits<double>::quiet_NaN();
    }
    return std::pow(base, exponent);
}

PlotWidget::PlotWidget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(400, 300);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

PlotWidget::~PlotWidget()
{
}

void PlotWidget::addFunction(const QString &func, const QColor &color)
{
    Function newFunc(func, color);
    try {
        // Используем preprocessExpression для полной обработки выражения
        QString processedExpr = newFunc.preprocessExpression(func);
        qDebug() << "Преобразованное выражение:" << processedExpr;
        
        // Пробное вычисление для проверки корректности
        newFunc.xValue = 0.0;
        newFunc.parser->SetExpr(processedExpr.toStdString());
        double testResult = newFunc.parser->Eval();
        qDebug() << "Тестовое вычисление при x=0:" << testResult;
        
        functions[func] = newFunc;
        update();
    }
    catch (const mu::Parser::exception_type &e) {
        qDebug() << "Ошибка разбора функции:" << QString::fromStdString(e.GetMsg());
        qDebug() << "Код ошибки:" << e.GetCode();
        qDebug() << "Позиция ошибки:" << e.GetPos();
        qDebug() << "Токен:" << QString::fromStdString(e.GetToken());
    }
    catch (const std::exception &e) {
        qDebug() << "Стандартная ошибка C++:" << e.what();
    }
    catch (...) {
        qDebug() << "Неизвестная ошибка при добавлении функции";
    }
}

void PlotWidget::updateFunction(const QString &oldFunc, const QString &newFunc, const QColor &color)
{
    if (functions.contains(oldFunc)) {
        functions.remove(oldFunc);
    }
    addFunction(newFunc, color);
}

void PlotWidget::removeFunction(const QString &func)
{
    functions.remove(func);
    update();
}

void PlotWidget::wheelEvent(QWheelEvent *event)
{
    QPoint numDegrees = event->angleDelta() / 8;
    if (!numDegrees.isNull()) {
        double factor = std::pow(1.2, numDegrees.y() / 15.0);
        zoom(factor, event->position().toPoint());
    }
    event->accept();
}

void PlotWidget::zoom(double factor, QPoint center)
{
    // Получаем координаты точки, в которой происходит зум
    QPair<double, double> graphPos = transformToGraph(center.x(), center.y());
    
    // Изменяем масштаб
    double newXMin = graphPos.first - (graphPos.first - xMin) / factor;
    double newXMax = graphPos.first + (xMax - graphPos.first) / factor;
    double newYMin = graphPos.second - (graphPos.second - yMin) / factor;
    double newYMax = graphPos.second + (yMax - graphPos.second) / factor;
    
    xMin = newXMin;
    xMax = newXMax;
    yMin = newYMin;
    yMax = newYMax;
    
    update();
}

void PlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Заполняем фон градиентом
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, QColor(240, 240, 245));
    gradient.setColorAt(1, QColor(250, 250, 255));
    painter.fillRect(rect(), gradient);

    // Рисуем сетку и оси
    drawGrid(painter);
    drawAxes(painter);
    drawAxisLabels(painter);

    // Рисуем все функции
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        drawFunction(painter, it.key(), it.value());
    }

    // Рисуем координаты или точку на графике
    if (isMouseInWidget) {
        if (isAltPressed) {
            drawCoordinates(painter);
        } else if (hasNearestPoint && !functions.isEmpty()) {
            drawGraphPoint(painter);
        }
    }
}

void PlotWidget::drawGrid(QPainter &painter)
{
    // Основная сетка
    painter.setPen(QPen(QColor(220, 220, 230), 1, Qt::SolidLine));

    // Определяем шаг сетки в зависимости от масштаба
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    
    double xStep = std::pow(10, std::floor(std::log10(xRange)) - 1);
    double yStep = std::pow(10, std::floor(std::log10(yRange)) - 1);
    
    if (xRange / xStep < 5) xStep /= 2;
    if (yRange / yStep < 5) yStep /= 2;
    
    // Вертикальные линии
    for (double x = std::ceil(xMin / xStep) * xStep; x <= xMax; x += xStep) {
        QPoint p1 = transformToScreen(x, yMin);
        QPoint p2 = transformToScreen(x, yMax);
        painter.drawLine(p1, p2);
    }

    // Горизонтальные линии
    for (double y = std::ceil(yMin / yStep) * yStep; y <= yMax; y += yStep) {
        QPoint p1 = transformToScreen(xMin, y);
        QPoint p2 = transformToScreen(xMax, y);
        painter.drawLine(p1, p2);
    }

    // Дополнительная сетка (более мелкая)
    painter.setPen(QPen(QColor(235, 235, 240), 1, Qt::DotLine));
    double smallStep = xStep / 5;
    
    // Вертикальные линии
    for (double x = std::ceil(xMin / smallStep) * smallStep; x <= xMax; x += smallStep) {
        if (std::fmod(x, xStep) != 0) { // Пропускаем линии основной сетки
            QPoint p1 = transformToScreen(x, yMin);
            QPoint p2 = transformToScreen(x, yMax);
            painter.drawLine(p1, p2);
        }
    }

    // Горизонтальные линии
    smallStep = yStep / 5;
    for (double y = std::ceil(yMin / smallStep) * smallStep; y <= yMax; y += smallStep) {
        if (std::fmod(y, yStep) != 0) { // Пропускаем линии основной сетки
            QPoint p1 = transformToScreen(xMin, y);
            QPoint p2 = transformToScreen(xMax, y);
            painter.drawLine(p1, p2);
        }
    }
}

void PlotWidget::drawAxisLabels(QPainter &painter)
{
    painter.setPen(QColor(60, 60, 70));
    QFont font = painter.font();
    font.setPointSize(9);
    painter.setFont(font);

    // Определяем шаг меток в зависимости от масштаба
    double xRange = xMax - xMin;
    double yRange = yMax - yMin;
    
    double xStep = std::pow(10, std::floor(std::log10(xRange)) - 1);
    double yStep = std::pow(10, std::floor(std::log10(yRange)) - 1);
    
    if (xRange / xStep < 5) xStep /= 2;
    if (yRange / yStep < 5) yStep /= 2;

    // Метки на оси X
    for (double x = std::ceil(xMin / xStep) * xStep; x <= xMax; x += xStep) {
        if (std::abs(x) < xStep/2) continue;
        QPoint pos = transformToScreen(x, 0);
        QString label = QString::number(x);
        QRect textRect = painter.fontMetrics().boundingRect(label);
        
        // Рисуем маленькую черточку
        painter.drawLine(pos.x(), pos.y() - 3, pos.x(), pos.y() + 3);
        
        // Рисуем текст с фоном
        QRect bgRect = textRect.adjusted(-2, -2, 2, 2);
        bgRect.moveCenter(QPoint(pos.x(), pos.y() + textRect.height() + 5));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 200));
        painter.drawRect(bgRect);
        
        painter.setPen(QColor(60, 60, 70));
        painter.drawText(bgRect, Qt::AlignCenter, label);
    }

    // Метки на оси Y
    for (double y = std::ceil(yMin / yStep) * yStep; y <= yMax; y += yStep) {
        if (std::abs(y) < yStep/2) continue;
        QPoint pos = transformToScreen(0, y);
        QString label = QString::number(y);
        QRect textRect = painter.fontMetrics().boundingRect(label);
        
        // Рисуем маленькую черточку
        painter.drawLine(pos.x() - 3, pos.y(), pos.x() + 3, pos.y());
        
        // Рисуем текст с фоном
        QRect bgRect = textRect.adjusted(-4, -2, 4, 2);
        bgRect.moveCenter(QPoint(pos.x() - textRect.width() - 10, pos.y()));
        
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 255, 255, 200));
        painter.drawRect(bgRect);
        
        painter.setPen(QColor(60, 60, 70));
        painter.drawText(bgRect, Qt::AlignCenter, label);
    }
}

void PlotWidget::drawAxes(QPainter &painter)
{
    // Рисуем оси
    QPen axisPen(QColor(60, 60, 70), 2);
    painter.setPen(axisPen);

    // Ось X
    QPoint xAxis1 = transformToScreen(xMin, 0);
    QPoint xAxis2 = transformToScreen(xMax, 0);
    painter.drawLine(xAxis1, xAxis2);

    // Ось Y
    QPoint yAxis1 = transformToScreen(0, yMin);
    QPoint yAxis2 = transformToScreen(0, yMax);
    painter.drawLine(yAxis1, yAxis2);

    // Стрелки на концах осей
    int arrowSize = 12;
    double arrowAngle = 25.0; // угол стрелки в градусах
    
    // Стрелка оси X
    QPointF xArrowP1 = xAxis2;
    QPointF xArrowP2 = xAxis2 - QPointF(arrowSize * std::cos(qDegreesToRadians(arrowAngle)),
                                       arrowSize * std::sin(qDegreesToRadians(arrowAngle)));
    QPointF xArrowP3 = xAxis2 - QPointF(arrowSize * std::cos(qDegreesToRadians(-arrowAngle)),
                                       arrowSize * std::sin(qDegreesToRadians(-arrowAngle)));
    
    QPolygonF xArrow;
    xArrow << xArrowP1 << xArrowP2 << xArrowP3;
    
    // Стрелка оси Y
    QPointF yArrowP1 = yAxis2;
    QPointF yArrowP2 = yAxis2 - QPointF(arrowSize * std::sin(qDegreesToRadians(arrowAngle)),
                                       arrowSize * std::cos(qDegreesToRadians(arrowAngle)));
    QPointF yArrowP3 = yAxis2 - QPointF(-arrowSize * std::sin(qDegreesToRadians(arrowAngle)),
                                       arrowSize * std::cos(qDegreesToRadians(arrowAngle)));
    
    QPolygonF yArrow;
    yArrow << yArrowP1 << yArrowP2 << yArrowP3;
    
    // Рисуем стрелки
    painter.setBrush(QColor(60, 60, 70));
    painter.drawPolygon(xArrow);
    painter.drawPolygon(yArrow);
}

void PlotWidget::drawFunction(QPainter &painter, const QString &expr, Function &func)
{
    painter.setPen(QPen(func.color, 2.5));
    painter.setBrush(Qt::NoBrush);

    QVector<QPair<double, double>> points = calculatePoints(func);
    if (points.isEmpty()) return;

    QPainterPath path;
    bool pathStarted = false;
    QPoint prevPoint;
    double prevX = 0, prevY = 0;
    bool prevPointVisible = false;

    auto isPointVisible = [this](double x, double y) {
        // Пропускаем точку (0,0) и близкие к ней точки
        if (std::abs(x) < 1e-10 && std::abs(y) < 1e-10) {
            return false;
        }
        return std::isfinite(y) && y >= yMin && y <= yMax && x >= xMin && x <= xMax;
    };

    for (int i = 0; i < points.size(); ++i) {
        double x = points[i].first;
        double y = points[i].second;
        bool currentPointVisible = isPointVisible(x, y);
        QPoint currentPoint = transformToScreen(x, y);

        if (!pathStarted && currentPointVisible) {
            // Начинаем новый путь
            path.moveTo(currentPoint);
            pathStarted = true;
        } else if (pathStarted) {
            if (!currentPointVisible && !prevPointVisible) {
                // Обе точки невидимы, прерываем путь
                painter.drawPath(path);
                path = QPainterPath();
                pathStarted = false;
            } else {
                // Проверяем расстояние на экране
                int screenDX = currentPoint.x() - prevPoint.x();
                int screenDY = currentPoint.y() - prevPoint.y();
                double screenDistance = std::sqrt(screenDX * screenDX + screenDY * screenDY);

                if (screenDistance > height()) {
                    // Слишком большой разрыв, начинаем новый путь
                    painter.drawPath(path);
                    path = QPainterPath();
                    if (currentPointVisible) {
                        path.moveTo(currentPoint);
                    } else {
                        pathStarted = false;
                    }
                } else {
                    // Добавляем линию к текущей точке
                    path.lineTo(currentPoint);
                }
            }
        }

        prevPoint = currentPoint;
        prevX = x;
        prevY = y;
        prevPointVisible = currentPointVisible;
    }

    if (pathStarted) {
        painter.drawPath(path);
    }

    // Рисуем текст функции в правом верхнем углу
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);
    
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(expr);
    textRect.adjust(-5, -5, 5, 5);
    
    // Рисуем фон для текста
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.drawRect(textRect);
    
    // Рисуем текст черным цветом
    painter.setPen(Qt::black);
    painter.drawText(textRect, Qt::AlignCenter, expr);
}

QPoint PlotWidget::transformToScreen(double x, double y)
{
    int screenX = width() * (x - xMin) / (xMax - xMin);
    int screenY = height() * (1 - (y - yMin) / (yMax - yMin));
    return QPoint(screenX, screenY);
}

QPair<double, double> PlotWidget::transformToGraph(int screenX, int screenY)
{
    double x = xMin + (xMax - xMin) * screenX / width();
    double y = yMin + (yMax - yMin) * (1 - (double)screenY / height());
    return {x, y};
}

QVector<QPair<double, double>> PlotWidget::calculatePoints(const Function &func)
{
    QVector<QPair<double, double>> points;
    const int numPoints = width() * 8;
    const double step = (xMax - xMin) / numPoints;

    // Добавляем дополнительные точки около нуля для функций типа 1/x
    if (xMin < 0 && xMax > 0) {
        // Точки слева от нуля
        for (int i = -10; i < 0; ++i) {
            double x = step * i / 1000.0;
            double y = evaluateFunction(x, func);
            if (std::isfinite(y)) {
                points.append({x, y});
            }
        }
        
        // Точки справа от нуля
        for (int i = 1; i <= 10; ++i) {
            double x = step * i / 1000.0;
            double y = evaluateFunction(x, func);
            if (std::isfinite(y)) {
                points.append({x, y});
            }
        }
    }

    // Основные точки графика
    for (int i = 0; i <= numPoints; ++i) {
        double x = xMin + i * step;
        // Пропускаем точку x = 0 для функций типа 1/x
        if (std::abs(x) < step/1000.0) continue;
        
        double y = evaluateFunction(x, func);
        points.append({x, y});
    }

    return points;
}

double PlotWidget::evaluateFunction(double x, const Function &func) const
{
    try {
        func.xValue = x;
        return func.parser->Eval();
    }
    catch (const mu::Parser::exception_type &e) {
        qDebug() << "Ошибка вычисления функции при x =" << x << ":" << QString::fromStdString(e.GetMsg());
        return std::numeric_limits<double>::quiet_NaN();
    }
    catch (...) {
        qDebug() << "Неизвестная ошибка при вычислении функции при x =" << x;
        return std::numeric_limits<double>::quiet_NaN();
    }
}

void PlotWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPanning = true;
        lastMousePos = event->pos();
        setCursor(Qt::ClosedHandCursor);
    }
}

void PlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        isPanning = false;
        setCursor(Qt::ArrowCursor);
    }
}

void PlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (isPanning) {
        QPoint delta = event->pos() - lastMousePos;
        pan(delta);
        lastMousePos = event->pos();
    }
    
    currentMousePos = event->pos();
    isMouseInWidget = true;

    // Находим ближайшую точку на графике, если не зажат Alt
    if (!isAltPressed && !functions.isEmpty()) {
        nearestPoint = findNearestPoint(currentMousePos);
        hasNearestPoint = true;
    }
    
    update();
}

void PlotWidget::pan(const QPoint &delta)
{
    // Преобразуем смещение в пикселях в смещение в координатах графика
    double dx = -(xMax - xMin) * delta.x() / width();
    double dy = (yMax - yMin) * delta.y() / height();

    // Смещаем область просмотра
    xMin += dx;
    xMax += dx;
    yMin += dy;
    yMax += dy;

    update();
}

void PlotWidget::drawCoordinates(QPainter &painter)
{
    // Получаем координаты в системе графика
    QPair<double, double> coords = transformToGraph(currentMousePos.x(), currentMousePos.y());
    
    // Форматируем координаты с двумя знаками после запятой
    QString coordText = QString("x: %1\ny: %2")
        .arg(coords.first, 0, 'f', 2)
        .arg(coords.second, 0, 'f', 2);

    // Настраиваем внешний вид текста
    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    // Создаем прямоугольник для фона текста
    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(QRect(0, 0, 1000, 1000), Qt::AlignLeft, coordText);
    textRect.adjust(-5, -5, 5, 5); // Добавляем отступы

    // Вычисляем позицию для отображения (смещение от курсора)
    QPoint textPos = currentMousePos + QPoint(10, 10);
    textRect.moveTopLeft(textPos);

    // Проверяем, не выходит ли текст за пределы виджета
    if (textRect.right() > width()) {
        textRect.moveRight(currentMousePos.x() - 10);
    }
    if (textRect.bottom() > height()) {
        textRect.moveBottom(currentMousePos.y() - 10);
    }

    // Рисуем фон с полупрозрачностью
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255, 230));
    painter.drawRect(textRect);

    // Рисуем рамку
    painter.setPen(QPen(Qt::black, 1));
    painter.drawRect(textRect);

    // Рисуем текст
    painter.drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, coordText);
}

void PlotWidget::leaveEvent(QEvent *event)
{
    isMouseInWidget = false;
    update();
    QWidget::leaveEvent(event);
}

void PlotWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt) {
        isAltPressed = true;
        update();
    }
    QWidget::keyPressEvent(event);
}

void PlotWidget::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt) {
        isAltPressed = false;
        update();
    }
    QWidget::keyReleaseEvent(event);
}

QPair<double, double> PlotWidget::findNearestPoint(const QPoint &mousePos)
{
    if (functions.isEmpty()) {
        return {0, 0};
    }

    // Получаем x-координату мыши в системе графика
    QPair<double, double> mouseCoords = transformToGraph(mousePos.x(), mousePos.y());
    double mouseX = mouseCoords.first;
    double bestDistance = std::numeric_limits<double>::infinity();
    QPair<double, double> bestPoint;

    // Проверяем каждую функцию
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        double y = evaluateFunction(mouseX, it.value());
        if (std::isfinite(y)) {
            double distance = std::abs(y - mouseCoords.second);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestPoint = {mouseX, y};
            }
        }
    }

    return bestPoint;
}

void PlotWidget::drawGraphPoint(QPainter &painter)
{
    if (!hasNearestPoint || functions.isEmpty()) {
        return;
    }

    QPoint screenPoint = transformToScreen(nearestPoint.first, nearestPoint.second);
    
    // Рисуем вертикальную пунктирную линию от оси X до точки
    painter.setPen(QPen(QColor(41, 128, 185, 100), 1, Qt::DashLine));
    painter.drawLine(screenPoint.x(), transformToScreen(0, 0).y(), screenPoint.x(), screenPoint.y());
    
    // Рисуем горизонтальную пунктирную линию от оси Y до точки
    painter.drawLine(transformToScreen(0, 0).x(), screenPoint.y(), screenPoint.x(), screenPoint.y());

    // Рисуем внешний круг точки
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 255));
    painter.drawEllipse(screenPoint, 7, 7);

    // Рисуем внутренний круг точки
    painter.setBrush(QColor(41, 128, 185));
    painter.drawEllipse(screenPoint, 5, 5);

    // Рисуем координаты точки
    QString coordText = QString("x: %1\ny: %2")
        .arg(nearestPoint.first, 0, 'f', 2)
        .arg(nearestPoint.second, 0, 'f', 2);

    QFont font = painter.font();
    font.setPointSize(10);
    painter.setFont(font);

    QFontMetrics fm(font);
    QRect textRect = fm.boundingRect(QRect(0, 0, 1000, 1000), Qt::AlignLeft, coordText);
    textRect.adjust(-8, -8, 8, 8);

    QPoint textPos = screenPoint + QPoint(15, 15);
    textRect.moveTopLeft(textPos);

    if (textRect.right() > width()) {
        textRect.moveRight(screenPoint.x() - 15);
    }
    if (textRect.bottom() > height()) {
        textRect.moveBottom(screenPoint.y() - 15);
    }

    // Создаем эффект тени для окна координат
    QColor shadowColor(0, 0, 0, 30);
    for (int i = 0; i < 5; ++i) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(shadowColor);
        painter.drawRect(textRect.adjusted(i, i, i, i));
    }

    // Рисуем фон с градиентом
    QLinearGradient gradient(textRect.topLeft(), textRect.bottomLeft());
    gradient.setColorAt(0, QColor(255, 255, 255, 250));
    gradient.setColorAt(1, QColor(245, 245, 250, 250));
    painter.setBrush(gradient);
    painter.setPen(QPen(QColor(200, 200, 210), 1));
    painter.drawRect(textRect);

    // Рисуем текст
    painter.setPen(QColor(60, 60, 70));
    painter.drawText(textRect, Qt::AlignCenter, coordText);
} 