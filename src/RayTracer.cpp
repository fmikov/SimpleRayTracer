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
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

const float AMBIENT_INTENSITY = 1.0f;
int envmap_width, envmap_height;
std::vector<Vec3f> envmap;

Vec3f cast_ray(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, const std::vector<Light>& lights, int depth);

float clamp(float n, float lower, float upper) {
    return std::max(lower, std::min(n, upper));
}

Vec3f reflect(const Vec3f& I, const Vec3f& N) {
    return I - N * 2.f * (I * N);
}

Vec3f refract(const Vec3f& I, const Vec3f& N, const float& ior)
{
    float cosi = clamp(I*N, -1, 1);
    float etai = 1, etat = ior;
    Vec3f n = N;
    if (cosi < 0) { cosi = -cosi; }
    else { std::swap(etai, etat); n = -N; }
    float eta = etai / etat;
    //k is used to check if critical angle of refraction, where we only have a reflection
    float k = 1 - eta * eta * (1 - cosi * cosi);
    return k < 0 ? Vec3f(0, 0, 0) : I * eta + (eta * cosi - sqrtf(k)) * n;
}

void fresnel(const Vec3f& I, const Vec3f& N, const float& ior, float& kr)
{
    float cosi = clamp(-1, 1, I*N);
    float etai = 1, etat = ior;
    if (cosi > 0) { std::swap(etai, etat); }
    // Compute sini using Snell's law
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
    // Total internal reflection
    if (sint >= 1) {
        kr = 1;
    }
    else {
        float cost = sqrtf(std::max(0.f, 1 - sint * sint));
        cosi = fabsf(cosi);
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
        kr = (Rs * Rs + Rp * Rp) / 2;
    }
    // As a consequence of the conservation of energy, the transmittance is given by:
    // kt = 1 - kr;
}



void render(std::vector<Sphere> spheres, std::vector<Light> lights) {
    const int width = 1024;
    const int height = 768;
    const Vec3f origin = Vec3f(0.0, 0.0, 0.0);
    const float screen_cam_dist = 1.0f;
    const float fov = 60.f * M_PI / 180.f; //in radians
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
    stbi_write_jpg("output.jpg", width, height, 3, framebuffer.data(), 100);
}

Vec3f sample_envmap(const Vec3f& ro, const Vec3f& rd) {
    // Convert direction vector to spherical coordinates
    float theta = acos(rd.y);  // Inclination angle, theta = arccos(cos(y_axis * rd.y)) by def of dot product
    float phi = atan2(rd.z, rd.x);  // Angle from the x axis, counterclockwise

    // Map spherical coordinates to pixel coordinates
    int u = (phi + M_PI) / (2 * M_PI) * envmap_width;
    int v = theta / M_PI * envmap_height;

    // Clamp pixel coordinates to valid range
    u = std::max(0, std::min(u, envmap_width - 1));
    v = std::max(0, std::min(v, envmap_height - 1));

    return envmap[u + v * envmap_width];
}


bool scene_intersect(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, Material& mat, float& t0, Vec3f& normal) {
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


    //Check intersection with plane
    if (fabs(rd.y) > 1e-3) {
        float dist = -(ro.y + 4) / rd.y; // the checkerboard plane has equation y = -4
        Vec3f pt = ro + rd * dist;
        if (dist > 0 && fabs(pt.x) < 10 && pt.z<-10 && pt.z>-30 && dist < closest_intersection) {
            t0 = dist;
            normal = Vec3f(0., 1., 0.);
            mat.color = (int(.5 * pt.x + 1000) + int(.5 * pt.z)) & 1 ? Vec3f(.3, .3, .3) : Vec3f(.3, .2, .1);
            return true;
        }
    }
    if (ind < 0) return false;
    mat = spheres[ind].material;
    t0 = closest_intersection;
    normal = (ro + rd * t0 - spheres[ind].center).normalize();

    return true;
}

Vec3f cast_ray(const Vec3f& ro, const Vec3f& rd, const std::vector<Sphere>& spheres, const std::vector<Light>& lights, int depth) {
    Material mat;
    float t0;
    Vec3f normal;
    Vec3f final_color(0., 0., 0.);

    if (depth > 4 || !scene_intersect(ro, rd, spheres, mat, t0, normal)) {
        return sample_envmap(ro, rd);
    }


    Vec3f hit = ro + rd * t0;

    float diffuse_light_intensity = 0.f;
    float specular_light_intensity = 0.f;
    
    //Check for shadows
    Material mat_sh;
    float t0_sh;
    Vec3f normal_sh;


    for (Light light : lights) {
        Vec3f to_light = (light.position - hit).normalize();
        //Check for shadow for current light
        Vec3f shadow_orig = hit + normal * 1e-3;
        float light_distance = (light.position - hit).norm();
        if (scene_intersect(shadow_orig, to_light, spheres, mat_sh, t0_sh, normal_sh) && (to_light*t0_sh).norm() < light_distance)
            continue;

        diffuse_light_intensity += light.intensity * std::max(0.f, to_light * normal);

        Vec3f half_way = (to_light -rd).normalize();
        specular_light_intensity += powf(std::max(0.f, normal * half_way), mat.shininess) * light.intensity;
    }

    Vec3f reflect_dir = reflect(rd, normal).normalize();
    Vec3f reflect_orig = reflect_dir * normal < 0 ? hit - normal * 1e-3 : hit + normal * 1e-3; // offset the original point to avoid occlusion by the object itself
    Vec3f reflect_color = cast_ray(reflect_orig, reflect_dir, spheres, lights, depth + 1);

    Vec3f refract_color;
    if (mat.refractivity > 0.0) {
        Vec3f refracted = refract(rd, normal, 1.333);
        if (!(refracted.x == 0.0 && refracted.y == 0.0 && refracted.z == 0.0)) {
            Vec3f refract_dir = refracted.normalize();
            // Similar to reflect orig but opposite, since we want to go through the object
            Vec3f refract_orig = reflect_dir * normal < 0 ? hit + normal * 1e-3 : hit - normal * 1e-3;
            refract_color = cast_ray(refract_orig, refract_dir, spheres, lights, depth + 1);
        }
    }


    Vec3f diffuse = mat.color * mat.diffuse * diffuse_light_intensity;
    Vec3f specular = mat.color * mat.specular * specular_light_intensity;
    Vec3f ambient = mat.color * mat.ambient * AMBIENT_INTENSITY;
    float kr = 1.0;
    if (mat.refractivity > 0. && mat.reflectivity > 0.)
        fresnel(rd, normal, mat.ior, kr);
    Vec3f reflected = reflect_color * mat.reflectivity;
    Vec3f refracted = refract_color * mat.refractivity;
    
    final_color = diffuse + specular + reflected*kr + refracted*(1-kr);
    return final_color;
}


int main() {

    int n = -1;
    unsigned char* pixmap = stbi_load("envmap.jpg", &envmap_width, &envmap_height, &n, 0);
    if (!pixmap || 3 != n) {
        std::cerr << "Error: can not load the environment map" << std::endl;
        return -1;
    }
    envmap = std::vector<Vec3f>(envmap_width * envmap_height);
    for (int j = envmap_height - 1; j >= 0; j--) {
        for (int i = 0; i < envmap_width; i++) {
            envmap[i + j * envmap_width] = Vec3f(pixmap[(i + j * envmap_width) * 3 + 0], pixmap[(i + j * envmap_width) * 3 + 1], pixmap[(i + j * envmap_width) * 3 + 2]) * (1 / 255.);
        }
    }
    stbi_image_free(pixmap);

    Material      ivory(Vec3f(0.4, 0.4, 0.3), 0.6, 0.3, 50., 0.1, 0.0, 1.0);
    Material red_rubber(Vec3f(0.3, 0.1, 0.1), 0.9, 0.1, 10., 0.0, 0.0, 1.0);
    Material     mirror(Vec3f(1.0, 1.0, 1.0), 0.0, 10.0, 1425., 0.8, 0.0, 1.0);
    Material      glass(Vec3f(0.6, 0.7, 0.8), 0.0, 0.5, 125., 0.1, 0.8, 1.5);


    std::vector<Sphere> spheres;
    spheres.push_back(Sphere(Vec3f(-3, 0, -16), 2, ivory));
    spheres.push_back(Sphere(Vec3f(-1.0, -1.5, -12), 2, glass));
    spheres.push_back(Sphere(Vec3f(1.5, -0.5, -18), 3, red_rubber));
    spheres.push_back(Sphere(Vec3f(7, 5, -18), 4, mirror));



    std::vector<Light>  lights;
    lights.push_back(Light(Vec3f(-20, 20, 20), 1.5));
    lights.push_back(Light(Vec3f(30, 50, -25), 1.8));
    lights.push_back(Light(Vec3f(30, 20, 30), 1.7));
    render(spheres, lights);

    std::cout << "Done" << std::endl;
    return 0;
}