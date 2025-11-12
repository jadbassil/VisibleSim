#ifndef INEQUALITY2D_HPP
#define INEQUALITY2D_HPP
#include <functional>
#include <iostream>

class Inequality2D {
   public:
    // Constructor to initialize the coefficients, the constant term, and the comparison operator
    Inequality2D(short _a, short _b, short _c = 0,
                 std::function<bool(short, short)> comp = std::less_equal<short>())
        : a(_a), b(_b), c(_c), comp(comp) {}

    // Method to evaluate the inequality for a given point (x, y)
    bool evaluate(short x, short y) const { return comp(a * x + b * y + c, 0); }

    // x >= 2 becomes x - 2 >= 0 => a = 1, b=0, c=-2

    // Method to check if this inequality is fully included in the interval of another inequality
    bool isIncludedIn(const Inequality2D& other) const {
        // Check if the coefficients and constant term of this inequality are included in the other
        // inequality
        return (a == other.a && b == other.b && c <= other.c);
    }

    // Overload the equality operator to compare two Inequality2D objects
    bool operator==(const Inequality2D& other) const {
        return comp.target_type() == other.comp.target_type() &&
            (other.isIncludedIn(*this) || isIncludedIn(other) || (a == other.a && b == other.b && c == other.c));
    }

    // Overload the less-than operator to allow usage of this class in a set
    bool operator<(const Inequality2D& other) const {
        if (a != other.a) return a < other.a;
        if (b != other.b) return b < other.b;
        if (c != other.c) return c < other.c;
        return comp.target_type().hash_code() < other.comp.target_type().hash_code();
    }

    // Overload the stream insertion operator to print the inequality
    friend std::ostream& operator<<(std::ostream& os, const Inequality2D& inequality) {
        if (inequality.a != 0) {
            os << inequality.a << "x ";
        }
        if (inequality.b != 0) {
            if (inequality.b > 0 && inequality.a != 0) {
                os << "+ ";
            }
            os << inequality.b << "y ";
        }
        if (inequality.c != 0) {
            if (inequality.c > 0 && (inequality.a != 0 || inequality.b != 0)) {
                os << "+ ";
            }
            os << inequality.c << " ";
        }
        if (inequality.comp.target_type().hash_code() ==
            typeid(std::less_equal<short>).hash_code()) {
            os << "<= 0";
        } else if(inequality.comp.target_type().hash_code() ==
            typeid(std::greater_equal<short>).hash_code()){
            os << ">= 0";
        } else {
            os << " = 0";
        }
        return os;
    }

    short getA() const { return a; }

    short getB() const { return b; }

    short getC() const { return c; }

    size_t getCompHash() const { return comp.target_type().hash_code(); }

   private:
    short a;                                 // Coefficient for x
    short b;                                 // Coefficient for y
    short c;                                 // Constant term
    std::function<bool(short, short)> comp;  // Comparison operator
};

#endif  // INEQUALITY2D_HPP