#include <vector>
#include <cmath>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor red = TGAColor(0xff, 0, 0, 0xff);
const TGAColor green = TGAColor(0, 0xff, 0, 0xff);
const TGAColor blue = TGAColor(0, 0, 0xff, 0xff);
const TGAColor white = TGAColor(0xff, 0xff, 0xff, 0xff);

Model *model = NULL;

double eps = 1e-9;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x - p1.x) < std::abs(p0.y - p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }
    for (int x = p0.x; x <= p1.x; x++) {
        float t = (x - p0.x) / (float) (p1.x - p0.x);
        int y = p0.y * (1. - t) + p1.y * t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void rasterize(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color, int ybuffer[]) {
    if (p0.x > p1.x) {
        std::swap(p0, p1);
    }
    for (int x = p0.x; x <= p1.x; x++) {
        float t = (x - p0.x) / (float) (p1.x - p0.x);
        int y = p0.y * (1. - t) + p1.y * t + .5;
        if (ybuffer[x] < y) {
            ybuffer[x] = y;
            image.set(x, 0, color);
        }
    }
}

void practice1() {
    const int width = 800;
    const int height = 500;

    { // just dumping the 2d scene (yay we have enough dimensions!)
        TGAImage scene(width, height, TGAImage::RGB);

        // scene "2d mesh"
        line(Vec2i(20, 34), Vec2i(744, 400), scene, red);
        line(Vec2i(120, 434), Vec2i(444, 400), scene, green);
        line(Vec2i(330, 463), Vec2i(594, 200), scene, blue);

        // screen line
        line(Vec2i(10, 10), Vec2i(790, 10), scene, white);

        scene.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        scene.write_tga_file("scene.tga");
    }
}

void practice2() {
    const int width = 800;
    const int height = 16;

    TGAImage render(width, 16, TGAImage::RGB);
    int ybuffer[width];
    std::fill(ybuffer, ybuffer + width, std::numeric_limits<int>::min());
    rasterize(Vec2i(20, 34), Vec2i(744, 400), render, red, ybuffer);
    rasterize(Vec2i(120, 434), Vec2i(444, 400), render, green, ybuffer);
    rasterize(Vec2i(330, 463), Vec2i(594, 200), render, blue, ybuffer);
    // 1-pixel wide image is bad for eyes, lets widen it
    for (int i = 0; i < width; i++) {
        for (int j = 1; j < 16; j++) {
            render.set(i, j, render.get(i, 0));
        }
    }
    render.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    render.write_tga_file("render.tga");
}


int main() {
    practice1();
    practice2();
    return 0;
}