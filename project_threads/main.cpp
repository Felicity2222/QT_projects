#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QThread>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QRandomGenerator>
#include <QVector>
#include <QPointF>
#include <QSet>
#include <QPair>
#include <QDateTime>
#include <QElapsedTimer>
#include <QDebug>
#include <QPolygonF>
#include <QLineF>
#include <algorithm>
#include <numeric>

struct Point {
    double x, y;
};

// ---------------------- Геометрия ----------------------
static inline double cross(const Point& O, const Point& A, const Point& B) {
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

// ---------------------- Последовательная оболочка (Эндрю) ----------------------
static QVector<Point> buildHullSequential(QVector<Point> pts) {
    if (pts.size() < 3) return pts;

    std::sort(pts.begin(), pts.end(), [](const Point& a, const Point& b) {
        return a.x < b.x || (a.x == b.x && a.y < b.y);
    });

    QVector<Point> hull;
    hull.reserve(pts.size() * 2);

    // нижняя цепочка
    for (const Point& p : pts) {
        while (hull.size() >= 2 &&
               cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    // верхняя цепочка
    int lowerSize = hull.size();
    for (int i = pts.size() - 2; i >= 0; --i) {
        const Point& p = pts[i];
        while (hull.size() > lowerSize &&
               cross(hull[hull.size() - 2], hull[hull.size() - 1], p) <= 0)
            hull.pop_back();
        hull.push_back(p);
    }

    hull.pop_back(); // последняя = первой
    return hull;
}

// ---------------------- Параллельный вариант ----------------------
static QVector<Point> buildHullParallel(QVector<Point> pts) {
    if (pts.size() < 1000)
        return buildHullSequential(std::move(pts));

    const int blockCount = QThread::idealThreadCount();
    const int blockSize  = (pts.size() + blockCount - 1) / blockCount;

    QVector<QVector<Point>> blocks;
    blocks.reserve(blockCount);
    for (int i = 0; i < blockCount; ++i) {
        int from = i * blockSize;
        int to   = std::min<int>(pts.size(), from + blockSize);
        if (from >= to) break;
        blocks.push_back(pts.mid(from, to - from));
    }

    QVector<QVector<Point>> localHulls(blocks.size());

    QVector<int> indices(blocks.size());
    std::iota(indices.begin(), indices.end(), 0);

    QFuture<void> fut = QtConcurrent::map(indices, [&](int i) {
        localHulls[i] = buildHullSequential(blocks[i]);
    });
    fut.waitForFinished();

    QVector<Point> merged;
    int total = 0;
    for (const auto& h : localHulls) total += h.size();
    merged.reserve(total);
    for (const auto& h : localHulls)
        merged.append(h);

    return buildHullSequential(std::move(merged));
}

// ---------------------- Виджет ----------------------
class HullWidget : public QWidget {
public:
    HullWidget(QVector<Point> pts, QVector<Point> hull)
        : m_pts(std::move(pts)), m_hull(std::move(hull)) {}

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), Qt::white);

        // --- оболочка: заливка + линии по контуру ---
        if (m_hull.size() >= 3) {
            QPolygonF poly;
            poly.reserve(m_hull.size() + 1);
            for (const Point& pt : m_hull)
                poly << QPointF(pt.x, pt.y);

            // зелёная заливка
            p.setBrush(QColor(0, 200, 0, 160));
            p.setPen(Qt::NoPen);
            p.drawPolygon(poly);

            // зелёный контур (линии между вершинами, контур замкнут)
            QPen pen(QColor(0, 120, 0));
            pen.setWidthF(2.0);
            pen.setJoinStyle(Qt::RoundJoin);
            pen.setCapStyle(Qt::RoundCap);
            p.setPen(pen);
            p.setBrush(Qt::NoBrush);

            QPolygonF closed = poly;
            if (!closed.isEmpty())
                closed << closed.first(); // замыкаем: последняя -> первая

            p.drawPolyline(closed);
        }

        // множество вершин оболочки
        QSet<QPair<int, int>> hullSet;
        hullSet.reserve(m_hull.size());
        for (const Point& pt : m_hull)
            hullSet.insert({int(pt.x * 1000), int(pt.y * 1000)});

        // красные точки — все, что не попали в оболочку
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::red);
        for (const Point& pt : m_pts) {
            if (!hullSet.contains({int(pt.x * 1000), int(pt.y * 1000)}))
                p.drawEllipse(QPointF(pt.x, pt.y), 1.2, 1.2);
        }

        // синие вершины оболочки
        p.setBrush(Qt::blue);
        for (const Point& pt : m_hull)
            p.drawEllipse(QPointF(pt.x, pt.y), 3, 3);
    }

private:
    QVector<Point> m_pts;
    QVector<Point> m_hull;
};

// ---------------------- main ----------------------
int main(int argc, char** argv) {
    QApplication app(argc, argv);

    const int W = 1000, H = 800;
    const int N = 10000;

    QRandomGenerator rng(QDateTime::currentMSecsSinceEpoch());

    QVector<Point> pts;
    pts.reserve(N);
    for (int i = 0; i < N; ++i) {
        pts.push_back({ double(rng.bounded(10, W - 10)),
                       double(rng.bounded(10, H - 10)) });
    }

    QElapsedTimer t;
    t.start();
    QVector<Point> hull = buildHullParallel(pts);
    qDebug("Hull: %d points, time: %lld ms",
           hull.size(), t.elapsed());

    HullWidget w(pts, hull);
    w.resize(W, H);
    w.setWindowTitle(QStringLiteral("Graham hull (parallel) — %1 pts, hull %2")
                         .arg(N).arg(hull.size()));
    w.show();

    return app.exec();
}
