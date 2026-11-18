#ifndef POLYGON_HPP
#define POLYGON_HPP

#include <vector>
#include <utility>
#include "math/cell3DPosition.h"

class Polygon {
public:
    Polygon() = default;
    explicit Polygon(const std::vector<Cell3DPosition>& vertices);

    void addVertex(const Cell3DPosition& vertex);
    void removeVertex(const Cell3DPosition& vertex);
    double area() const;
    double perimeter() const;
    bool isConvex() const;

    std::vector<Cell3DPosition>& getVertices();
     std::vector<Cell3DPosition> convexHull() const;

private:
    std::vector<Cell3DPosition> vertices;
    Cell3DPosition centroid;

    double distance(const Cell3DPosition& p1, const Cell3DPosition& p2) const;
    bool compareClockwise(const Cell3DPosition& p1, const Cell3DPosition& p2) const;
    Cell3DPosition calculateCentroid() const;

    void sortVerticesClockwise();
   
};

#endif // POLYGON_HPP