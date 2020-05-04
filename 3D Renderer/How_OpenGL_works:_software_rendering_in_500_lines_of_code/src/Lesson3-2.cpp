#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <iostream>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

Model *model = nullptr;
const int width = 1024;
const int height = 1024;

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i = 2; i--;) {
        s[i][0] = C[i] - A[i];
        s[i][1] = B[i] - A[i];
        s[i][2] = A[i] - P[i];
    }
    Vec3f u = s[0] ^s[1];
    if (std::abs(u[2]) > 1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
    return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, float *zbuffer, TGAImage &image, TGAColor color) {
    Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

    bboxmin = Vec2f(std::min({pts[0].x, pts[1].x, pts[2].x}), std::min({pts[0].y, pts[1].y, pts[2].y}));
    bboxmax = Vec2f(std::max({pts[0].x, pts[1].x, pts[2].x}), std::max({pts[0].y, pts[1].y, pts[2].y}));

    Vec3f P;
    for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++) {
        for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++) {
            Vec3f bc_screen = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x < 0 || bc_screen.y < 0 || bc_screen.z < 0) continue;
            P.z = 0;
            for (int i = 0; i < 3; i++) P.z += pts[i][2] * bc_screen[i];
            if (zbuffer[int(P.x + P.y * width)] < P.z) {

                zbuffer[int(P.x + P.y * width)] = P.z;
                image.set(P.x, P.y, color);
            }
        }
    }
}
void triangle_with_texture(Vec3f t0, Vec3f t1, Vec3f t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image, float intensity,
               int *zbuffer) {
    if (t0.y == t1.y && t0.y == t2.y) return; // i dont care about degenerate triangles
    if (t0.y > t1.y) {
        std::swap(t0, t1);
        std::swap(uv0, uv1);
    }
    if (t0.y > t2.y) {
        std::swap(t0, t2);
        std::swap(uv0, uv2);
    }
    if (t1.y > t2.y) {
        std::swap(t1, t2);
        std::swap(uv1, uv2);
    }

    int total_height = t2.y - t0.y;
    for (int i = 0; i < total_height; i++) {
        bool second_half = i > t1.y - t0.y || t1.y == t0.y;
        int segment_height = second_half ? t2.y - t1.y : t1.y - t0.y;
        float alpha = (float) i / total_height;
        float beta = (float) (i - (second_half ? t1.y - t0.y : 0)) /
                     segment_height; // be careful: with above conditions no division by zero here
        Vec3i A = t0 + Vec3f(t2 - t0) * alpha;
        Vec3i B = second_half ? t1 + Vec3f(t2 - t1) * beta : t0 + Vec3f(t1 - t0) * beta;
        Vec2i uvA = uv0 + (uv2 - uv0) * alpha;
        Vec2i uvB = second_half ? uv1 + (uv2 - uv1) * beta : uv0 + (uv1 - uv0) * beta;
        if (A.x > B.x) {
            std::swap(A, B);
            std::swap(uvA, uvB);
        }
        for (int j = A.x; j <= B.x; j++) {
            float phi = B.x == A.x ? 1. : (float) (j - A.x) / (float) (B.x - A.x);
            Vec3i P = Vec3f(A) + Vec3f(B - A) * phi;
            Vec2i uvP = uvA + (uvB - uvA) * phi;
            int idx = P.x + P.y * width;
            if (zbuffer[idx] < P.z) {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(color.r * intensity, color.g * intensity, color.b * intensity));
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x + 1.) * width / 2. + .5), int((v.y + 1.) * height / 2. + .5), v.z);
}

int main() {
    {
        model = new Model("../obj/african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);
        float *zbuffer = new float[width * height + 1];
        for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());


        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3f pts[3];
            for (int i = 0; i < 3; i++) pts[i] = world2screen(model->vert(face[i]));
            triangle(pts, zbuffer, image, TGAColor(rand() % 255, rand() % 255, rand() % 255, 255));
        }

        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output_coloful.tga");
        delete[] zbuffer;
        delete model;
    }
    {
        model = new Model("../obj/african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);
        float *zbuffer = new float[width * height + 1];
        for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

        Vec3f light_dir(0, 0, -1);
        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3f pts[3];
            Vec3f vs[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(face[j]);
                pts[j] = world2screen(v);
                vs[j] = v;
            }
            Vec3f n = (vs[2] - vs[0]) ^(vs[1] - vs[0]); // cross product
            n.normalize();
            float intensity = n * light_dir; // scalar product
            if (intensity > 0) {
                triangle(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
            }
        }
        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output_light.tga");
        delete[] zbuffer;
        delete model;

    }
    {
        model = new Model("../obj/african_head.obj");
        TGAImage image(width, height, TGAImage::RGB);
        int *zbuffer = new int[width * height + 1];
        for (int i = width * height; i--; zbuffer[i] = std::numeric_limits<int>::min());

        Vec3f light_dir(0, 0, -1);
        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3f pts[3];
            Vec3f vs[3];
            for (int j = 0; j < 3; j++) {
                Vec3f v = model->vert(face[j]);
                pts[j] = world2screen(v);
                vs[j] = v;
            }
            Vec3f n = (vs[2] - vs[0]) ^(vs[1] - vs[0]); // cross product
            n.normalize();
            float intensity = n * light_dir; // scalar product
            if (intensity > 0) {
                Vec2i uv[3];
                for (int k = 0; k < 3; k++) {
                    uv[k] = model->uv(i, k);
                }
                triangle_with_texture(pts[0], pts[1], pts[2], uv[0], uv[1], uv[2], image, intensity,
                          zbuffer);
            }
        }
        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output_texture.tga");
        delete[] zbuffer;
        delete model;
    }
    return 0;
}
