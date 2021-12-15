#ifndef CLIENT_H
#define CLIENT_H

#include "../utils/RFCprotocol.h"

typedef void (* ExecutorHook)(class Client *, std::string&);
typedef void (* HandlerHook)(class Client *, class Message*);

class Client {
    private:
        std::map<std::string, ExecutorHook> executors;
        std::map<std::string, HandlerHook> handlers;

        int hub_socket;

        void reset();
    public:
        std::string nickname;
        std::string channel;
        std::chrono::steady_clock::time_point ping_time;
        std::chrono::steady_clock::time_point pong_time;
        bool connected;

        string download_file_name;
        bool downloading;
        int file_size;
        int downloaded_file_size;
        fstream cur_file;

        Client();
        
        void quit();
        void create_connection(std::string&);
        void parse_command(std::string&);
        
        bool receiver();

        void send_message(Message*);

        ~Client();
};

#endif