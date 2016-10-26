//
//  Client.hpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#ifndef Client_hpp
#define Client_hpp

#include <iostream>
#include <cstdlib>
#include <cstring>
#include "JSP.hpp"

class JSPClient {
    JSP *mProtocol;
    const char* mServer;
    unsigned char** mData;
    int mNumChunks;

public:
    JSPClient();
    JSPClient(const char* server, int port);
    ~JSPClient();
    void connect();
    int init();
    void setServer(const char* server) {mServer = server;}
};

void displayUsage();

#endif /* Client_hpp */
