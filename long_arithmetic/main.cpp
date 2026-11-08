#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QVector>


static QString strip(const QString &s)
{
    QString r = s;
    while (r.size() > 1 && r[0] == '0') r.remove(0, 1);
    return r.isEmpty() ? "0" : r;
}

// |a| + |b|
static QString addAbs(QString a, QString b)
{
    if (a.size() < b.size()) std::swap(a, b);
    int diff = a.size() - b.size();
    QString res;
    int carry = 0;
    for (int i = a.size() - 1; i >= -1; --i) {
        int da = (i >= 0) ? a[i].digitValue() : 0;
        int db = (i - diff >= 0) ? b[i - diff].digitValue() : 0;
        if (i < 0 && carry == 0) break;
        int s = da + db + carry;
        res.prepend(QChar('0' + s % 10));
        carry = s / 10;
    }
    return strip(res);
}

// |a| - |b|, a >= b
static QString subAbs(QString a, QString b)
{
    int diff = a.size() - b.size();
    QString res;
    int borrow = 0;
    for (int i = a.size() - 1; i >= 0; --i) {
        int da = a[i].digitValue();
        int db = (i - diff >= 0) ? b[i - diff].digitValue() : 0;
        int d = da - db - borrow;
        if (d < 0) { d += 10; borrow = 1; } else borrow = 0;
        res.prepend(QChar('0' + d));
    }
    return strip(res);
}

// сравнение модулей
static int cmpAbs(const QString &a, const QString &b)
{
    if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
    if (a == b) return 0;
    return a < b ? -1 : 1;
}


static void split(const QString &s, bool &neg, QString &ip, QString &fp)
{
    QString t = s.trimmed();
    neg = false;
    if (t.startsWith('-')) { neg = true; t.remove(0, 1); }
    int dot = t.indexOf('.');
    if (dot == -1) { ip = t; fp = ""; }
    else { ip = t.left(dot); fp = t.mid(dot + 1); }
    if (ip.isEmpty()) ip = "0";
    while (fp.endsWith('0')) fp.chop(1);
    while (ip.size() > 1 && ip[0] == '0') ip.remove(0, 1);
}

static QString join(bool neg, QString ip, QString fp)
{
    while (ip.size() > 1 && ip[0] == '0') ip.remove(0, 1);
    while (fp.endsWith('0')) fp.chop(1);
    if (ip == "0" && fp.isEmpty()) return "0";
    return (neg ? "-" : "") + ip + (fp.isEmpty() ? "" : "." + fp);
}

static QString add(QString A, QString B)
{
    bool na, nb; QString ia, fa, ib, fb;
    split(A, na, ia, fa);
    split(B, nb, ib, fb);


    int flen = qMax(fa.size(), fb.size());
    fa = fa.leftJustified(flen, '0');
    fb = fb.leftJustified(flen, '0');


    QString a = strip(ia + fa);
    QString b = strip(ib + fb);


    QString r;
    bool neg;
    if (na == nb) {
        r = addAbs(a, b);
        neg = na;
    } else {
        int c = cmpAbs(a, b);
        if (c == 0) return "0";
        if (c > 0) {
            r = subAbs(a, b);
            neg = na;
        } else {
            r = subAbs(b, a);
            neg = nb;
        }
    }


    QString ri, rf;
    if (flen > 0) {
        if (r.size() <= flen) {
            r = QString(flen - r.size() + 1, '0') + r;
        }
        ri = r.left(r.size() - flen);
        rf = r.mid(r.size() - flen);
    } else {
        ri = r;
        rf = "";
    }

    return join(neg, ri, rf);
}

static QString sub(QString A, QString B)
{
    if (B.startsWith('-')) B.remove(0, 1);
    else B.prepend('-');
    return add(A, B);
}

static QString mul(QString A, QString B)
{
    bool na, nb; QString ia, fa, ib, fb;
    split(A, na, ia, fa); split(B, nb, ib, fb);

    QString a = strip(ia + fa), b = strip(ib + fb);
    if (a == "0" || b == "0") return "0";

    QVector<int> res(a.size() + b.size(), 0);
    for (int i = a.size() - 1; i >= 0; --i)
        for (int j = b.size() - 1; j >= 0; --j) {
            int s = (a[i].digitValue()) * (b[j].digitValue()) + res[i + j + 1];
            res[i + j + 1] = s % 10;
            res[i + j] += s / 10;
        }

    QString r;
    for (int v : res) if (!(r.isEmpty() && v == 0)) r += QChar('0' + v);

    int flen = fa.size() + fb.size();
    if (flen > 0) {
        if (r.size() <= flen) r = QString(flen - r.size() + 1, '0') + r;
        r.insert(r.size() - flen, '.');
    }
    return join(na != nb, r.section('.', 0, 0), r.section('.', 1));
}

static QString div(QString A, QString B)
{
    bool na, nb; QString ia, fa, ib, fb;
    split(A, na, ia, fa); split(B, nb, ib, fb);

    QString a = strip(ia + fa), b = strip(ib + fb);
    if (b == "0") return "Деление на ноль!";

    const int PREC = 40;
    int shift = fb.size() - fa.size() + PREC;
    if (shift > 0) a += QString(shift, '0');
    else if (shift < 0) b += QString(-shift, '0');

    QString res, rem;
    for (QChar ch : a) {
        rem += ch;
        rem = strip(rem);
        int d = 0;
        while (cmpAbs(rem, b) >= 0) { rem = subAbs(rem, b); ++d; }
        res += QChar('0' + d);
    }
    res = strip(res);

    if (res.size() <= PREC) res = QString(PREC - res.size() + 1, '0') + res;
    res.insert(res.size() - PREC, '.');

    return join(na != nb, res.section('.', 0, 0), res.section('.', 1));
}

//Предикат поворота

// Знак числа: -1, 0, +1
static int signOf(const QString &s)
{
    bool neg; QString ip, fp;
    split(s, neg, ip, fp);
    if (ip == "0" && fp.isEmpty()) return 0;
    return neg ? -1 : 1;
}


static int turn(const QString &Ax, const QString &Ay,
                const QString &Bx, const QString &By,
                const QString &Cx, const QString &Cy)
{
    QString t1 = mul(sub(Bx, Ax), sub(Cy, Ay));
    QString t2 = mul(sub(By, Ay), sub(Cx, Ax));
    QString d  = sub(t1, t2);
    return signOf(d);
}


static QString turnText(int t)
{
    if (t > 0) return "слева";
    if (t < 0) return "справа";
    return "на прямой";
}



class TurnWindow : public QWidget
{
public:
    TurnWindow(QWidget *parent = nullptr) : QWidget(parent)
    {
        setWindowTitle("Предикат");
        QVBoxLayout *v = new QVBoxLayout(this);

        // Точка A
        QHBoxLayout *rowA = new QHBoxLayout;
        QLineEdit *ax = new QLineEdit; ax->setPlaceholderText("Ax");
        QLineEdit *ay = new QLineEdit; ay->setPlaceholderText("Ay");
        rowA->addWidget(new QLabel("A:")); rowA->addWidget(ax); rowA->addWidget(ay);

        // Точка B
        QHBoxLayout *rowB = new QHBoxLayout;
        QLineEdit *bx = new QLineEdit; bx->setPlaceholderText("Bx");
        QLineEdit *by = new QLineEdit; by->setPlaceholderText("By");
        rowB->addWidget(new QLabel("B:")); rowB->addWidget(bx); rowB->addWidget(by);

        // Точка C
        QHBoxLayout *rowC = new QHBoxLayout;
        QLineEdit *cx = new QLineEdit; cx->setPlaceholderText("Cx");
        QLineEdit *cy = new QLineEdit; cy->setPlaceholderText("Cy");
        rowC->addWidget(new QLabel("C:")); rowC->addWidget(cx); rowC->addWidget(cy);

        QLabel *res = new QLabel("Результат:");
        res->setWordWrap(true);
        res->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QPushButton *btn = new QPushButton("Определить положение C");

        v->addLayout(rowA);
        v->addLayout(rowB);
        v->addLayout(rowC);
        v->addWidget(btn);
        v->addWidget(res);

        QObject::connect(btn, &QPushButton::clicked, [=]{
            // Простейшая валидация: поля не должны быть пустыми
            if (ax->text().trimmed().isEmpty() || ay->text().trimmed().isEmpty() ||
                bx->text().trimmed().isEmpty() || by->text().trimmed().isEmpty() ||
                cx->text().trimmed().isEmpty() || cy->text().trimmed().isEmpty())
            {
                res->setText("Результат: заполните все шесть координат");
                return;
            }

            // Вычисляем векторное произведение (длинная арифметика)
            QString t1 = mul(sub(bx->text(), ax->text()), sub(cy->text(), ay->text()));
            QString t2 = mul(sub(by->text(), ay->text()), sub(cx->text(), ax->text()));
            QString cross = sub(t1, t2);

            int t = signOf(cross);
            res->setText("Результат: " + cross + " — точка C " + turnText(t));
        });

        resize(560, 260);
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget w;
    w.setWindowTitle("Длинная арифметика");
    QVBoxLayout *v = new QVBoxLayout(&w);

    QLineEdit *a = new QLineEdit; a->setPlaceholderText("Первое число");
    QLineEdit *b = new QLineEdit; b->setPlaceholderText("Второе число");
    QLabel *res = new QLabel("Результат:");
    res->setWordWrap(true);
    res->setTextInteractionFlags(Qt::TextSelectableByMouse);

    QHBoxLayout *h = new QHBoxLayout;
    QPushButton *bAdd = new QPushButton("+");
    QPushButton *bSub = new QPushButton("−");
    QPushButton *bMul = new QPushButton("×");
    QPushButton *bDiv = new QPushButton("÷");
    h->addWidget(bAdd); h->addWidget(bSub);
    h->addWidget(bMul); h->addWidget(bDiv);

    v->addWidget(a); v->addWidget(b); v->addLayout(h); v->addWidget(res);

    QObject::connect(bAdd, &QPushButton::clicked, [&]{ res->setText("Результат: " + add(a->text(), b->text())); });
    QObject::connect(bSub, &QPushButton::clicked, [&]{ res->setText("Результат: " + sub(a->text(), b->text())); });
    QObject::connect(bMul, &QPushButton::clicked, [&]{ res->setText("Результат: " + mul(a->text(), b->text())); });
    QObject::connect(bDiv, &QPushButton::clicked, [&]{ res->setText("Результат: " + div(a->text(), b->text())); });


    QPushButton *bTurn = new QPushButton("Поворот");
    h->addWidget(bTurn);

    TurnWindow *tw = new TurnWindow;
    tw->setAttribute(Qt::WA_DeleteOnClose, false);
    QObject::connect(bTurn, &QPushButton::clicked, [tw]{
        tw->show();
        tw->raise();
        tw->activateWindow();
    });

    w.resize(600, 300);
    w.show();
    return app.exec();
}
