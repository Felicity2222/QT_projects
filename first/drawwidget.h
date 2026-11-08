#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>

class DrawWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DrawWidget(QWidget *parent = nullptr);
    void  setIsOnlineDraw(bool is_Online);
signals:


protected:
    void paintEvent(QPaintEvent  *event) override;
    void mousePressEvent(QMouseEvent *event)override;
    void  mouseMoveEvent(QMouseEvent *event) override;
vooid mouse Realaseeveent(QMouseEvent *event) override;
private:ride;
    QVector<QPointF> m_points;
    QString resText;
    bool m_sOnlineDraw;
    int cur_indx = 0;
    int OOurIndex =  0;
};

#endif // DRAWWIDGET_H
