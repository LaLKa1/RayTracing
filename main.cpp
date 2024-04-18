#include <SFML/Graphics.hpp>
#include "./Vector.cpp"
#include "collision.cpp"
#include "primitives.cpp"
#include "scene.cpp"

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>

#define WIDTH 1500
#define HEIGHT 1000
#define PI 3.1415927f
float FOV = PI/2;
const float fly_speed = 1;
float fly_velocity = fly_speed;
bool window_focus = true;
bool running = true;
int reflection_count = 4;

int processed_rows = 1000;
std::mutex m_main, m_subprocess;
std::condition_variable cv_main, cv_subprocess;
int thread_count = std::max(1u, std::thread::hardware_concurrency());
std::vector<std::thread> threads;


F3Vector rotate_z(F3Vector vector, float angle, F3Vector point = {0, 0, 0}, float sin_a = NAN, float cos_a = NAN){
    if (std::isnan(sin_a)) {
        sin_a = sin(angle);
        cos_a = cos(angle);
    }
    F3Vector d = vector - point;
    F3Vector res = {
        d.x * cos_a + d.y * sin_a,
        -d.x * sin_a + d.y * cos_a,
        d.z
    };
    return res + point;
}

F3Vector rotate_y(F3Vector vector, float angle, F3Vector point = {0, 0, 0}, float sin_a = NAN, float cos_a = NAN){
    if (std::isnan(sin_a)){
        sin_a = sin(angle);
        cos_a = cos(angle);
    }
    F3Vector d = vector - point;
    F3Vector res = {
        d.x * cos_a - d.z * sin_a,
        d.y,
        d.x * sin_a + d.z * cos_a
    };
    return res + point;
}


F3Vector origin = F3Vector{0, 0, 0};
F3Vector rotation = F3Vector{1, 0, 0};
float foc_l = 1.f;
float angle_z = 0;  // left-right
float angle_y = 0;  // up-down

constexpr int scale = 2;
sf::VertexArray buffer{sf::PrimitiveType::Points, WIDTH*HEIGHT};
Scene current_scene = Scene();


void build_BVH(Scene& scene, int merge_count = 2){
    struct object{
        std::pair<F3Vector, float> bound;
        Scene::type type;
        void* ptr;
    };

    std::vector<object> objects;
    for (auto & i : scene.objects) {
        object o;
        o.type = i.type_;
        o.ptr = i.ptr;
        switch (i.type_){
            case (Scene::type::Sphere):{
                o.bound = ((Sphere*)i.ptr)->get_bounds();
                break;
            }
            case (Scene::type::Plane):{
                o.bound = ((Plane*)i.ptr)->get_bounds();
                break;
            }
            case (Scene::type::SphericVolume):{
                o.bound = ((SphericVolume*)i.ptr)->get_bounds();
                break;
            }
        }
        objects.push_back(o);
    }
    std::vector<object> current;
    Scene new_scene;
    for (auto & obj : objects){
        if (obj.bound.second == INFINITY)
            new_scene.add_object(obj.ptr, obj.type);
         else{
             current.push_back(obj);
             if (current.size() == merge_count){
                 SphericVolume subvolume;

             }
         }
    }
}


/// @brief calculates single pixel from origin in vector direction
/// @param vector direction of ray (unit vector)
/// @param origin start point of ray
/// @return Closest collision
Collision calc(F3Vector vector, F3Vector origin, const Scene& scene, int depth = 0){
        vector = vector / vector.norm();
        float distance = INFINITY;
        Collision closest;
        closest.color = sf::Color::Black;
        closest.is_collided = false;
        for (int i = 0; i < scene.objects_count; i++){
            Scene::object object = scene.objects[i];
            Collision coll;
            switch (object.type_){
                case Scene::type::Sphere:{
                    Sphere obj = *(Sphere*)object.ptr;
                    coll = obj.intersection(origin, vector);
                    break;
                }
                case Scene::type::Plane:{
                    Plane obj = *(Plane*)object.ptr;
                    coll = obj.intersection(origin, vector);
                    break;
                }
                case Scene::type::SphericVolume:{
                    SphericVolume& obj = *(SphericVolume*)object.ptr;
                    if (obj.intersection(origin, vector)){
                        coll = calc(vector, origin, obj.scene, depth);
                    } else coll.is_collided = false;
                    break;
                }
                default:
                    break;
            }
            if (coll.is_collided && coll.distance < distance){
                closest = coll;
                distance = coll.distance;
            };
        }
        if (closest.is_collided && depth < reflection_count) {
            F3Vector point = origin + vector * distance;
            F3Vector vector_ = vector - closest.norm * closest.norm.dot(vector) * 2;
            Collision next = calc(vector_, point + closest.norm * 0.01f, scene,  ++depth);
            if (next.is_collided){
                closest.color *= next.color;
            }
        }
        // float alpha = 255 / sqrt(1 + pow(distance, 2));
        float alpha = 255 / (1 + distance*distance/32);
        sf::Color far = sf::Color(alpha, alpha, alpha);
        // if (distance < 0) closest.color = sf::Color::Red;
        closest.color = closest.color * far;
        return closest;
}


/// @brief calls every frame (single thread)
void old_update(){
    float ratio = (float)WIDTH / HEIGHT;
    float scr_width = 2 * foc_l * tan(FOV/2);
    float scr_height = scr_width / ratio;
    F3Vector scr_center = origin + F3Vector{1, 0, 0} * foc_l;

    for (float y = -scr_height/2, yi=0; yi < HEIGHT-scale;  y+=scr_height*scale/(HEIGHT-1), yi += scale){
        for (float x = -scr_width/2, xi=0; xi < WIDTH - scale;  x+=scr_width*scale/(WIDTH-1), xi += scale){
            sf::Vertex point{sf::Vector2f(xi, yi)};
            F3Vector dot = scr_center + F3Vector(0, x, y); //point on 'screen'
            dot = rotate_y(dot, angle_y, origin);
            dot = rotate_z(dot, angle_z, origin);
            F3Vector vector = (dot - origin);
            Collision collision = calc(vector, origin, current_scene);
            point.color = collision.color;
            for (int _y = 0; _y < scale; _y++)
                for (int _x = 0; _x < scale; _x++){
                    point.position = sf::Vector2f(xi + static_cast<float>(_x), yi + static_cast<float>(yi));
                    buffer[(yi + _y)*WIDTH + xi + _x] = point;
                }
        }
    }
}

/// @brief calls every frame (multi thread)
void update(){
    processed_rows = 0;
    cv_subprocess.notify_all();
    std::unique_lock<std::mutex> lock(m_main);
    cv_main.wait(lock, [&]{return processed_rows == (HEIGHT-scale);});
}


void subprocess(){
    float row = 0.f;
    while(true){
        std::unique_lock<std::mutex> sub_lock(m_subprocess);
        cv_subprocess.wait(sub_lock, [&]{return processed_rows < (HEIGHT - scale);});
        if(!running) return;
        float ratio = (float)WIDTH / HEIGHT;
        float scr_width = 2 * foc_l * tan(FOV/2);
        float scr_height = scr_width / ratio;
        F3Vector scr_center = origin + F3Vector{1, 0, 0} * foc_l;
        while(true){
            if(!sub_lock.owns_lock()) sub_lock.lock();
            if (processed_rows < (HEIGHT-scale)){
                row = (float)processed_rows++;
            } else {
                cv_main.notify_all();
                sub_lock.unlock();
                break;
            }
            sub_lock.unlock();
            float yi = row;
            auto sin_z = static_cast<float>(sin(angle_z));
            auto cos_z = static_cast<float>(cos(angle_z));
            auto sin_y = static_cast<float>(sin(angle_y));
            auto cos_y = static_cast<float>(cos(angle_y));
            float y = -scr_height/2 + row*(scr_height/HEIGHT);
            for (float x = -scr_width/2, xi=0; xi < WIDTH - scale;  x+=scr_width*scale/(WIDTH-1), xi += scale){
                sf::Vertex point{sf::Vector2f(xi, yi)};
                F3Vector dot = scr_center + F3Vector(0, x, y); //point on 'screen
                dot = rotate_y(dot, angle_y, origin, sin_y, cos_y);
                dot = rotate_z(dot, angle_z, origin, sin_z, cos_z);
                F3Vector vector = (dot - origin);
                Collision collision = calc(vector, origin, current_scene);
                point.color = collision.color;
                for (int _y = 0; _y < scale; _y++)
                for (int _x = 0; _x < scale; _x++){
                    point.position = sf::Vector2f(xi + _x, row + _y);
                    buffer[(yi + _y)*WIDTH + xi + _x] = point;
                }
            }
        }
    }
}


void draw(sf::RenderWindow& window){
    window.draw(buffer);
}

void clear_buffer(sf::Color color = sf::Color::Black){
    for (size_t y = 0; y < HEIGHT; y++)
        for (size_t x = 0; x < WIDTH; x++)
            buffer[y*HEIGHT + x] = sf::Vertex(sf::Vector2f(x, y), color);
}

int main(){
    float FPS = 30;

    for (int i = 0; i < thread_count; i++){
        threads.emplace_back(subprocess);
    }
    
    static sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "New Ray$hit");
    window.setFramerateLimit(static_cast<int>(FPS));
    
    current_scene.add_object(
            new Sphere({5, 2, 0}, 1.5f, {255, 100, 100}),
            Scene::Sphere);
    current_scene.add_object(
            new Sphere({5, -2, 0}, 1.5f, {100, 255, 100}),
            Scene::Sphere);
    current_scene.add_object(
            new Sphere({5, 0, 3}, 1.5f, {100, 100, 255}),
            Scene::Sphere);
    current_scene.add_object(
            new Plane({0, 0, 5}, {0, 0, 1}, sf::Color::White),
            Scene::Plane);

    while(window.isOpen()){
        sf::Event event;
        while (window.pollEvent(event)){
            switch (event.type){
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::LostFocus:
                window_focus = false;
                break;
            case sf::Event::GainedFocus:
                window_focus = true;
                break;
            default:
                break;
            }
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
            origin = origin + rotation * (fly_velocity/FPS) / rotation.norm();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
            origin = origin - rotation * (fly_velocity/FPS) / rotation.norm();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
            origin = origin + rotate_z(rotation, PI/2)*fly_velocity / (rotation.norm() * FPS);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
            origin = origin + rotate_z(rotation, -PI/2)*fly_velocity / (rotation.norm() * FPS);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::E)){
            origin.z -= fly_velocity / FPS;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)){
            origin.z += fly_velocity / FPS;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)){
            fly_velocity = fly_speed * 3;
        } else {
            fly_velocity = fly_speed;
        }

        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
            angle_z += PI / FPS; // 5 degrees
            rotation = rotate_z({1, 0, 0}, angle_z);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
            angle_z -= PI / FPS; // 5 degrees
            rotation = rotate_z({1, 0, 0}, angle_z);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
            angle_y -= PI / 36; // 5 degrees
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
            angle_y += PI / 36; // 5 degrees
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num2)) {
            // scale = scale*2 > 30 ? 30 : scale*2;
            // clear_buffer();
        };

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Num1)) {
            // scale = scale/2 < 1 ? 1 : scale/2;
            // clear_buffer();
        };
        update();
        draw(window);
        window.display();
    }
    running = false;
    processed_rows = 0;
    cv_subprocess.notify_all();
    for (int i = 0; i < thread_count; i++){
        threads[i].join();
    }
    return 0;
}
