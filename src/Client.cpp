//
//  Client.cpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#include "Client.hpp"

JSPClient::JSPClient()
{
    mServer = NULL;
    mProtocol = new JSP();
}

JSPClient::JSPClient(const char* server, int port)
{
    mServer = server;
    mProtocol = new JSP(port);
}

JSPClient::~JSPClient()
{
    delete mProtocol;
}

void JSPClient::connect()
{
    if(mServer == NULL)
    {
        std::cerr << "Server not Specified" << std::endl;
        exit(EXIT_FAILURE);
    }
    mProtocol->tunnel(mServer);
}

int JSPClient::init()
{
    char buf[1025];
    char chk[5] = {'W','A','I','T','\0'};
    long size = 0;
    
    while (strcmp(chk, "HELO") != 0)
    {
        mProtocol->send("POKE");
        strcpy(buf, mProtocol->replyWait());
        for(int i = 0; i < 4; i++)
            chk[i] = buf[i];
    }
    size = atol(&buf[5]);
    mNumChunks = (size/1024)+1;
    
    mData = new unsigned char*[mNumChunks];
    
    for(int i = 0 ; i < mNumChunks; i++)
    {
        mData[i] = new unsigned char[1024];
    }
    
    return size;
}

int main(int argc, char** argv)
{
    std::cout << "Client Started." << std::endl;
    JSPClient *client = new JSPClient("127.0.0.1", 2525);
    client->connect();
    std::cout << "File Size: " << client->init() << std::endl;
    delete client;
    std::cout << "Client Stopped." << std::endl;
    return 0;
}