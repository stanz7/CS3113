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
public:
    FlareMap();
    ~FlareMap();
    
    void Load(const std::string fileName);
    
    int mapWidth;
    int mapHeight;
    unsigned int **mapData;
    void drawMap();
    std::vector<FlareMapEntity> entities;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    
private:
    
    bool ReadHeader(std::ifstream &stream);
    bool ReadLayerData(std::ifstream &stream);
    bool ReadEntityData(std::ifstream &stream);
    
};

#endif /* FlareMap_hpp */
