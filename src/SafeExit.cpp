//
//  SafeExit.cpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#include "SafeExit.hpp"

void cleanExit(int p)
{
    exit(0);
}

void safeExit()
{
    signal(SIGTERM, cleanExit);
    signal(SIGINT, cleanExit);
    return;
}