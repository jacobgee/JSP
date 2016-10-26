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
#include <queue>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <pthread.h>
#include "SafeExit.hpp"
#include "JSP.hpp"

#define MAXTHREADS 10

typedef struct ClientStatus {
    Caller client;
    time_t lastPacket;
    int eRTT = 0;
    int std = 0;
    int seq = 0;
    long absoluteByteLocation = 0;
    
    bool operator<(const ClientStatus& rhs) const
    {
        return eRTT < rhs.eRTT;
    }
    
    bool operator==(const ClientStatus& rhs) const
    {
        return seq == rhs.seq;
    }
    
} client_t; 

class JSPServer
{
    JSP *mProtocol;
    long mFileSize;
    pthread_t mThreads[MAXTHREADS];
    int mUsage[MAXTHREADS];
    void dispatchCommand(Caller*);
    unsigned char **mData;
    int mNumChunks;
    std::priority_queue<client_t> mClientStatus;
    client_t qPop(Caller*);
public:
    JSPServer();
    JSPServer(int);
    ~JSPServer();
    long openFile(const char*);
    void listen();
    void timeouts();
    void* ListenThread(int threadid);
    static void* ListenThreadHelper(void* arguments);
};

struct ThreadArgs {
    JSPServer *context;
    int threadid;
};

void displayUsage();

#endif /* Server_hpp */
