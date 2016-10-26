//
//  JSP.hpp
//  jsend
//
//  Created by Jacob Gutierrez on 10/10/16.
//  Copyright © 2016 Jacob Gutierrez. All rights reserved.
//

#ifndef JSP_hpp
#define JSP_hpp

#define STANDARD_PORT 2525
#define BUFLEN 1024
#define MAXTHR 3

// STATE CODES
#define NOSOCKET   0
#define NOTUNNEL 100
#define TUNNEL   101
#define NOBIND   300
#define BOUND    301

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

class Caller
{
public:
    sockaddr_in c_addr;
    char message[BUFLEN];
};

class JSP {
    // variables
    int mPort;                              // The port to listen on
    int mSocket;                            // The socket we are listening on
    int mStatus;                            // Bind Status
    int mState;                             // Current JSP state
    
    struct sockaddr_in mLocal, mForeign;    // IP Addresses
    
    // functions
    void die(const char*);
public:
    JSP();
    JSP(int port);
    ~JSP();
    void bindSocket();
    void tunnel(const char*);
    Caller* listen();
    char* replyWait();
    void send(const char*);
    void send(Caller*, const char*);
    void setPort(int port);
    int getPort();
};

#endif /* JSP_hpp */
