//
//  Server.cpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#include "Server.hpp"

JSPServer::JSPServer()
{
    //Create Socket
    mProtocol = new JSP();
    mProtocol -> bindSocket();
    for(int i=0; i < MAXTHREADS; i++)
    {
        mUsage[i]=-1;
    }

}

JSPServer::JSPServer(int port)
{
    //Create Socket
    mProtocol = new JSP(port);
    mProtocol -> bindSocket();
    for(int i=0; i < MAXTHREADS; i++)
    {
        mUsage[i]=-1;
    }

}

JSPServer::~JSPServer()
{
    delete mProtocol;
}

long JSPServer::openFile(const char* filename)
{
    std::ifstream is (filename, std::ifstream::binary);
    if (is) {
        // get length of file:
        is.seekg (0, is.end);
        mFileSize = is.tellg();
        is.seekg (0, is.beg);
        
        // allocate memory:
        char * buffer = new char [mFileSize];
        
        // read data as a block:
        is.read (buffer,mFileSize);
        
        is.close();
        
        mNumChunks = (mFileSize / 1024) + 1;
        
        mData = new unsigned char*[mNumChunks];
        
        for(int i = 0 ; i < mNumChunks; i++)
        {
            mData[i] = new unsigned char[1024];
        }
        
        return mFileSize;
    }
    
    return -1; // failed
}

void JSPServer::listen()
{
    
    for(int i = 0; i < MAXTHREADS; i++)
    {
        if(mUsage[i] == 0)
            continue;
        
        struct ThreadArgs args;
        args.context = this;
        args.threadid = i;
        
        mUsage[i] = pthread_create(&mThreads[i], NULL,
                            &JSPServer::ListenThreadHelper, (void *)&args);
        if(mUsage[i]!=0)
            std::cerr << "::: Error Creating Thread " << i << "=( :::" << std::endl;
    }
    
    for(int i = 0; i < MAXTHREADS; i++)
    {
        if(mUsage[i] == 0)
        {
            mUsage[i] = pthread_join(mThreads[i],NULL);
            if(mUsage[i]!=0)
                std::cerr << "::: Error Joining Thread " << i << " =( :::" << std::endl;
        }
    }
}

void* JSPServer::ListenThread(int threadid)
{
    Caller *c = mProtocol -> listen();
    dispatchCommand(c);
    mUsage[threadid] = -1;
    pthread_exit(NULL);
}

void* JSPServer::ListenThreadHelper(void* arguments)
{
    struct ThreadArgs *args =  (struct ThreadArgs *) arguments;
    
    return args->context->ListenThread(args->threadid);
}

void JSPServer::dispatchCommand(Caller *c)
{
    char cmd[5];
    
    cmd[4] = '\0';
    
    for(int i = 0; i < 4; i++)
        cmd[i] = c->message[i];

    if(strcmp(cmd, "POKE") == 0)
    {
        std::string msg;
        msg = "HELO ";
        msg += std::to_string(mFileSize);
        std::cout << "RECV Initiation from " << inet_ntoa(c->c_addr.sin_addr) << std::endl;
        mProtocol->send(c, msg.c_str());
    }
        
}

int main(int argc, char** argv)
{
    int port = STANDARD_PORT;
    long fSize = 0;
    const char* filename;
    if(argc != 2 && argc != 3)
    {
        displayUsage();
        return EXIT_SUCCESS;
    }
    
    filename = argv[1];
    
    if(argc == 3)
    {
        port = atoi(argv[2]);
        std::cout << "Server started on " << port << std::endl;
    }
    
    safeExit();
    
    std::cout << "Listening for clients..." << std::endl;
    JSPServer *server = new JSPServer(port);
    fSize = server -> openFile(filename);
    if(fSize == -1)
    {
        std::cerr << "::: Could not open file =( :::" << std::endl;
        exit(EXIT_FAILURE);
    }
    else
        std::cout << "File size: " << fSize << std::endl;
    
    while(1)
    {
        server->listen();
    }
    
    delete server;
    std::cout << "Server Stopped." << std::endl;
    return 0;
}

void displayUsage()
{
    std::cout << "Usage: server filename [port]" << std::endl;
}