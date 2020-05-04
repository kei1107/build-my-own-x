#include <vector>
#include <iostream>
#include <cmath>
#include "tgaimage.h"
#include "model.h"
#include "geometry.h"

const TGAColor white  = TGAColor(255, 255, 255, 255);
const TGAColor red    = TGAColor(255, 0,   0,   255);
const TGAColor green  = TGAColor(0,   255, 0,   255);
const TGAColor blue   = TGAColor(0,   0,   255, 255);
const TGAColor yellow = TGAColor(255, 255, 0,   255);

Model *model = nullptr;
int width  = 100;
int height = 100;
const int depth  = 255;

void line(Vec3i p0, Vec3i p1, TGAImage &image,const TGAColor& color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t+.5;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Matrix viewport(int x, int y, int w, int h) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

Matrix translation(Vec3f v) {
    Matrix Tr = Matrix::identity(4);
    Tr[0][3] = v.x;
    Tr[1][3] = v.y;
    Tr[2][3] = v.z;
    return Tr;
}

Matrix zoom(float factor) {
    Matrix Z = Matrix::identity(4);
    Z[0][0] = Z[1][1] = Z[2][2] = factor;
    return Z;
}

Matrix rotation_x(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[1][1] = R[2][2] = cosangle;
    R[1][2] = -sinangle;
    R[2][1] =  sinangle;
    return R;
}

Matrix rotation_y(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[0][0] = R[2][2] = cosangle;
    R[0][2] =  sinangle;
    R[2][0] = -sinangle;
    return R;
}

Matrix rotation_z(float cosangle, float sinangle) {
    Matrix R = Matrix::identity(4);
    R[0][0] = R[1][1] = cosangle;
    R[0][1] = -sinangle;
    R[1][0] =  sinangle;
    return R;
}

void triangle(Vec3i t0, Vec3i t1, Vec3i t2, Vec2i uv0, Vec2i uv1, Vec2i uv2, TGAImage &image, float intensity, int *zbuffer) {
    if (t0.y==t1.y && t0.y==t2.y) return; // i dont care about degenerate triangles
    if (t0.y>t1.y) { std::swap(t0, t1); std::swap(uv0, uv1); }
    if (t0.y>t2.y) { std::swap(t0, t2); std::swap(uv0, uv2); }
    if (t1.y>t2.y) { std::swap(t1, t2); std::swap(uv1, uv2); }

    int total_height = t2.y-t0.y;
    for (int i=0; i<total_height; i++) {
        bool second_half = i>t1.y-t0.y || t1.y==t0.y;
        int segment_height = second_half ? t2.y-t1.y : t1.y-t0.y;
        float alpha = (float)i/total_height;
        float beta  = (float)(i-(second_half ? t1.y-t0.y : 0))/segment_height; // be careful: with above conditions no division by zero here
        Vec3i A   =               t0  + Vec3f(t2-t0  )*alpha;
        Vec3i B   = second_half ? t1  + Vec3f(t2-t1  )*beta : t0  + Vec3f(t1-t0  )*beta;
        Vec2i uvA =               uv0 +      (uv2-uv0)*alpha;
        Vec2i uvB = second_half ? uv1 +      (uv2-uv1)*beta : uv0 +      (uv1-uv0)*beta;
        if (A.x>B.x) { std::swap(A, B); std::swap(uvA, uvB); }
        for (int j=A.x; j<=B.x; j++) {
            float phi = B.x==A.x ? 1. : (float)(j-A.x)/(float)(B.x-A.x);
            Vec3i   P = Vec3f(A) + Vec3f(B-A)*phi;
            Vec2i uvP =     uvA +   (uvB-uvA)*phi;
            int idx = P.x+P.y*width;
            if (zbuffer[idx]<P.z) {
                zbuffer[idx] = P.z;
                TGAColor color = model->diffuse(uvP);
                image.set(P.x, P.y, TGAColor(color.r*intensity, color.g*intensity, color.b*intensity));
            }
        }
    }
}


int main() {
    {
        width  = 100;
        height = 100;
        model = new Model("../obj/cube.obj");
        TGAImage image(width, height, TGAImage::RGB);
        Matrix VP = viewport(width / 4, width / 4, width / 2, height / 2);

        { // draw the axes
            Vec3f x(1.f, 0.f, 0.f), y(0.f, 1.f, 0.f), o(0.f, 0.f, 0.f);
            o = m2v(VP * v2m(o));
            x = m2v(VP * v2m(x));
            y = m2v(VP * v2m(y));
            line(o, x, image, red);
            line(o, y, image, green);
        }
        for (int i = 0; i < model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            for (int j = 0; j < (int) face.size(); j++) {
                Vec3f wp0 = model->vert(face[j]);
                Vec3f wp1 = model->vert(face[(j + 1) % face.size()]);

                { // draw the original model
                    Vec3f sp0 = m2v(VP * v2m(wp0));
                    Vec3f sp1 = m2v(VP * v2m(wp1));
                    line(sp0, sp1, image, white);
                }

                { // draw the deformed model
                    //*
                    Matrix T = zoom(1.5);
                    //*/
                    /*
                    Matrix T = Matrix::identity(4);
                    T[0][1] = 0.333;
                    //*/
                    /*
                    Matrix T = translation(Vec3f(.33, .5, 0))*rotation_z(cos(10.*M_PI/180.), sin(10.*M_PI/180.));
                    //*/
                    Vec3f sp0 = m2v(VP * T * v2m(wp0));
                    Vec3f sp1 = m2v(VP * T * v2m(wp1));
                    line(sp0, sp1, image, yellow);
                }
            }
            break;
        }
        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output.tga");
        delete model;
    }
    {
        width  = 800;
        height = 800;
        Vec3f light_dir(0,0,-1);
        Vec3f camera(0,0,3);

        model = new Model("../obj/african_head.obj");
        int* zbuffer = new int[width*height+1];
        std::fill(zbuffer,zbuffer+width*height+1,std::numeric_limits<int>::min());

        Matrix Projection   = Matrix::identity(4);
        Matrix ViewPort     = viewport(width/8,height/8,width*3/4,height*3/4);
        Projection[3][2]    = -1.f/camera.z;

        TGAImage image(width,height,TGAImage::RGB);
        for (int i=0; i<model->nfaces(); i++) {
            std::vector<int> face = model->face(i);
            Vec3i screen_coords[3];
            Vec3f world_coords[3];
            for (int j=0; j<3; j++) {
                Vec3f v = model->vert(face[j]);
                screen_coords[j] =  m2v(ViewPort*Projection*v2m(v));
                world_coords[j]  = v;
            }
            Vec3f n = (world_coords[2]-world_coords[0])^(world_coords[1]-world_coords[0]);
            n.normalize();
            float intensity = n*light_dir;
            if (intensity>0) {
                Vec2i uv[3];
                for (int k=0; k<3; k++) {
                    uv[k] = model->uv(i, k);
                }
                triangle(screen_coords[0], screen_coords[1], screen_coords[2], uv[0], uv[1], uv[2], image, intensity, zbuffer);
            }
        }
        image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
        image.write_tga_file("output_african.tga");

        { // dump z-buffer (debugging purposes only)
            TGAImage zbimage(width, height, TGAImage::GRAYSCALE);
            for (int i=0; i<width; i++) {
                for (int j=0; j<height; j++) {
                    zbimage.set(i, j, TGAColor(zbuffer[i+j*width], 1));
                }
            }
            zbimage.flip_vertically(); // i want to have the origin at the left bottom corner of the image
            zbimage.write_tga_file("zbuffer.tga");
        }
        delete model;
        delete[] zbuffer;
    }
    return 0;
}
