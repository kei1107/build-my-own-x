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
bool inside(Vec2i &p, Vec2i &p0, Vec2i &p1, Vec2i &p2) {
    double Area = 0.5 * (-p1.y * p2.x + p0.y * (-p1.x + p2.x) + p0.x * (p1.y - p2.y) + p1.x * p2.y);
    double s = 1 / (2 * Area) * (p0.y * p2.x - p0.x * p2.y + (p2.y - p0.y) * p.x + (p0.x - p2.x) * p.y);
    double t = 1 / (2 * Area) * (p0.x * p1.y - p0.y * p1.x + (p0.y - p1.y) * p.x + (p1.x - p0.x) * p.y);

    return (0 - eps < s) && (s < 1 + eps) && (0 - eps < t) && (t < 1 + eps) && (0 - eps < 1 - s - t) && (1 - s - t < 1 + eps);

}

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

void triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    line(t0, t1, image, color);
    line(t1, t2, image, color);
    line(t2, t0, image, color);
}

void myfill_triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    // sort by y
    if (t0.y > t1.y) std::swap(t0, t1);
    if (t0.y > t2.y) std::swap(t0, t2);
    if (t1.y > t2.y) std::swap(t1, t2);

    int len_y0_y2 = t2.y - t0.y;
    int len_y0_y1 = t1.y - t0.y;
    int len_y1_y2 = t2.y - t1.y;
    if (len_y0_y2 == 0) len_y0_y2++;
    if (len_y0_y1 == 0) len_y0_y1++;
    if (len_y1_y2 == 0) len_y1_y2++;
    for (int y = t0.y; y <= t2.y; y++) {
        float c1 = (y - t0.y) / (float) len_y0_y2;
        Vec2i p1 = t0 + (t2 - t0) * c1;

        if (y <= t1.y) {
            float c2 = (y - t0.y) / (float) len_y0_y1;
            Vec2i p2 = t0 + (t1 - t0) * c2;

            for (int x = std::min(p1.x, p2.x); x <= std::max(p1.x, p2.x); x++) {
                image.set(x, y, color);
            }
        } else {
            float c2 = (y - t1.y) / (float) len_y1_y2;
            Vec2i p2 = t1 + (t2 - t1) * c2;

            for (int x = std::min(p1.x, p2.x); x <= std::max(p1.x, p2.x); x++) {
                image.set(x, y, color);
            }
        }
    }
}
void myfill_triangle2(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage &image, TGAColor color) {
    int lx = std::min({t0.x,t1.x,t2.x});
    int rx = std::max({t0.x,t1.x,t2.x});
    int ly = std::min({t0.y,t1.y,t2.y});
    int ry = std::max({t0.y,t1.y,t2.y});

    for(int x = lx; x <= rx; x++){
        for(int y = ly; y <= ry; y++){
            Vec2i p(x,y);
            if(inside(p,t0,t1,t2)) image.set(x,y,color);
        }
    }
}

void practice1() {
    TGAImage image(200, 200, TGAImage::RGB);

    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    myfill_triangle(t0[2], t0[0], t0[1], image, red);
    myfill_triangle(t1[0], t1[2], t1[1], image, green);
    myfill_triangle(t2[0], t2[1], t2[2], image, blue);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output1.tga");
}
void practice2() {
    TGAImage image(200, 200, TGAImage::RGB);

    Vec2i t0[3] = {Vec2i(10, 70), Vec2i(50, 160), Vec2i(70, 80)};
    Vec2i t1[3] = {Vec2i(180, 50), Vec2i(150, 1), Vec2i(70, 180)};
    Vec2i t2[3] = {Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180)};

    myfill_triangle2(t0[2], t0[0], t0[1], image, red);
    myfill_triangle2(t1[0], t1[2], t1[1], image, green);
    myfill_triangle2(t2[0], t2[1], t2[2], image, blue);

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output2.tga");
}
void practice3() {
    const int width = 800;
    const int height = 800;

    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("../obj/african_head.obj");

    for(int i = 0; i < model->nfaces(); i++){
        std::vector<int> face = model->face(i);
        Vec2i ts[3];
        for(int j = 0; j < 3;j++){
            Vec3f v = model->vert(face[j]);
            ts[j] = Vec2i((v.x+1.)*width/2.,(v.y+1.)*height/2.);
        }
        myfill_triangle(ts[0],ts[1],ts[2],image,TGAColor(rand()%255,rand()%255,rand()%255,255));
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output3.tga");
}

void practice4(){
    const int width = 800;
    const int height = 800;

    TGAImage image(width, height, TGAImage::RGB);
    model = new Model("../obj/african_head.obj");

    Vec3f light_dir(0,0,-1);
    for(int i = 0; i < model->nfaces(); i++){
        std::vector<int> face = model->face(i);
        Vec2i ts[3];
        Vec3f vs[3];
        for(int j = 0; j < 3;j++){
            Vec3f v = model->vert(face[j]);
            ts[j] = Vec2i((v.x+1.)*width/2.,(v.y+1.)*height/2.);
            vs[j] = v;
        }
        Vec3f n = (vs[2]-vs[0])^(vs[1]-vs[0]); // cross product
        n.normalize();
        float intensity = n*light_dir; // scalar product
        if(intensity > 0){
            myfill_triangle(ts[0],ts[1],ts[2],image,TGAColor(intensity*255,intensity*255,intensity*255,255));
        }
    }
    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output4.tga");
}

int main() {

    practice1();
    practice2();
    practice3();
    practice4();

    return 0;
}