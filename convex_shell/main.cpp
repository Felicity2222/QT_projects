#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QPointF>
#include <algorithm>
#include <cmath>


// > 0 — левый поворот, < 0 — правый, = 0 — коллинеарны.
static double cross(const QPointF &p0, const QPointF &p1, const QPointF &p2)
{
    return (p1.x() - p0.x()) * (p2.y() - p0.y())
    - (p1.y() - p0.y()) * (p2.x() - p0.x());
}


static double dist2(const QPointF &a, const QPointF &b)
{
    double dx = a.x() - b.x();
    double dy = a.y() - b.y();
    return dx * dx + dy * dy;
}



static QVector<int> grahamScan(const QVector<QPointF> &pts)
{
    int n = pts.size();
    if (n < 3) {
        QVector<int> r;
        for (int i = 0; i < n; ++i) r.append(i);
        return r;
    }


    int start = 0;
    for (int i = 1; i < n; ++i) {
        if (pts[i].y() < pts[start].y() ||
            (pts[i].y() == pts[start].y() && pts[i].x() < pts[start].x()))
            start = i;
    }


    QVector<int> order;
    order.reserve(n);
    for (int i = 0; i < n; ++i) if (i != start) order.append(i);

    std::sort(order.begin(), order.end(), [&](int a, int b) {
        double c = cross(pts[start], pts[a], pts[b]);
        if (std::fabs(c) < 1e-9) {

            return dist2(pts[start], pts[a]) < dist2(pts[start], pts[b]);
        }
        return c > 0;
    });


    QVector<int> hull;
    hull.append(start);
    hull.append(order[0]);

    for (int k = 1; k < order.size(); ++k) {
        int cur = order[k];

        while (hull.size() >= 2 &&
               cross(pts[hull[hull.size() - 2]], pts[hull.last()], pts[cur]) <= 0)
            hull.removeLast();
        hull.append(cur);
    }
    return hull;
}


class HullWidget : public QWidget
{
public:
    HullWidget(QWidget *parent = nullptr) : QWidget(parent) {}

protected:
    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) {
            pts.append(e->pos());
            rebuildHull();
            update();
        } else if (e->button() == Qt::RightButton) {

            pts.clear();
            hull.clear();
            update();
        }
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);


        if (hull.size() >= 2) {
            p.setPen(QPen(Qt::darkGreen, 2));
            for (int i = 0; i < hull.size(); ++i) {
                int a = hull[i];
                int b = hull[(i + 1) % hull.size()];
                p.drawLine(pts[a], pts[b]);
            }
        }


        for (int i = 0; i < pts.size(); ++i) {
            bool onHull = hull.contains(i);
            p.setBrush(onHull ? Qt::green : Qt::red);
            p.setPen(Qt::black);
            p.drawEllipse(pts[i], 5, 5);
        }
    }

private:
    QVector<QPointF> pts;
    QVector<int>     hull;

    void rebuildHull()
    {
        hull = grahamScan(pts);
    }
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    HullWidget w;
    w.setWindowTitle("Выпуклая оболочка (алгоритм Грэхема)");
    w.resize(800, 600);
    w.setStyleSheet("background-color: white;");
    w.show();

    return app.exec();
}
