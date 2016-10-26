//
//  Server.hpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <pthread.h>
#include "SafeExit.hpp"
#include "JSP.hpp"

#define MAXTHREADS 10

class JSPServer
{
    JSP *mProtocol;
    long mFileSize;
    pthread_t mThreads[MAXTHREADS];
    int mUsage[MAXTHREADS];
    void dispatchCommand(Caller*);
    unsigned char **mData;
    int mNumChunks;
public:
    JSPServer();
    JSPServer(int);
    ~JSPServer();
    long openFile(const char*);
    void listen();
    void* ListenThread();
    static void* ListenThreadHelper(void* context) {return ((JSPServer*)context)->ListenThread();}
};

void displayUsage();

#endif /* Server_hpp */
