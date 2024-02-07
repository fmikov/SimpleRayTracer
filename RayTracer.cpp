#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES

#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"
#include "objects.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const float AMBIENT_INTENSITY = 1.0f;

Vec3f cast_ray(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, const std::vector<Light>& lights, int depth);

Vec3f reflect(const Vec3f& I, const Vec3f& N) {
    return I - N * 2.f * (I * N);
}


void render(std::vector<Sphere> spheres, std::vector<Light> lights) {
    const int width = 1024;
    const int height = 768;
    const int channels = 3;
    const int byte_stride = width;
    const Vec3f origin = Vec3f(0.0, 0.0, 0.0);
    const float screen_cam_dist = 1.0f;
    const float fov = 90.f * M_PI / 180.f; //in radians
    const float aspect = width / (float)height;

    std::vector<Vec3uc> framebuffer(width * height);

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays.html
            float screen_width = tan(fov / 2.f) * screen_cam_dist;
            float rx = (2*(i + 0.5f) / (float)width -1) * screen_width * aspect;
            float ry = (1 - 2*(j + 0.5f) / (float)height) * screen_width;
            Vec3f rd = Vec3f(rx, ry, -1.f).normalize();

            Vec3f color = cast_ray(origin, rd, spheres, lights, 0);
            framebuffer[i + j * width] = Vec3uc(
                int(std::min(1.f, color.x) * 255), 
                int(std::min(1.f, color.y) * 255), 
                int(std::min(1.f, color.z) * 255));
        }
    }
    stbi_write_bmp("output.bmp", width, height, channels, framebuffer.data());

    std::cout << "Done" << std::endl;

}

int main() {
    Material      ivory(Vec3f(0.4, 0.4, 0.3));
    Material red_rubber(Vec3f(0.3, 0.1, 0.1));

    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, red_rubber));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, ivory));



    std::vector<Light>  lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));
    render(spheres, lights);
    return 0;
}

bool scene_intersect(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, int& index, float& t0) {
    float intersection = std::numeric_limits<float>::max(), closest_intersection = std::numeric_limits<float>::max();
    int ind = -1;
    for (int i = 0; i < spheres.size(); i++) {
        if (spheres[i].ray_intersect(ro, rd, intersection)) {
            if (intersection < closest_intersection) {
                ind = i;
                closest_intersection = intersection;
            }
        }
    }
    if (ind < 0) {
        return false;
    }
    index = ind;
    t0 = closest_intersection;
    return true;
}

Vec3f cast_ray(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, const std::vector<Light>& lights, int depth) {
    int ind;
    float t0;
    Vec3f final_color(0., 0., 0.);

    if (depth > 4 || !scene_intersect(ro, rd, spheres, ind, t0)) {
        return Vec3f(0.2, 0.7, 0.8);
    }

    Material mat = spheres[ind].material;

    Vec3f hit = ro + rd * t0;
    Vec3f normal = (hit - spheres[ind].center).normalize();


    Vec3f reflect_dir = reflect(hit.normalize(), normal).normalize();
    Vec3f reflect_orig = reflect_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

    float diffuse_light_intensity = 0.f;
    float specular_light_intensity = 0.f;
    
    //Check for shadows
    int ind_sh;
    float t0_sh;

    for (Light light : lights) {
        Vec3f to_light = (light.position - hit).normalize();
        //Check for shadow for current light
        Vec3f shadow_orig = hit + normal * 1e-3;
        float light_distance = (light.position - hit).norm();
        if (scene_intersect(shadow_orig, to_light, spheres, ind_sh, t0_sh) && (to_light*t0_sh).norm() < light_distance) continue;

        diffuse_light_intensity += light.intensity * std::max(0.f, to_light * normal);

        Vec3f half_way = (to_light -hit.normalize()).normalize();
        specular_light_intensity += powf(std::max(0.f, normal * half_way), mat.shininess) * light.intensity;
    }
    Vec3f diffuse = mat.color * mat.diffuse * diffuse_light_intensity;
    Vec3f specular = mat.color * mat.specular * specular_light_intensity;
    Vec3f ambient = mat.color * mat.ambient * AMBIENT_INTENSITY;
    Vec3f reflected = reflect_color * mat.reflectance;

    final_color = diffuse + specular + reflected;
    return final_color;
}