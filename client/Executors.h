#ifndef EXECUTORS_H
#define EXECUTORS_H

#include "Client.h"

namespace Executors {
    void connect_executor(Client* client, std::string& text);
    void file_upload_executor(Client* client, std::string& text);
    void file_download_executor(Client* client, std::string& text);
    void join_executor(Client* client, std::string& text);
    void kick_executor(Client* client, std::string& text);
    void mute_executor(Client* client, std::string& text);
    void nick_executor(Client* client, std::string& text);
    void ping_executor(Client* client, std::string& text);
    void quit_executor(Client* client, std::string& text);
    void say_executor(Client* client, std::string& text);
    void unmute_executor(Client* client, std::string& text);
    void who_is_executor(Client* client, std::string& text);
}

#endif