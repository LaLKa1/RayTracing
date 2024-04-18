#pragma once
#include "./Vector.cpp"
#include <SFML/Graphics.hpp>

struct Collision {
    bool is_collided;
    float distance;
    F3Vector norm;
    sf::Color color;
};
