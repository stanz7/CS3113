//
//  FlareMap.hpp
//  NYUCodebase
//
//  Created by Stanley Zeng on 3/27/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef FlareMap_hpp
#define FlareMap_hpp

#include <stdio.h>
#include <string>
#include <vector>

struct FlareMapEntity {
    std::string type;
    float x;
    float y;
};
class FlareMap {
private:
    bool readLayerData(std::ifstream &stream);
    bool readEntityData(std::ifstream &stream);
    bool readHeaderData(std::ifstream &stream);
public:
    FlareMap();
    void Load(const std::string fileName);
    int mapWidth;
    int mapHeight;
    unsigned int **mapData;
    std::vector<FlareMapEntity> entities;
};

#endif /* FlareMap_hpp */
