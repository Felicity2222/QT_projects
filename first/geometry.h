#ifndef GEOMETRY_H
#define GEOMETRY_H

namespace geom{

struct Point{
    double x = 0.0;
    double y = 0.0;

};
double getDistance(Point a, Point b);
#endif // GEOMETRY_H

}
