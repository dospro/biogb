#ifndef BIOGB_NET
#define BIOGB_NET

#define SERVER 0xE0
#define CLIENT 0xF0

#include <SDL/SDL_net.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "CPU/cCpu.h"

class cNet {
    IPaddress ip, *remoteip;
    TCPsocket server, client;
    s8 message[8];
    s32 len;
    u16 port;

    bool active;
    u8 conType;

   public:
    cNet();
    bool init(no);
    void send(u8 data);
    u8 recieve(void);
    void finish(void);

    bool isActive(void);
};

#endif
