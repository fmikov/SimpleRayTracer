#define _CRT_SECURE_NO_WARNINGS
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"


Vec3f cast_ray(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, const std::vector<Light>& lights);


void render(std::vector<Sphere> spheres, std::vector<Light> lights) {
    const int width = 1024;
    const int height = 768;
    const int channels = 3;
    const int byte_stride = width;
    const Vec3f origin = Vec3f(0.0, 0.0, 0.0);
    const float screen_cam_dist = 1.0;
    const float fov_angle = 90.;
    const float aspect = width / (float)height;

    std::vector<Vec3uc> framebuffer(width * height);
    //unsigned char* test = new unsigned char[width * height * channels];

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            //https://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-generating-camera-rays/generating-camera-rays.html
            float screen_width = tan(fov_angle / 2.) * screen_cam_dist;
            float rx = (2*(i + 0.5) / width -1) * screen_width * aspect;
            float ry = (2*(j + 0.5) / height -1) * screen_width;
            float rz = -1.; //Camera is looking in negative Z direction
            Vec3f rd = Vec3f(rx, ry, rz).normalize();

            Vec3f color = cast_ray(origin, rd, spheres, lights);
            framebuffer[i + j * width] = Vec3uc(int(color.x * 255), int(color.y * 255), int(color.z * 255));
            //test[(i + j * width) * 3] = r;
            //test[(i + j * width) * 3 + 1] = g;
            //test[(i + j * width) * 3 + 2] = b;
        }
    }

    stbi_write_bmp("output.bmp", width, height, channels, framebuffer.data());

    //delete(test);

    std::cout << "Done" << std::endl;

}

int main() {
    Sphere sphere1 = Sphere(Vec3f(0., 0., -4.0), 3.0, Vec3f(1.0, 0.5, 0.5));
    std::vector<Sphere> spheres = { sphere1 };
    Light light1 = Light(Vec3f( - 2., 3., -2.), 1.0);
    std::vector<Light> lights = { light1 };
    render(spheres, lights);
    return 0;
}


Vec3f cast_ray(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, const std::vector<Light>& lights) {
    float t0 = std::numeric_limits<float>::max();
    for (Sphere sphere : spheres) {
        if (sphere.ray_intersect(ro, rd, t0)) {
            float diffuse_light_intensity = 0.f;
            for (Light light : lights) {
                Vec3f light_dir = (light.position - (ro + rd*t0)).normalize();
                diffuse_light_intensity += light.intensity * std::max(0.f, light_dir * rd);
            }
            return sphere.color * diffuse_light_intensity;
        }

    }
    return Vec3f(0.2, 0.7, 0.8);
}