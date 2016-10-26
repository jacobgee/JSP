//
//  JSP.cpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#include "JSP.hpp"

void JSP::die(const char* s)
{
    std::cerr << s << std::endl;
    exit(EXIT_FAILURE);
}

JSP::JSP()
{
    mPort = STANDARD_PORT;
    mState = NOSOCKET;
    if ((mSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("::: Could not open socket =( :::");
    }
    mState = NOBIND;
}

JSP::JSP(int port)
{
    mPort = port;
    mState = NOSOCKET;
    if ((mSocket=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("::: Could not open socket =( :::");
    }
    mState = NOBIND;
}

JSP::~JSP()
{
    // Do closing stuff here
    if(mSocket != -1)
        close(mSocket);
}

void JSP::bindSocket()
{
    mLocal.sin_family = AF_INET;
    mLocal.sin_port = htons(mPort);
    mLocal.sin_addr.s_addr = INADDR_ANY;
    
    mStatus = bind(mSocket, (struct sockaddr*)&mLocal, sizeof(mLocal));
    
    if(mStatus < 0)
        die("::: Error Binding Socket =( :::");
    
    mState = BOUND;
}

void JSP::tunnel(const char* addr)
{
    mForeign.sin_family = AF_INET;
    mForeign.sin_port = htons(mPort);
    if(inet_aton(addr , &mForeign.sin_addr) == 0)
    {
        die("::: Error mapping hostname =( :::");
    }
    mState = TUNNEL;
}

Caller* JSP::listen()
{
    struct sockaddr_in foreign;
    int recLen;
    socklen_t sLen = sizeof(foreign);
    char *buf = new char[BUFLEN];
    
    if(mState < BOUND)
        return NULL;
    
    // Listening...
    if((recLen = recvfrom(mSocket, buf, BUFLEN, 0, (struct sockaddr *) &foreign, &sLen)) < 0)
    {
        die("::: Could Not Recieve Packets =( :::");
    }
    
    Caller *c = new Caller();
    memcpy(&c->c_addr, &foreign, sizeof(foreign));
    memcpy(c->message, buf, BUFLEN);
    return c;
}

char* JSP::replyWait()
{
    struct sockaddr_in foreign;
    int recLen;
    socklen_t sLen = sizeof(foreign);
    char *buf = new char[BUFLEN];
    
    // Listening...
    if((recLen = recvfrom(mSocket, buf, BUFLEN, 0, (struct sockaddr *) &foreign, &sLen)) < 0)
    {
        die("::: Could Not Recieve Packets =( :::");
    }
    
    return buf;
}

void JSP::send(Caller *c, const char* msg)
{
    socklen_t sLen = sizeof(c->c_addr);
    if(sendto(mSocket, msg, strlen(msg), 0, (struct sockaddr *) &c->c_addr, sLen) == -1)
    {
        die("::: Error Sending packet to Client =( :::");
    }
}

void JSP::send(const char* msg)
{
    socklen_t sLen = sizeof(mForeign);
    if (sendto(mSocket, msg, strlen(msg) , 0,(struct sockaddr *) &mForeign, sLen)==-1)
    {
        die("::: Error Sending Packet =( :::");
    }
}