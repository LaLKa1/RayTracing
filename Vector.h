#pragma once

struct F3Vector{
    float x{}, y{}, z{};
    F3Vector(float x, float y, float z);
    F3Vector();
    F3Vector operator+(const F3Vector& other);
    F3Vector operator-(const F3Vector& other);
    F3Vector operator*(const F3Vector& other);
    F3Vector operator*(float k);
    F3Vector operator/(float k);
    bool operator==(const F3Vector& other) const;
    float dot(F3Vector other);
    float norm();
    float sum() const;
};
