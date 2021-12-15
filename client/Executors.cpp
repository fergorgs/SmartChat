#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHClient.h"
#endif

#include "Executors.h"

#include "../utils/RFCprotocol.h"
#include "ui/Screen.h"

#define MSG_MAX_SIZE 4000
#define MAX_FILE_SIZE 8192
#define FILE_UPLOAD_RATE 1024

void Executors::connect_executor(Client* client, std::string& text) {
    if (client->connected) {
        Screen::log_message("You are already connected.", Screen::LogType::ERROR);
        return;
    }

    std::size_t arg1 = text.find(" ");
    std::string server = text.substr(0, arg1);

    client->create_connection(std::ref(server));
}

void Executors::file_upload_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->nickname.empty()) {
        Screen::log_message("Select your nickname first (/nick [nick]).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/upload [filepath]).", Screen::LogType::ERROR);
        return;
    }

    ifstream file(text, ios::binary);

    if (!file) {
        Screen::log_message("Erro ao abrir o arquivo '" + text + "'.", Screen::LogType::ERROR);
        return;
    }

    file.seekg(0, ios::end);

    std::size_t file_size = file.tellg();

    if (file_size > MAX_FILE_SIZE) {
        Screen::log_message("O arquivo '" + text + "' tem tamanho " + to_string(file_size) + " bytes. Isso é mais do que " + to_string(MAX_FILE_SIZE) + " bytes permitidos.", Screen::LogType::ERROR);
        return;
    }

    file.seekg(0, ios::beg);

    Message* msg = new Message();

    msg->prefix.setNick(client->nickname);
    msg->command.set_cmd("FILE_UPLOAD");
    msg->params.addMiddleParam(to_string(file_size));
    msg->params.setTrailing(text);

    client->send_message(msg);

    delete msg;

    char file_buffer[FILE_UPLOAD_RATE + 1];
    memset(file_buffer, 0, sizeof(file_buffer));
    for (int i = 0; i * FILE_UPLOAD_RATE < file_size; i++) {
        Message* msg = new Message();

        msg->prefix.setNick(client->nickname);
        msg->command.set_cmd("FILE_CONTENTS");
        msg->params.addMiddleParam(to_string(i * FILE_UPLOAD_RATE)); // offset

        file.read(file_buffer, FILE_UPLOAD_RATE);
        string string_buffer (file_buffer, FILE_UPLOAD_RATE);

        msg->params.setTrailing(string_buffer);

        client->send_message(msg);

        delete msg;
        memset(file_buffer, 0, sizeof(file_buffer));
    }

}

void Executors::file_download_executor(Client* client, std::string& text) {
    if(client->downloading) {
        Screen::log_message("A download is already in progress.", Screen::LogType::ERROR);
        return;
    }

    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->nickname.empty()) {
        Screen::log_message("Select your nickname first (/nick [nick]).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/download [file_id]).", Screen::LogType::ERROR);
        return;
    }

    client->downloading = true;
    client->download_file_name = text;

    Message* msg = new Message();

    msg->prefix.setNick(client->nickname);
    msg->command.set_cmd("FILE_DOWNLOAD");
    msg->params.setTrailing(text);

    client->send_message(msg);
}

void Executors::join_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    std::size_t arg1 = text.find(" ");
    std::string channel = text.substr(0, arg1);

    if (channel.size() < 2 || (channel.size() >= 2 && channel[0] != '#')) {
        Screen::log_message("Channel must be in #[name] format.", Screen::LogType::ERROR);
        return;
    }

    Message* join = new Message();

    join->prefix.setNick(client->nickname);
    join->command.set_cmd("JOIN");
    join->params.addMiddleParam(channel);

    client->send_message(join);

    delete join;
}

void Executors::kick_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }
    
    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/kick [nick] { [reason] }).", Screen::LogType::ERROR);
        return;
    }

    std::size_t arg1 = text.find(" ");
    std::string nick = text.substr(0, arg1);

    Message* kick = new Message();

    kick->command.set_cmd("KICK");
    kick->params.addMiddleParam(client->channel);
    kick->params.addMiddleParam(nick);

    if (arg1 != std::string::npos) {
        std::string reason = text.substr(arg1 + 1);
        kick->params.setTrailing(reason);
    }

    client->send_message(kick);

    delete kick;
}

void Executors::mute_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/mute [nick] { [reason] }).", Screen::LogType::ERROR);
        return;
    }

    std::size_t arg1 = text.find(" ");
    std::string nick = text.substr(0, arg1);

    Message* mute = new Message();

    mute->command.set_cmd("MUTE");
    mute->params.addMiddleParam(client->channel);
    mute->params.addMiddleParam(nick);

    if (arg1 != std::string::npos) {
        std::string reason = text.substr(arg1 + 1);
        mute->params.setTrailing(reason);
    }

    client->send_message(mute);

    delete mute;
}

void Executors::nick_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("Connect to server to change your nick (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/nick [nick]).", Screen::LogType::ERROR);
        return;
    }

    size_t arg1 = text.find(" ");
    std::string new_nick = text.substr(0, arg1);

    if (new_nick.size() > 20 || new_nick.size() < 3) {
        Screen::log_message("Nickname must have more than 2 chars and less than 21 chars.", Screen::LogType::ERROR);
        return;
    }

    Message* msg = new Message();

    if (client->nickname.size() != 0)
        msg->prefix.setNick(client->nickname);

    msg->command.set_cmd("NICK");
    msg->params.addMiddleParam(new_nick);

    client->send_message(msg);

    delete msg;
}

void Executors::ping_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    Message* msg = new Message();

    msg->prefix.setNick(client->nickname);
    msg->command.set_cmd("PING");
    msg->params.setTrailing("Hello from earth"); // LOL

    client->send_message(msg);

    delete msg;
}

void Executors::quit_executor(Client* client, std::string& text) {
    client->quit();
}

void Executors::say_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->nickname.empty()) {
        Screen::log_message("Select your nickname first (/nick [nick]).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    for (int i = 0; i * MSG_MAX_SIZE < text.size(); i++) {
        Message* msg = new Message();

        msg->prefix.setNick(client->nickname);
        msg->command.set_cmd("SAY");
        msg->params.setTrailing(text.substr(i * MSG_MAX_SIZE, MSG_MAX_SIZE));

        client->send_message(msg);

        delete msg;
    }
}

void Executors::who_is_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    Message* whois = new Message();

    whois->command.set_cmd("WHOIS");
    whois->params.addMiddleParam(text);

    client->send_message(whois);

    delete whois;
}

void Executors::unmute_executor(Client* client, std::string& text) {
    if (!client->connected) {
        Screen::log_message("You need to be connected to do this (/connect).", Screen::LogType::ERROR);
        return;
    }

    if (client->channel.empty()) {
        Screen::log_message("Join a Channel first (/join [channel]).", Screen::LogType::ERROR);
        return;
    }

    if (text.empty()) {
        Screen::log_message("This command needs more parameters (/mute [nick] { [reason] }).", Screen::LogType::ERROR);
        return;
    }

    std::size_t arg1 = text.find(" ");
    std::string nick = text.substr(0, arg1);

    Message* unmute = new Message();

    unmute->command.set_cmd("UNMUTE");
    unmute->params.addMiddleParam(client->channel);
    unmute->params.addMiddleParam(nick);

    client->send_message(unmute);

    delete unmute;
}