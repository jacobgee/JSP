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
        mProtocol->send("POKE");
        strcpy(buf, mProtocol->replyWait());
        for(int i = 0; i < 4; i++)
            chk[i] = buf[i];
    }
    size = atol(&buf[5]);
    mNumChunks = (size/1024)+1;
    
    mCurrent = 0;
    
    return size;
}

void JSPClient::fetch()
{
    char buff[1026];
    char pkt;
    int packet;
    if(mCurrent == -1)
        return;
    
    mTotalReceived = 0;
    mData = new char*[mNumChunks];
    
    for(int i = 0 ; i < mNumChunks; i++)
    {
        mData[i] = new char[1024];
    }
    
    while(mTotalReceived < mNumChunks)
    {
        std::string msg;
        msg = "YAAS ";
        if(mCurrent != 0)
            msg += (char)mCurrent;
        else
            msg += 'i';
        //std::cout << "\tREQ: " << mCurrent << std::endl;
        mProtocol->send(msg.c_str());
        memcpy(buff, mProtocol->replyWait(), 1026);
        //std::cout << "GOT " << buff << std::endl;
        pkt = (char) buff[0];
        if(pkt == 'i')
            packet = 0;
        else
            packet = (int) pkt;
        
        if (packet == mCurrent)
        {
            //std::cout << "\tREC: " << packet << std::endl;
            // save data
            for(int i = 0; i < 1024; i++)
            {
                mData[mCurrent][i]=buff[i+1];
            }
            // rdy for next
            mCurrent++;
            mTotalReceived++;
        } else {
            continue;
        }
        if(mCurrent > 255)
            mCurrent = 0;
        std::cout << "Status: " << mTotalReceived << "/" << mNumChunks << std::endl;
    }
    std::cout << "Recieved all data!" << std::endl;
}

void JSPClient::saveFile(const char* filename)
{
    FILE* out;
    out = fopen(filename, "wb");
    for(int i = 0; i < mNumChunks; i++)
    {
        fwrite(mData[i], sizeof(char), 1024, out);
    }
    fclose(out);
    return;
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
    client->fetch();
    std::cout << "Saving file...";
    client->saveFile(filename);
    std::cout << "done." << std::endl;
    delete client;
    std::cout << "Client Stopped." << std::endl;
    return 0;
}