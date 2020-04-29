#include "tgaimage.h"

const TGAColor red = TGAColor(0xff,0,0,0xff);
const TGAColor green = TGAColor(0, 0xff, 0, 0xff);
const TGAColor blue = TGAColor(0, 0, 0xff, 0xff);
const TGAColor white = TGAColor(0xff, 0xff, 0xff, 0xff);

int main(){
    TGAImage image(100,100,TGAImage::RGB);

    image.set(20,30,red);
    image.set(60, 60, green);
    image.set(90, 10, blue);
    image.set(50, 50, white);

    image.write_tga_file("output.tga");
    return 0;
}