//
//  FlareMap.cpp
//  NYUCodebase
//
//  Created by Stanley Zeng on 3/27/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "FlareMap.hpp"
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <cassert>

FlareMap::FlareMap() {
    mapData = nullptr;
    mapWidth = -1;
    mapHeight = -1;
}

bool FlareMap::readLayerData(std::ifstream &stream) {
    std::string line;
    while(getline(stream, line)) {
        if (line == "") {break;}
        std::istringstream sStream(line);
        std::string key,value;
        getline(sStream, key, '=');
        getline(sStream, value);
        if (key == "data"){
            for (int y = 0; y < mapHeight; y++) {
                getline(stream, line);
                std::istringstream lineStream(line);
                std::string tile;
                
                for (int x = 0; x < mapWidth; x++ ){
                    getline(lineStream, tile, ',');
                    unsigned char val = (unsigned char)atoi(tile.c_str());
                    if (val > 0) {
                        //be careful, the tiles in this format are indexed from 1 not 0
                        mapData[y][x] = val -1;
                    } else {
                        mapData[y][x] = 0;
                    }
                }
            }
        }
    }
    return true;
}

bool FlareMap::readEntityData(std::ifstream &stream) {
    std::string line;
    std::string type;
    while(getline(stream, line)) {
        if (line == "") { break; }
        
        std::istringstream sStream(line);
        std::string key, value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if (key == "type") {
            type = value;
        } else if(key == "location") {
            
            std::istringstream lineStream(value);
            std::string xPosition, yPosition;
            getline(lineStream, xPosition, ',');
            getline(lineStream, yPosition, ',');
            
            FlareMapEntity newEntity;
            newEntity.type= type;
            newEntity.x = std::atoi(xPosition.c_str());
            newEntity.y = std::atoi(yPosition.c_str());
            entities.push_back(newEntity);
        }
    }
    return true;
}

bool FlareMap::readHeaderData(std::ifstream &stream){
    std::string line;
    mapWidth = -1;
    mapHeight = -1;
    while ( getline(stream, line)) {
        if (line == "") {break;}
        
        std::istringstream sStream(line);
        std::string key, value;
        getline(sStream, key, '=');
        getline(sStream, value);
        
        if (key == "width") {
            mapWidth = atoi(value.c_str());
        } else if (key == "height") {
            mapHeight = atoi(value.c_str());
        }
    }
    
    if (mapWidth == -1 || mapHeight == -1) {
        return false;
    } else {
        mapData = new unsigned int*[mapHeight];
        for (int i = 0; i < mapHeight; i++) {
            mapData[i] = new unsigned int[mapWidth];
        }
        return true;
    }
}

void FlareMap::Load(const std::string fileName) {
    std::ifstream ifs(fileName);
    if (ifs.fail()) {
        assert(false);
    }
    std::string str;
    while(getline(ifs, str)) {
        if (str == "[header]") {
            if (!readHeaderData(ifs)) {
                assert(false);
            }
        }
        else if (str == "[layer]") {
            readLayerData(ifs);
        }
        else if (str == "[ObjectsLayer]") {
            readEntityData(ifs);
        }
    }
}
