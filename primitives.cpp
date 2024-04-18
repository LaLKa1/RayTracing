#include "./Vector.cpp"
#include <SFML/Graphics.hpp>
#include "collision.cpp"
#include "scene.cpp"

template <typename T>
T pow2(T a){ return a * a; }

class Sphere{
public:
    F3Vector center;
    float radius;
    sf::Color color;
    Sphere(F3Vector center, float radius, sf::Color color){
        this->center = center;
        this->radius = radius;
        this->color = color;
    }

    Collision intersection(F3Vector origin, F3Vector vector){
        F3Vector delta_coord = origin - center;
        float dot = vector.dot(delta_coord);
        if (dot > 0){
            Collision coll;
            coll.is_collided = false;
            return  coll;
        }
        double delta = dot * dot - (pow2(delta_coord.norm()) - radius * radius);
        Collision collision;
        collision.color = this->color;
        collision.is_collided = true;
        if (delta < 0) collision.is_collided = false;
        else{
            double a = -dot + sqrt(delta);
            double b = -dot - sqrt(delta);
            if (a > 0 && b > 0) collision.distance = std::min(a, b);
            else if (a > 0) collision.distance = static_cast<float>(a);
            else if (b > 0) collision.distance = static_cast<float>(a);
            else collision.is_collided = false;
            collision.norm = delta_coord + vector * collision.distance;
            collision.norm = collision.norm / collision.norm.norm();
        }
        return collision;
    }

    std::pair<F3Vector, float> get_bounds() { return {this->center, this->radius}; }
};

class Plane{
public:
    F3Vector point;
    F3Vector normal;
    sf::Color color;
    Plane(F3Vector point, F3Vector normal, sf::Color color){
        this->point = point;
        this->normal = normal / normal.norm();
        this->color = color;
    };
    Collision intersection(F3Vector origin, F3Vector vector){
        Collision coll;
        if (vector.dot(this->normal) == 0){
            coll.is_collided = false;
            return coll;
        }
        float distance = (this->point - origin).dot(this->normal) / vector.dot(this->normal);
        if (distance > 0){
            coll.distance = distance;
            coll.color = this->color;
            coll.is_collided = true;
            coll.norm = (vector.dot(normal) < 0) ?  this->normal : this->normal * -1.f;
        } else {
            coll.is_collided = false;
        }
        return coll;
    }

    std::pair<F3Vector, float> get_bounds(){
        return {F3Vector(), INFINITY};
    }
};

class SphericVolume{
public:
    F3Vector center;
    float radius;
    Scene scene;

    SphericVolume(F3Vector center, float radius, const Scene& scene){
        this->center = center;
        this->scene = scene;
        this->radius = radius;
    }

    bool intersection(F3Vector origin, F3Vector vector){
        F3Vector delta_coord = origin - center;
        float dot = vector.dot(delta_coord);
        if (dot > 0) return false;
        double delta = dot * dot - (pow2(delta_coord.norm()) - radius * radius);
        return delta > 0;
    }


    void add_obj(void* ptr, Scene::type type, std::pair<F3Vector, float> bound){
        F3Vector diff = {bound.first.x + bound.second - center.x + radius,
                         bound.first.y + bound.second - center.y + radius,
                         bound.first.z + bound.second - center.z + radius};
        if (diff.x > 0) {
            radius += diff.x / 2;
            center.x += diff.x / 2;
        }
    }

    std::pair<F3Vector, float> get_bounds() { return {center, radius}; }
};
