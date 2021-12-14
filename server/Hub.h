#ifndef HUB_H
#define HUB_H

#include "Connection.h"
#include "MessageSendController.h"
#include "../utils/RFCprotocol.h"
#include "Handlers.h"
#include "Channel.h"

class Hub {
    private:
        int hubSocket = 0;
        void waitConnection();
        void IOConnections();
        std::unordered_map<string, std::function<void(Message*, Hub*, Connection*)>> handlers;
    public:
        std::unordered_map<std::string, Channel*> channels;
        bool alive;
        std::list<Connection*> connections;
        std::unordered_map<std::string, Connection*> nicks;
        void setAlive(bool);
        Hub();
        void run(int);
        ~Hub();
        
};

#endif