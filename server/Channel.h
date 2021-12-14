#ifndef CHANNEL_H
#define CHANNEL_H

#include "Connection.h"
class Connection;

class Channel {
    private:
        std::list<Connection*> whitelist;
    public:
        std::string name;
        Connection* admin;
        std::list<Connection*> members;
        std::list<Connection*> mutedMembers;
        
        Channel(std::string, Connection*);
        ~Channel();
        
        void mute(Connection*);
        void unmute(Connection*);
        
        void connect(Connection*);

        void remove(Connection*); 
        void remove(std::string);
        
        Connection* find(std::string);
    };


#endif