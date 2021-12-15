#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHClient.h"
#endif

#include "Handlers.h"
#include "ui/Screen.h"
#include <filesystem>

bool create_directory_if_null(std::string path) {
    filesystem::path dir_path = filesystem::path(path);
    
    if (filesystem::exists(dir_path))
        return true;
    
    if (filesystem::create_directory(dir_path))
    {
        cout << "Created directory at " << dir_path << endl;
        return true;
    }

    return false;
}

/*
    ======================================================
    ||                 COMMAND REPLIES                  ||
    ======================================================
*/

/* Reply JOIN */
void Handlers::channel_join_handler(Client* client, Message* msg) {
    client->channel = msg->params.getMiddleContent()[0];

    Screen::log_message("Joined channel <b>" + client->channel + "</b>.", Screen::LogType::WARNING);
}

/* Reply KICK */
void Handlers::kicked_from_channel_handler(Client* client, Message* msg) {
    std::string reason = msg->params.getTrailing();
    std::string channel = msg->params.getMiddleContent()[0];

    Screen::log_message("You have been kicked from " + channel + ": " + reason, Screen::LogType::WARNING);

    client->channel = "";
}

/* Reply MUTE */
void Handlers::mute_handler(Client* client, Message* msg) {
    std::string nick = msg->params.getMiddleContent()[0];

    Screen::log_message(nick + " has been muted.", Screen::LogType::WARNING);
}

/* Reply MUTEWARN */
void Handlers::mute_warning_handler(Client* client, Message* msg) {
    std::string reason = msg->params.getTrailing();

    Screen::log_message("You have been muted: " + reason, Screen::LogType::WARNING);
}

/* Reply NICK */
void Handlers::nickname_change_handler(Client* client, Message* msg) {
    client->nickname = msg->params.getMiddleContent()[0];

    Screen::log_message("Nickname changed to <b>" + client->nickname + "</b>.", Screen::LogType::WARNING);
}

/* Reply PONG */
void Handlers::pong_handler(Client* client, Message* msg) {
    std::string time = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(client->pong_time - client->ping_time).count());

    Screen::log_message(time + "ms until PONG", Screen::LogType::WARNING);
}

/* Reply SAY */
void Handlers::message_handler(Client* client, Message* msg) {
    std::string nick = msg->prefix.getNick();
    std::string trail = msg->params.getTrailing();
        
    Screen::add_message(std::ref(trail), std::ref(nick));

    // ACK MSG Received
    Message* response = new Message();
    response->command.set_cmd("ACK");
    response->prefix.setNick(client->nickname);

    client->send_message(response);
    
    delete response;
}

/* Reply UNMUTE */
void Handlers::unmute_handler(Client* client, Message* msg) {
    std::string nick = msg->params.getMiddleContent()[0];

    Screen::log_message(nick + " has been unmuted.", Screen::LogType::WARNING);
}

/* Reply UNMUTEWARN */
void Handlers::unmute_warning_handler(Client* client, Message* msg) {
    Screen::log_message("You have been unmuted.", Screen::LogType::WARNING);
}

/* INIT_DOWNLOAD */
void Handlers::init_download_handler(Client* client, Message* msg) {

    client->file_size = stoi(msg->params.getMiddleContent()[0]);
    client->downloaded_file_size = 0;

    create_directory_if_null("medias");
    fstream file("medias/" + client->download_file_name, ios::out | ios::binary);
    client->cur_file = std::move(file);
    Screen::log_message("Download started...", Screen::LogType::WARNING);
}

/* FILE_CONTENTS */
void Handlers::file_contents_handler(Client* client, Message* msg) {

    if(!client->downloading) return;

    int offset = stoi(msg->params.getMiddleContent()[0]);

    client->cur_file.seekp(offset);
    client->cur_file.write(msg->params.getTrailing().c_str(), msg->params.getTrailing().size());

    client->downloaded_file_size += msg->params.getTrailing().size();

    if(client->downloaded_file_size >= client->file_size) {
        Screen::log_message("Download completed at medias/" + client->download_file_name, Screen::LogType::SUCCESS);

        client->file_size = -1;
        client->cur_file.close();
        client->download_file_name = "";
        client->downloading = false;
    }
    else {
        stringstream ss;
        ss.precision(2);
        ss << fixed << "Download at " << (client->downloaded_file_size / (float)client->file_size) * 100  << "%";
        Screen::log_message(ss.str(), Screen::LogType::SUCCESS);
    }
}



/*
    ======================================================
    ||                 NUMERIC REPLIES                  ||
    ======================================================
*/

/* No 311 */
void Handlers::who_is_response_handler(Client* client, Message* msg) {
    std::vector<std::string> params = msg->params.getMiddleContent();

    std::string nick = params[0];
    std::string ip_addr = params[1];

    Screen::log_message("User <b>" + nick + "</b> [ IP: " + ip_addr + " ]", Screen::LogType::WARNING);
}



/*
    ======================================================
    ||                  ERROR REPLIES                   ||
    ======================================================
*/

/* No 401 */ 
void Handlers::no_such_nick_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string nick = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + nick + ")", Screen::LogType::ERROR);
}

/* No 403 */ 
void Handlers::no_such_channel_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string name = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + name + ")", Screen::LogType::ERROR);
}

//  No 422
void Handlers::download_error_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string name = msg->params.getMiddleContent()[0];

    client->downloading = false;
    client->download_file_name = "";

    Screen::log_message(command + ": " + reason + " (" + name + ")", Screen::LogType::ERROR);
}

/* No 431 */
void Handlers::no_nick_given_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();

    Screen::log_message(command + ": " + reason, Screen::LogType::ERROR);
}

/* No 433 */ 
void Handlers::nickname_in_use_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string nick = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + nick + ")", Screen::LogType::ERROR);
}

/* No 441 */
void Handlers::user_not_in_channel_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string nick = msg->params.getMiddleContent()[0];
    std::string channel = msg->params.getMiddleContent()[1];

    Screen::log_message(command + ": " + reason + " (" + nick + " in " + channel + ")", Screen::LogType::ERROR);
} 

/* No 442 */ 
void Handlers::not_on_channel_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string name = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + name + ")", Screen::LogType::ERROR);
}

/* No 461 */ 
void Handlers::need_more_params_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string command_given = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + command_given + ")", Screen::LogType::ERROR);
}

/* No 476 */ 
void Handlers::bad_channel_mask_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string name = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + name + ")", Screen::LogType::ERROR);
}

/* No 482 */ 
void Handlers::channel_op_needed_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string name = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + name + ")", Screen::LogType::ERROR);
}

/* No 700 */ 
void Handlers::operation_denied_handler(Client* client, Message* msg) {
    std::string command = msg->command.get_id();
    std::string reason = msg->params.getTrailing();
    std::string name = msg->params.getMiddleContent()[0];

    Screen::log_message(command + ": " + reason + " (" + name + ")", Screen::LogType::ERROR);
}