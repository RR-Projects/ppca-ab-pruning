#pragma once

#include <math.h>

struct Point {
public:
    int x, y;

    Point() { }

    Point(int pos, int boardX) {
        x = pos % boardX;
        y = pos / boardX;
    }

    Point inline operator-(const Point& p) {
        Point r;
        r.x = x - p.x;
        r.y = y - p.y;
        return r;
    }

    bool inline operator==(const Point& p) {
        return x == p.x && y == p.y;
    }

    bool inline isZero() {
        return x == 0 && y == 0;
    }

    bool inline isAdjacent(const Point& p) {
        return abs(x - p.x) < 2 && abs(y - p.y) < 2;
    }

};