#include "bitmapUtils.h"

string BitmapUtils::bitmap = "";

void BitmapUtils::readFile(string path_to_file) {
    fstream bitmapFile(path_to_file);
    unsigned char c;
    while (bitmapFile >> c) {
        bitmap += c;
    }
}

bool BitmapUtils::isInside(Cell3DPosition catomPosition, int side_size) {
    unsigned int pos = catomPosition[0] +
        catomPosition[1]*side_size +
        catomPosition[2]*side_size*side_size;
    unsigned char c = bitmap[pos/8];
    return ((c >> (pos%8)) & 1);
}
