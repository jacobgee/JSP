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
#include <fstream>
#include <cstdlib>
#include <cstring>
#include "JSP.hpp"

class JSPClient {
    JSP *mProtocol;
    const char* mServer;
    unsigned char** mData;
    int mNumChunks;
    int mCurrent;
    int mTotalReceived;
public:
    JSPClient();
    JSPClient(const char* server, int port);
    ~JSPClient();
    void connect();
    int init();
    void fetch();
    void saveFile(const char*);
    void setServer(const char* server) {mServer = server;}
};

void displayUsage();

#endif /* Client_hpp */
