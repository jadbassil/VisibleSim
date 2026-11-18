#include "stoyUtils.h"

vector<Brick> StoyUtils::bricks;

void StoyUtils::readFile(string path_to_file) {
    fstream stoyFile;
    stoyFile.open(path_to_file);

    Brick b;
    float x1, y1, z1, x2, y2, z2;
    while (stoyFile >> x1 >> y1 >> z1) {
        stoyFile >> x2 >> y2 >> z2;
        b.p1.set(x1, y1, z1);
        b.p2.set(x2, y2, z2);
        bricks.push_back(b);
    }

    stoyFile.close();
}

bool StoyUtils::isInside(Vector3D catomPosition) {
    for (unsigned int i = 0; i < bricks.size(); i++) {
        if (bricks[i].p1[0] <= catomPosition[0] &&
            bricks[i].p1[1] <= catomPosition[1] &&
            bricks[i].p1[2] <= catomPosition[2] &&
            bricks[i].p2[0] >= catomPosition[0] &&
            bricks[i].p2[1] >= catomPosition[1] &&
            bricks[i].p2[2] >= catomPosition[2])
            return true;
    }
    return false;
}
