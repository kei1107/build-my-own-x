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
const int width = 800;
const int height = 800;

void line_v1(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    for (float t = 0; t < 1; t += .01) {
        int x = x0 + (x1 - x0) * t;
        int y = y0 + (y1 - y0) * t;
        image.set(x, y, color);
    }
}

void line_v2(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float) (x1 - x0);
        int y = y0 * (1.0 - t) + y1 * t;
        image.set(x, y, color);
    }
}

void line_v3(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }
    for (int x = x0; x <= x1; x++) {
        float t = (x - x0) / (float) (x1 - x0);
        int y = y0 * (1. - t) + y1 * t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

void line_v4(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    float derror = std::abs(dy / (float) dx);
    float error = 0;

    for (int x = x0, y = y0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error += derror;
        if (error > 0.5) {
            y += (y1 > y0 ? 1 : -1);
            error -= 1.0;
        }
    }
}

void line_v5(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
        std::swap(x0, y0);
        std::swap(x1, y1);
        steep = true;
    }
    if (x0 > x1) {
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    int dx = x1 - x0;
    int dy = y1 - y0;
    int derror2 = std::abs(dy) * 2;
    int error2 = 0;

    for (int x = x0, y = y0; x <= x1; x++) {
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
        error2 += derror2;
        if (error2 > dx) {
            y += (y1 > y0 ? 1 : -1);
            derror2 -= dx * 2;
        }
    }
}

int main() {
    //TGAImage image(100, 100, TGAImage::RGB);
    /*
    line_v1(10,10,90,60,image,white);
    //*/
    /*
    line_v2(13, 20, 80, 40, image, green);
    line_v2(20, 13, 40, 80, image, red);
    line_v2(80, 40, 13, 20, image, red); // No
    //*/
    /*
    line_v3(13, 20, 80, 40, image, green);
    line_v3(20, 13, 40, 80, image, red);
    line_v3(80, 40, 13, 20, image, red); // No
    //*/
    /*
    line_v4(13, 20, 80, 40, image, white);
    line_v4(20, 13, 40, 80, image, red);
    line_v4(80, 40, 13, 20, image, red);
    //*/

    model = new Model("../obj/african_head.obj");
    TGAImage image(width, height, TGAImage::RGB);

    for(int i = 0; i < model->nfaces(); i++){
        std::vector<int> face = model->face(i);
        for(int j = 0; j < 3; j++){
            Vec3f v0 = model->vert(face[j]);
            Vec3f v1 = model->vert(face[(j+1)%3]);

            int x0 = (v0.x+1.)*width/2.;
            int y0 = (v0.y+1.)*height/2.;
            int x1 = (v1.x+1.)*width/2.;
            int y1 = (v1.y+1.)*height/2.;

            line_v3(x0,y0,x1,y1,image,blue);
        }
    }

    image.flip_vertically();
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}