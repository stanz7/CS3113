
#include "FlareMap.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#define spriteCountX 16
#define spriteCountY 8
#define TILE_SIZE 0.5f


FlareMap::FlareMap() {
    mapData = nullptr;
    mapWidth = -1;
    mapHeight = -1;
}

FlareMap::~FlareMap() {
    for(int i=0; i < mapHeight; i++) {
        delete mapData[i];
    }
    delete mapData;
}

bool FlareMap::ReadHeader(std::ifstream &stream) {
    std::string line;
    mapWidth = -1;
    mapHeight = -1;
    while(std::getline(stream, line)) {
        if(line == "") { break; }
        std::istringstream sStream(line);
        std::string key,value;
        std::getline(sStream, key, '=');
        std::getline(sStream, value);
        if(key == "width") {
            mapWidth = std::atoi(value.c_str());
        } else if(key == "height"){
            mapHeight = std::atoi(value.c_str());
        }
    }
    if(mapWidth == -1 || mapHeight == -1) {
        return false;
    } else {
        mapData = new unsigned int*[mapHeight];
        for(int i = 0; i < mapHeight; ++i) {
            mapData[i] = new unsigned int[mapWidth];
        }
        return true;
    }
}

bool FlareMap::ReadLayerData(std::ifstream &stream) {
    std::string line;
    while(getline(stream, line)) {
        if(line == "") { break; }
        std::istringstream sStream(line);
        std::string key,value;
        std::getline(sStream, key, '=');
        std::getline(sStream, value);
        if(key == "data") {
            for(int y=0; y < mapHeight; y++) {
                getline(stream, line);
                std::istringstream lineStream(line);
                std::string tile;
                for(int x=0; x < mapWidth; x++) {
                    std::getline(lineStream, tile, ',');
                    unsigned int val = atoi(tile.c_str());
                    if(val > 0) {
                        mapData[y][x] = val-1;
                    } else {
                        mapData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}


bool FlareMap::ReadEntityData(std::ifstream &stream) {
    std::string line;
    std::string type;
    while(getline(stream, line)) {
        if(line == "") { break; }
        std::istringstream sStream(line);
        std::string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if(key == "type") {
            type = value;
        } else if(key == "location") {
            std::istringstream lineStream(value);
            std::string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            
            FlareMapEntity newEntity;
            newEntity.type = type;
            newEntity.x = std::atoi(xPosition.c_str());
            newEntity.y = std::atoi(yPosition.c_str());
            entities.push_back(newEntity);
        }
    }
    return true;
}

void FlareMap::Load(const std::string fileName) {
    std::ifstream infile(fileName);
    if(infile.fail()) {
        assert(false); // unable to open file
    }
    std::string line;
    while (std::getline(infile, line)) {
        if(line == "[header]") {
            if(!ReadHeader(infile)) {
                assert(false); // invalid file data
            }
        } else if(line == "[layer]") {
            ReadLayerData(infile);
        } else if(line == "[ObjectsLayer]") {
            ReadEntityData(infile);
        }
    }
}

void FlareMap::drawMap() {
    
    
    for (int y = 0; y < mapHeight; y++) {
        for (int x = 0; x < mapWidth; x++) {
            
            if (mapData[y][x] != 0) {
                float u = (float)(((int)mapData[y][x]) % spriteCountX) / (float)spriteCountX;
                float v = (float)(((int)mapData[y][x]) / spriteCountX) / (float)spriteCountY;
                
                float spriteWidth = 1.0f / (float)spriteCountX;
                float spriteHeight = 1.0f / (float)spriteCountY;
                
                vertexData.insert(vertexData.end(), {
                    TILE_SIZE * x, -TILE_SIZE * y,
                    TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                    TILE_SIZE * x, -TILE_SIZE * y,
                    (TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
                    (TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
                });
                
                texCoordData.insert(texCoordData.end(), {
                    u, v,
                    u, v + (spriteHeight),
                    u + spriteWidth, v + (spriteHeight),
                    u, v,
                    u + spriteWidth, v + (spriteHeight),
                    u + spriteWidth, v
                });
                
            }
        }
    }
}

