#ifndef VECTORRRR
#define VECTORRRR

#include "Vector.h"
#include <cmath>

F3Vector::F3Vector(float x, float y, float z){
    this->x = x;    
    this->y = y;
    this->z = z;
};

F3Vector::F3Vector() = default;

F3Vector F3Vector::operator+(const F3Vector& other){
    return F3Vector(this->x + other.x, this->y + other.y, this->z + other.z);
};

F3Vector F3Vector::operator-(const F3Vector& other){
    return F3Vector(this->x - other.x, this->y - other.y, this->z - other.z);
};

F3Vector F3Vector::operator*(const F3Vector& other){
    return F3Vector(this->x * other.x, this->y * other.y, this->z * other.z);
};

F3Vector F3Vector::operator*(float k){
    return F3Vector(this->x * k, this->y * k, this->z * k);
};

F3Vector F3Vector::operator/(float k){
    return F3Vector(this->x / k, this->y / k, this->z / k);
};

float F3Vector::dot(F3Vector other){
//    F3Vector mul = *this * other;
    return this->x * other.x + this->y * other.y + this->z * other.z;
}

float F3Vector::norm(){
    return sqrt(x*x + y*y + z*z);
    // return sqrt(this->dot(*this));
}

float F3Vector::sum() const {return x + y + z;}

bool F3Vector::operator==(const F3Vector &other) const{
    return this->x == other.x && this->y == other.y && this->z == other.z;
}

#endif