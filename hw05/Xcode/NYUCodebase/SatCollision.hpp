//
//  SatCollision.hpp
//  NYUCodebase
//
//  Created by Stanley Zeng on 4/3/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef SatCollision_hpp
#define SatCollision_hpp

#include <stdio.h>
#include <vector>
#include <utility>

bool CheckSATCollision(const std::vector<std::pair<float,float>> &e1Points, const std::vector<std::pair<float,float>> &e2Points, std::pair<float,float> &penetration);


#endif /* SatCollision_hpp */
