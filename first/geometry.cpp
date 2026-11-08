#include "geometry.h"
#include "cmath"

double getDistance(Point a, Point b){
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

