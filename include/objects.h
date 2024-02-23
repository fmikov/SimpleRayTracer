#pragma once

#include "geometry.h"

struct Material {
    Material(const Vec3f& color, const float& diffuse, const float& specular, const float& shininess, 
        const float& reflectivity, const float& refractivity, const float& ior)

        : color(color), diffuse(diffuse), specular(specular), shininess(shininess), ambient(0.1), 
        reflectivity(reflectivity), refractivity(refractivity), ior(ior) {}
    Material(const Vec3f& color)
        : color(color), diffuse(0.6f), specular(0.5f), ambient(0.1f), shininess(50.f), reflectivity(0.5f), refractivity(0.5f), ior(1.0f) {}
    Material() 
        : color(Vec3f(1.0f, 1.0f, 1.0f)), diffuse(0.9f), specular(0.1f), shininess(10.f), reflectivity(0.f), refractivity(0.f), ior(1.f) {}
    Vec3f color;
    float diffuse;
    float specular;
    float shininess;
    float ambient;
    float reflectivity;
    float refractivity;
    float ior;
};

struct Sphere {
    Vec3f center;
    float radius;
    Material material;

    Sphere(const Vec3f& c, const float& r, const Material& material) : center(c), radius(r), material(material) {}

    bool ray_intersect(const Vec3f& orig, const Vec3f& dir, float& t0) const {
        Vec3f L = center - orig;
        float tca = L * dir;
        float d2 = L * L - tca * tca;
        if (d2 > radius * radius) return false;
        float thc = sqrtf(radius * radius - d2);
        t0 = tca - thc;
        float t1 = tca + thc;
        if (t0 < 0) t0 = t1;
        if (t0 < 0) return false;
        return true;
    }
};

struct Light {
    Light(const Vec3f& p, const float& i) : position(p), intensity(i) {}
    Vec3f position;
    float intensity;
};

