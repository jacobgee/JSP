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
        char * buffer = new char [mFileSize + 1];
        buffer[mFileSize] = '\0';
        
        // read data as a block:
        is.read (buffer,mFileSize);
        
        is.close();
        
        mNumChunks = (mFileSize / 1024) + 1;
        
        mData = new char*[mNumChunks];
        
        for(int i = 0 ; i < mNumChunks; i++)
        {
            mData[i] = new char[1024];
            for(int j = 0; j < 1024; j++)
                mData[i][j] = '\0';
        }
        
        int k=0;
        
        for(int i = 0; i < mNumChunks; i++)
        {
            for(int j = 0; j < 1024; j++)
            {
                mData[i][j] = buffer[k++];
            }
        }
        
        delete buffer;
        
        return mFileSize;
    }
    
    return -1; // failed
}

// Warning: Here be dragons
void JSPServer::listen()
{
    
    for(int i = 0; i < MAXTHREADS; i++)
    {
        struct ThreadArgs args;
        args.context = this;
        args.threadid = i;
        
        mUsage[i] = pthread_create(&mThreads[i], NULL,
                            &JSPServer::ListenThreadHelper, (void *)&args); // create thread
        if(mUsage[i]!=0)
            std::cerr << "::: Error Creating Thread " << i << "=( Error code: "<< mUsage[i] <<" :::" << std::endl;
    }
    
    //int status = 
    pthread_create(&mTimeout, NULL,
                                &JSPServer::TimeoutThreadHelper, (void *)this); // create timeout thread
    /*if(status == 0)
        pthread_join(mTimeout, NULL); // join if no error */
    
    for(int i = 0; i < MAXTHREADS; i++)
    {
        if(mUsage[i] == 0)
        {
            mUsage[i] = pthread_join(mThreads[i],NULL); // join threads
            //if(mUsage[i]!=0)
                //std::cerr << "::: Error Joining Thread " << i << " =( Error code: "<< mUsage[i] <<" :::" << std::endl;
        }
    }
}

void JSPServer::timeouts()
{
    if(mClientStatus.empty())
    {
        return; // nothing in timeout queue, jsut die.
    }
    client_t c;
    memcpy(&c, &mClientStatus.top(), sizeof(client_t)); // check the top
    time_t currentTime = time(NULL);  // get current time
    int delta = 0;
    int timeout = 0;
    timeout = (c.eRTT+3*c.std);
    delta = currentTime - c.lastPacket; // get time difference since last packet
    if(delta > timeout)        // if the difference is greater than the timeout
    {
        mClientStatus.pop();            // pop it and then
        std::cout << "Retry: ";         // resend
        dispatchCommand(&c.client);     // the packet
    }
}

void* JSPServer::ListenThread(int threadid)
{
    Caller *c = mProtocol -> listen();
    dispatchCommand(c);
    delete c;
    pthread_exit(NULL);
}

void* JSPServer::TimeoutThread()
{
    while(1)
    {
        //std::cout << "Checking Timeouts: " << std::endl;
        timeouts();
        sleep(1);
    }

    pthread_exit(NULL);
}

void* JSPServer::TimeoutThreadHelper(void* context)
{
    return ((JSPServer *)context)->TimeoutThread();
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
        // report file size to client
        msg = "HELO ";
        msg += std::to_string(mFileSize);
        // report init to console
        std::cout << "RECV Initiation from " << inet_ntoa(c->c_addr.sin_addr) << std::endl;
        // send file size to client
        mProtocol->send(c, msg.c_str(), msg.size());
        // now build the timeout packet
        client_t s;
        memcpy(&s.client, c, sizeof(Caller));
        s.lastPacket=time(NULL);
        s.eRTT = STANDARD_TIMEOUT;  // standard timeout
        s.std = 0;
        s.seq = 0;                  // seq index starts at 0
        s.absoluteByteLocation = 0; // start at the beginning
        mClientStatus.push(s);      // save to timeout
    }
    else if(strcmp(cmd, "YAAS") == 0)
    {
        int absolute, sRTT;
        std::string msg;
        float std;
        float eRTT;
        unsigned char ack = c->message[5];
        int packet;
        
        packet = (int) ack;
        
        
        // build timeout packet
        client_t s = qPop(c, packet-1);
        absolute = s.absoluteByteLocation;
        
        if(absolute + packet + 1 > mNumChunks)
            std::cout << inet_ntoa(c->c_addr.sin_addr) << " Complete." << std::endl;
        
        // is our sequence finished?
        if(packet == 0 && s.seq == 255)
            absolute += 256; // inc. the absolute position
        
        // calculate timeout
        sRTT = time(NULL) - s.lastPacket;
        eRTT = 0.825 * s.eRTT + 0.175 * sRTT;
        std = 0.75*s.std + 0.25 * abs(int(sRTT-eRTT));
        
        // build our timeout packet
        memcpy(&s.client, c, sizeof(Caller));
        s.lastPacket=time(NULL);
        s.eRTT = (int)eRTT;
        s.std = (int)std;
        s.seq = packet;
        s.absoluteByteLocation = absolute;
        mClientStatus.push(s); // save to queue.
        
        std::cout << inet_ntoa(c->c_addr.sin_addr) << " REQUESTS " << packet+absolute << std::endl;
        
        // now send packet
        msg.append(mData[packet+absolute],1024);
        msg += ack;
        mProtocol->send(c, msg.c_str(), 1026);
        
        //std::cout << "LOAD " << mData[packet+absolute] << std::endl;
        //std::cout << "SEND " << msg.c_str() << std::endl;
    }
        
}

client_t JSPServer::qPop(Caller* c, int seq)
{
    std::priority_queue<client_t> temp;
    client_t s;
    while(!mClientStatus.empty()) // dig through the timeout queue
    {
        s = mClientStatus.top(); // look at the top
        if(strcmp(inet_ntoa(c->c_addr.sin_addr), inet_ntoa(s.client.c_addr.sin_addr)) == 0)
        {
            mClientStatus.pop(); // pop if it's the one we are looking for.
            break;               // we found it, no need to continue
        }
        
        temp.push(s);            // save it to the temp
        mClientStatus.pop();     // clear it from queue
    }
    while(!temp.empty())         // dig through our temp queue
    {
        client_t r = temp.top(); // get the data
        mClientStatus.push(r);   // return it the the timeoutqueue
        temp.pop();              // clear temp queue
    }
    return s;                    // return what we found, or the last element.
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
    std::cout << "Usage: jserver filename [port]" << std::endl;
    std::cout << "\tStandard Port: 2525" << std::endl;
}
