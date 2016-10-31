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
    mProtocol->tunnel(mServer); // Let our UDP protocol know of the server
}

int JSPClient::init()
{
    char buf[1025]; // empty buffer
    char chk[5] = {'W','A','I','T','\0'}; // Empty buffer
    long size = 0;  // size of hte file we are downloading
    
    while (strcmp(chk, "HELO") != 0) // Poke server until we get what we want.
    {
        mProtocol->send("POKE", 4);  // init the communication
        strcpy(buf, mProtocol->replyWait()); // copy the reply to the buffer
        for(int i = 0; i < 4; i++)
            chk[i] = buf[i];         // the command?
    }
    size = atol(&buf[5]);            // get the size of the file
    mNumChunks = (size/1024)+1;      // calculate the number of chunks we need to save
    
    mCurrent = 0;                    // set our current chunk to 0
    
    return size;
}

void JSPClient::fetch(const char* filename)
{
    char buff[1026];                 // buffer
    unsigned char pkt;               // which paket are we getting?
    int packet;                      // convert unsigned char to int for comparason
    if(mCurrent == -1)               // mem not allocated. try again
        return;
    
    mTotalReceived = 0;              // total chunks recvd.
    
    FILE* out;
    out = fopen(filename, "wb");     // open file for writing
    
    if(out == NULL) // if it fails.
    {
        std::cerr << "Invalid file!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    while(mTotalReceived < mNumChunks)
    {
        std::string msg;
        msg = "YAAS ";                  // ack packet
        msg += (unsigned char)mCurrent; // convert the current packet to get the packet
        std::cout << "\tREQ: " << mCurrent << std::endl; // let user know what we are requesting
        mProtocol->send(msg.c_str(), 6);                 // send ack to server
        memcpy(buff, mProtocol->replyWait(), 1026);      // listen to what server has to say
        pkt = (unsigned char) buff[1024];                // get the last byte, which is the packet num
        
        packet = (int) pkt;                              // convert to int for comparing
        
        std::cout << "\tREC: " << packet << std::endl;   // let user know what we recv'd.
        
        if (packet == mCurrent)                          // got what we expected
        {
            fwrite(buff, 1, 1024, out);                  // write to file
            // rdy for next
            mCurrent++;
            mTotalReceived++;
        } else {
            //std::cout << "Invalid Packet: " << packet << std::endl;
            continue; // got wrong packet, ask again
        }
        if(mCurrent > 255) // refresh counter
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