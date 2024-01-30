#define _CRT_SECURE_NO_WARNINGS
#include <limits>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include "geometry.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void render(Sphere sphere) {
    const int width = 1024;
    const int height = 768;
    const int channels = 3;
    const int byte_stride = width;
    const Vec3f origin = Vec3f(0.0, 0.0, 0.0);
    const float screen_cam_dist = 1.0;
    const float fov_angle = 90.;
    const float aspect = width / height;

    std::vector<Vec3uc> framebuffer(width * height);
    //unsigned char* test = new unsigned char[width * height * channels];

    for (size_t j = 0; j < height; j++) {
        for (size_t i = 0; i < width; i++) {
            int r = int(j / float(height) * 255);
            int g = int(i / float(width) * 255);
            int b = 0;
            framebuffer[i + j * width] = Vec3uc(r, g, b);

            float screen_width = tan(fov / 2.) * 2 * screen_cam_dist;
            float rx = (i + 0.5) / width * screen_width * aspect;
            float ry = (j + 0.5) / height * screen_width;
            float rz = -1.; //Camera is looking in negative Z direction
            //test[(i + j * width) * 3] = r;
            //test[(i + j * width) * 3 + 1] = g;
            //test[(i + j * width) * 3 + 2] = b;
        }
    }

    stbi_write_bmp("output.bmp", width, height, channels, framebuffer.data());

    delete(test);

    std::cout << "Done" << std::endl;

}

int main() {
    Sphere sphere = Sphere(Vec3f(0.8, 0.3, -4.0), 4.0);
    render(sphere);
    return 0;
}

void ray_loop(Vec3f origin, float screen_dist, float fov) {

}

Vec3f cast_ray(Vec3f ro, Vec3f rd) {

}