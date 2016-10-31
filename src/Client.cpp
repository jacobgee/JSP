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
    mCurrent = -1;
}

JSPClient::JSPClient(const char* server, int port)
{
    mServer = server;
    mProtocol = new JSP(port);
    mCurrent = -1;
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
        mProtocol->send("POKE", 4);
        strcpy(buf, mProtocol->replyWait());
        for(int i = 0; i < 4; i++)
            chk[i] = buf[i];
    }
    size = atol(&buf[5]);
    mNumChunks = (size/1024)+1;
    
    mCurrent = 0;
    
    return size;
}

void JSPClient::fetch(const char* filename)
{
    char buff[1026];
    unsigned char pkt;
    int packet;
    if(mCurrent == -1)
        return;
    
    mTotalReceived = 0;
    
    FILE* out;
    out = fopen(filename, "wb");
    
    if(out == NULL)
    {
        std::cerr << "Invalid file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    while(mTotalReceived < mNumChunks)
    {
        std::string msg;
        msg = "YAAS ";
        msg += (unsigned char)mCurrent;
        std::cout << "\tREQ: " << mCurrent << std::endl;
        mProtocol->send(msg.c_str(), 6);
        memcpy(buff, mProtocol->replyWait(), 1026);
        pkt = (unsigned char) buff[1024];
        
        packet = (int) pkt;
        
        std::cout << "\tREC: " << packet << std::endl;
        
        if (packet == mCurrent)
        {
            fwrite(buff, 1, 1024, out);
            // rdy for next
            mCurrent++;
            mTotalReceived++;
        } else {
            //std::cout << "Invalid Packet: " << packet << std::endl;
            continue;
        }
        if(mCurrent > 255)
            mCurrent = 0;
        std::cout << "Status: " << mTotalReceived << "/" << mNumChunks << std::endl;
    }
    std::cout << "Recieved all data!" << std::endl;
    fclose(out);
}

void displayUsage()
{
    std::cout << "Usage: jclient server_ip [port] filename" << std::endl;
    return;
}

int main(int argc, char** argv)
{
    int port = STANDARD_PORT;
    char *server;
    char *filename;

    if(argc < 3)
    {
        displayUsage();
        return EXIT_SUCCESS;
    }
    
    server = argv[1];
    
    if(argc == 4)
    {
        port = atoi(argv[2]);
        filename = argv[3];
        std::cout << "Contacting Server on Port " << port << std::endl;
    } else {
        filename = argv[2];
    }

    std::cout << "Client Started. Downloading: " << filename << std::endl;
    JSPClient *client = new JSPClient(server, 2525);
    client->connect();
    std::cout << "File Size: " << client->init() << std::endl;
    client->fetch(filename);
    delete client;
    std::cout << "Client Stopped." << std::endl;
    return 0;
}