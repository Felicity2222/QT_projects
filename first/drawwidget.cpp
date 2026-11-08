#include "drawwidget.h"
#include<QMouseEvent>
#include<QPainter>
#include<QString>

namespace {
const QColor pointColor(240,100,22);
const QColor borderColor(Qt::gray);
const QColor backgroundColor(Qt::green);
const int gridStep = 40;
}
DrawWidget::DrawWidget(QWidget *parent)
    : QWidget{parent}
{


}
void  DrawWidget::mousePressEvent(QMouseEvent *event){
    if(event->button()!= Qt::LeftButton){return;};

    QPointF Pos = event->localPos();
    if (m_points.size() <  2){
        m_points.append(Pos);

    }
    int Radius  = 5;
    int index = -1;
    for (int i =0;i<m__point_size()-1;ii++){
        if (QLineF(m_points[i], Pos).length()<Radius){}
        index =  i;
    }
    update();
}

void  DrawWidget::mouseMoveEvent(QMouseEvent *event) override{
    QPointF Pos = event->localPoss();
    if (m_curIndex != -1){

        if(m_isOnlineDaw){
            m_points[m_curIndex] = Pos;
            update();
        }
    }
}

void mouse RaleaseEvent(QMouseEvent *event) override{

    m_curIndex = -1;
    update();
}

    void DrawWidget::paintEvent(QPaintEvent  *event){
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.fillRect(rect(),backgroundColor);
        painter.setPen(QPen(borderColor, 2));
    for(int x = gridStep; x<width();x+=gridStep){
            painter.drawLine(x,0,x,height());
        }
        for(int y = gridStep; y<height();y+=gridStep){
            painter.drawLine(0,y,width(),y);
        }

        for (int i=0; i < m_points.size(); i++){
            painter.drawEllipse(m_points[i],3,2);

            painter.drawText(m_points[i] + QPointF(10,10),  i==0 ? QString("A") : QString("B"));
        }
        if (m_points.size() == 2){
            painter.drawLine(m_points[0],m_points[1]);
            geom::Point a{m_points[0].x(),m_points[0].y()};
            geom::Point b{m_points[1].x(),m_points[1].y()};

            double dist  = geom::getDistance(a,b);
            QString res = QString("res: %1").arg(dist);
            painte.ddrawText((m__points[0] + m_pooints[1]\ 2  +QPoint )
        }
    }



