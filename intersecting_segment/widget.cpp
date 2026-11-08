#include "widget.h"
#include <QPainter>
#include <QMouseEvent>
#include <cmath>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(600, 600);
    setWindowTitle("Пересечение отрезков (Widget)");

    // Начальные координаты
    seg1_start = QPoint(100, 100);
    seg1_end   = QPoint(400, 400);

    seg2_start = QPoint(100, 400);
    seg2_end   = QPoint(400, 100);

    isDraggingSeg1Start = false;
    isDraggingSeg1End = false;
    isDraggingSeg2Start = false;
    isDraggingSeg2End = false;
}

Widget::~Widget() {}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    bool intersect = areSegmentsIntersecting(seg1_start, seg1_end, seg2_start, seg2_end);

    // Меняем цвет фона
    if (intersect) {
        painter.fillRect(rect(), QColor(144, 238, 144)); // Зеленый
    } else {
        painter.fillRect(rect(), QColor(255, 182, 193)); // Красный
    }

    // Рисуем линии
    painter.setPen(QPen(Qt::blue, 4));
    painter.drawLine(seg1_start, seg1_end);

    painter.setPen(QPen(Qt::magenta, 4));
    painter.drawLine(seg2_start, seg2_end);

    // Рисуем маркеры концов
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::blue);
    painter.drawEllipse(seg1_start, 8, 8);
    painter.drawEllipse(seg1_end, 8, 8);

    painter.setBrush(Qt::magenta);
    painter.drawEllipse(seg2_start, 8, 8);
    painter.drawEllipse(seg2_end, 8, 8);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    double threshold = 15.0;

    if (distance(pos, seg1_start) < threshold) isDraggingSeg1Start = true;
    else if (distance(pos, seg1_end) < threshold) isDraggingSeg1End = true;
    else if (distance(pos, seg2_start) < threshold) isDraggingSeg2Start = true;
    else if (distance(pos, seg2_end) < threshold) isDraggingSeg2End = true;
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();

    if (isDraggingSeg1Start) { seg1_start = pos; update(); }
    else if (isDraggingSeg1End) { seg1_end = pos; update(); }
    else if (isDraggingSeg2Start) { seg2_start = pos; update(); }
    else if (isDraggingSeg2End) { seg2_end = pos; update(); }
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    isDraggingSeg1Start = false;
    isDraggingSeg1End = false;
    isDraggingSeg2Start = false;
    isDraggingSeg2End = false;
}

double Widget::distance(QPoint p1, QPoint p2)
{
    return std::sqrt(std::pow(p1.x() - p2.x(), 2) + std::pow(p1.y() - p2.y(), 2));
}

bool Widget::areSegmentsIntersecting(QPoint p1, QPoint p2, QPoint p3, QPoint p4)
{
    auto crossProduct = [](QPoint a, QPoint b) {
        return a.x() * b.y() - a.y() * b.x();
    };

    auto subtract = [](QPoint a, QPoint b) {
        return QPoint(a.x() - b.x(), a.y() - b.y());
    };

    QPoint r = subtract(p2, p1);
    QPoint s = subtract(p4, p3);
    QPoint qp = subtract(p3, p1);

    double r_cross_s = crossProduct(r, s);
    double qp_cross_r = crossProduct(qp, r);

    if (std::abs(r_cross_s) < 1e-9) {
        if (std::abs(qp_cross_r) < 1e-9) {
            // Коллинеарные отрезки (проверка наложения)
            QPoint min1(std::min(p1.x(), p2.x()), std::min(p1.y(), p2.y()));
            QPoint max1(std::max(p1.x(), p2.x()), std::max(p1.y(), p2.y()));
            QPoint min2(std::min(p3.x(), p4.x()), std::min(p3.y(), p4.y()));
            QPoint max2(std::max(p3.x(), p4.x()), std::max(p3.y(), p4.y()));

            return (min1.x() <= max2.x() && max1.x() >= min2.x() &&
                    min1.y() <= max2.y() && max1.y() >= min2.y());
        }
        return false;
    }

    double t = crossProduct(qp, s) / r_cross_s;
    double u = crossProduct(qp, r) / r_cross_s;

    return (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0);
}
