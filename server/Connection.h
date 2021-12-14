#ifndef CONNECTION_H
#define CONNECTION_H

#include "Channel.h"
#include "MessageSendController.h"
#include "../utils/RFCprotocol.h"

class Channel;

class Connection {
    private:
        bool alive = true;
        int retryCount = 0;
        std::queue<MessageSendController*> message_queue;
        std::chrono::time_point<std::chrono::system_clock> last_timestamp;
    public:
        int socket;
        Channel* cur_channel;
        std::list<Connection*>::iterator channel_pos;
        std::string nick;
        std::string ip_addr;
        Connection(int, std::string);
        ~Connection();
        void confirmReceive();
        void write(MessageSendController*);
        // void pong();
        void send_msg(Message*);
        Message* read(int&);
        int file_size;
        std::fstream cur_file;
        int written_size;
        std::string file_name;
};

#endif