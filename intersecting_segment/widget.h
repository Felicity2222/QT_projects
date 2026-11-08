#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QPoint>

class Widget : public QWidget{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    bool areSegmentsIntersecting(QPoint p1, QPoint p2, QPoint p3, QPoint p4);
    double distance(QPoint p1, QPoint p2);

    QPoint seg1_start, seg1_end;
    QPoint seg2_start, seg2_end;

    bool isDraggingSeg1Start;
    bool isDraggingSeg1End;
    bool isDraggingSeg2Start;
    bool isDraggingSeg2End;
};

#endif // WIDGET_H
