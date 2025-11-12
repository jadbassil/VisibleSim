#include "polygon.hpp"
#include <algorithm>
#include <cmath>
#include <vector>

Polygon::Polygon(const std::vector<Cell3DPosition>& vertices)
    : vertices(vertices) {}

vector<Cell3DPosition>& Polygon::getVertices() {
    sortVerticesClockwise();
    return vertices;
}

void Polygon::addVertex(const Cell3DPosition& vertex) {
    vertices.push_back(vertex);
}

void Polygon::removeVertex(const Cell3DPosition& vertex) {
    auto it = std::find(vertices.begin(), vertices.end(), vertex);
    if (it != vertices.end()) {
        vertices.erase(it);
    }
}

double Polygon::area() const {
    double area = 0;
    std::vector<Cell3DPosition> vertexVector(vertices.begin(), vertices.end());
    for (size_t i = 0; i < vertexVector.size(); ++i) {
        const auto& p1 = vertexVector[i];
        const auto& p2 = vertexVector[(i + 1) % vertexVector.size()];
        area += p1.pt[0] * p2.pt[1] - p2.pt[0] * p1.pt[1];
    }
    return std::abs(area) / 2;
}

double Polygon::perimeter() const {
    double perimeter = 0;
    std::vector<Cell3DPosition> vertexVector(vertices.begin(), vertices.end());
    for (size_t i = 0; i < vertexVector.size(); ++i) {
        const auto& p1 = vertexVector[i];
        const auto& p2 = vertexVector[(i + 1) % vertexVector.size()];
        perimeter += std::hypot(p1.pt[0] - p2.pt[0], p1.pt[1] - p2.pt[1]);
    }
    return perimeter;
}


double Polygon::distance(const Cell3DPosition& p1, const Cell3DPosition& p2) const {
    return std::hypot(p1.pt[0] - p2.pt[0], p1.pt[1] - p2.pt[1]);
}

bool Polygon::compareClockwise(const Cell3DPosition& p1, const Cell3DPosition& p2) const {
    double angle1 = std::atan2(p1.pt[1] - centroid.pt[1], p1.pt[0] - centroid.pt[0]);
    double angle2 = std::atan2(p2.pt[1] - centroid.pt[1], p2.pt[0] - centroid.pt[0]);
    return angle1 > angle2;
}

Cell3DPosition Polygon::calculateCentroid() const {
    double x = 0, y = 0;
    for (const auto& vertex : vertices) {
        x += vertex.pt[0];
        y += vertex.pt[1];
    }
    x /= vertices.size();
    y /= vertices.size();
    return Cell3DPosition(x, y, 0); // Assuming 2D polygon
}

void Polygon::sortVerticesClockwise() {
    centroid = calculateCentroid();
    std::sort(vertices.begin(), vertices.end(), [this](const Cell3DPosition& p1, const Cell3DPosition& p2) {
        return compareClockwise(p1, p2);
    });
}

std::vector<Cell3DPosition> Polygon::convexHull() const {
    if (vertices.size() < 3) return vertices;

    // Find the point with the lowest y-coordinate, break ties by x-coordinate
    auto it = std::min_element(vertices.begin(), vertices.end(), [](const Cell3DPosition& p1, const Cell3DPosition& p2) {
        return (p1.pt[1] < p2.pt[1]) || (p1.pt[1] == p2.pt[1] && p1.pt[0] < p2.pt[0]);
    });
    Cell3DPosition start = *it;
    std::vector<Cell3DPosition> sortedVertices(vertices);
    std::swap(sortedVertices[0], sortedVertices[it - vertices.begin()]);

    // Sort the points by polar angle with the start point
    std::sort(sortedVertices.begin() + 1, sortedVertices.end(), [&start](const Cell3DPosition& p1, const Cell3DPosition& p2) {
        double angle1 = std::atan2(p1.pt[1] - start.pt[1], p1.pt[0] - start.pt[0]);
        double angle2 = std::atan2(p2.pt[1] - start.pt[1], p2.pt[0] - start.pt[0]);
        return angle1 < angle2;
    });

    // Build the convex hull using Graham's scan algorithm
    std::vector<Cell3DPosition> hull;
    hull.push_back(sortedVertices[0]);
    hull.push_back(sortedVertices[1]);

    for (size_t i = 2; i < sortedVertices.size(); ++i) {
        while (hull.size() > 1) {
            const Cell3DPosition& p1 = hull[hull.size() - 2];
            const Cell3DPosition& p2 = hull[hull.size() - 1];
            const Cell3DPosition& p3 = sortedVertices[i];
            double crossProduct = (p2.pt[0] - p1.pt[0]) * (p3.pt[1] - p2.pt[1]) - (p2.pt[1] - p1.pt[1]) * (p3.pt[0] - p2.pt[0]);
            if (crossProduct > 0) break;
            hull.pop_back();
        }
        hull.push_back(sortedVertices[i]);
    }

    return hull;
}