#include <vector>
#ifndef SCENE
#define SCENE

struct Scene{
    enum type {Sphere, Plane, SphericVolume};
    struct object{
        void* ptr;
        type type_;
    };
    int objects_count = 0;
    std::vector<object> objects;

    template <typename T>
    void add_object(T* object_p, Scene::type type) {
        objects.push_back({(void*)(object_p), type});
        objects_count++;
    }
};

#endif