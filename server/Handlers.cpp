#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHServer.h"
#endif

#include "Handlers.h"
#include <random>

string get_uuid() {
    static random_device dev;
    static mt19937 rng(dev());

    uniform_int_distribution<int> dist(0, 15);

    const char *v = "0123456789abcdef";
    const bool dash[] = { 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0 };

    string res;
    for (int i = 0; i < 16; i++) {
        if (dash[i]) res += "-";
        res += v[dist(rng)];
        res += v[dist(rng)];
    }
    return res;
}

void broadcast_on_channel(Message* message, Channel* channel) {
    std::size_t members = channel->members.size();

    MessageSendController* control = new MessageSendController(members);
    control->setBuffer(message->serializeMessage());
    int i = 0;
    for (auto& recipient : channel->members) {
        if (i++ < members) {
            recipient->write(control);
        } else {
            break;
        }
    }
}

void Handlers::say(Message* message, Hub* server, Connection* sender) {

    if (sender->nick != message->prefix.getNick()) {
        Message* no_nick = new Message();

        no_nick->command.set_cmd("431");
        no_nick->params.setTrailing("No nickname given.");

        sender->send_msg(no_nick);

        delete no_nick;

        return;
    }

    if (sender->cur_channel == nullptr) 
        return;

    bool senderIsMuted = false;

    for (auto& memb : sender->cur_channel->mutedMembers) {
        if(memb == sender) {
            senderIsMuted = true;
            break;
        }
    }

    if (!senderIsMuted) {
        broadcast_on_channel(message, sender->cur_channel);
    }
}

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

void Handlers::file_upload(Message* message, Hub* server, Connection* sender) {
    if(sender->file_size > 0) {
        Message* error = new Message();
        error->command.set_cmd("700");
        error->params.setTrailing("Upload denied - a file upload is already in progress.");
        sender->send_msg(error);
        delete error;
        return;
    }

    sender->file_size = stoi(message->params.getMiddleContent()[0]);
    string filename = message->params.getTrailing();
    int i = filename.size() - 1;
    while(i >= 0 && filename[i] != '.') --i;
    string file_id = get_uuid() +  (i > 0 ? filename.substr(i) : "");

    create_directory_if_null("files");

    string id = "files/" + file_id;
    fstream file(id, ios::out | ios::binary);
    sender->cur_file = std::move(file);
    sender->written_size = 0;
    sender->file_name = file_id;
}


void Handlers::file_contents(Message* message, Hub* server, Connection* sender) {

    if (sender->nick != message->prefix.getNick()) {
        Message* no_nick = new Message();

        no_nick->command.set_cmd("431");
        no_nick->params.setTrailing("No nickname given.");

        sender->send_msg(no_nick);

        delete no_nick;

        return;
    }

    if (sender->cur_channel == nullptr) 
        return;

    // TODO: HANDLE FILE CONTENT
    message->params.getTrailing();

    int offset = stoi(message->params.getMiddleContent()[0]);

    sender->cur_file.seekp(offset);
    sender->cur_file.write(message->params.getTrailing().c_str(), message->params.getTrailing().size());

    sender->written_size += message->params.getTrailing().size();

    if(sender->written_size >= sender->file_size) {
        sender->file_size = -1;
        sender->cur_file.close();

        Message* file_recv_msg = new Message();
        file_recv_msg->command.set_cmd("SAY");
        file_recv_msg->prefix.setNick("Server");
        file_recv_msg->params.setTrailing(sender->nick + " uploaded file: \"" + sender->file_name + "\" successfully");
        
        broadcast_on_channel(file_recv_msg, sender->cur_channel);
    }
}

void Handlers::file_download(Message* message, Hub* server, Connection* sender) {

    string filename = "files/" + message->params.getTrailing();


    if(!filesystem::exists(filename)) {
        Message* error = new Message();
        error->command.set_cmd("422");
        error->params.addMiddleParam("ERR_FILE_DOWNLOAD");
        error->params.setTrailing("File does not exist.");

        sender->send_msg(error);
        return;
    }

    ifstream file(filename, ios::binary);

    if (!file) {
        Message* error = new Message();
        error->command.set_cmd("422");
        error->params.addMiddleParam("ERR_FILE_DOWNLOAD");
        error->params.setTrailing("Erro ao abrir o arquivo \"" + filename + "\".");

        sender->send_msg(error);
        return;
    }

    file.seekg(0, ios::end);

    std::size_t file_size = file.tellg();

    file.seekg(0, ios::beg);

    Message* msg = new Message();

    msg->prefix.setNick(sender->nick);
    msg->command.set_cmd("INIT_DOWNLOAD");
    msg->params.addMiddleParam(to_string(file_size));
    msg->params.setTrailing(filename);

    sender->send_msg(msg);

    delete msg;

    char file_buffer[FILE_UPLOAD_RATE + 1];
    memset(file_buffer, 0, sizeof(file_buffer));
    for (int i = 0; i * FILE_UPLOAD_RATE < file_size; i++) {
        Message* msg = new Message();

        msg->prefix.setNick(sender->nick);
        msg->command.set_cmd("FILE_CONTENTS");
        msg->params.addMiddleParam(to_string(i * FILE_UPLOAD_RATE)); // offset

        file.read(file_buffer, FILE_UPLOAD_RATE);
        string string_buffer (file_buffer, FILE_UPLOAD_RATE);

        msg->params.setTrailing(string_buffer);

        sender->send_msg(msg);

        delete msg;
        memset(file_buffer, 0, sizeof(file_buffer));
    }
   
}

void Handlers::ping(Message* message, Hub* server, Connection* sender) {
    Message* pong = new Message();
    
    pong->command.set_cmd("PONG");

    sender->send_msg(pong);

    delete pong;
}

void Handlers::nick(Message* message, Hub* server, Connection* sender) {
    std::vector<std::string> params = message->params.getMiddleContent();
    
    if (params.size() == 1) {
        std::string new_nick = params[0];

        if(server->nicks.find(new_nick) != server->nicks.end()) {
            Message* nick_in_use = new Message();

            nick_in_use->command.set_cmd("433");
            nick_in_use->params.addMiddleParam(new_nick);
            nick_in_use->params.setTrailing("Nickname is already in use.");

            sender->send_msg(nick_in_use);

            delete nick_in_use;

        } else {
            if (sender->nick != message->prefix.getNick()) {
                Message* no_nick = new Message();

                no_nick->command.set_cmd("431");
                no_nick->params.setTrailing("No nickname given.");

                sender->send_msg(no_nick);

                delete no_nick;

                return;
            }

            if(sender->nick.size() != 0) 
                server->nicks.erase(sender->nick);
            
            server->nicks[new_nick] = sender;
            sender->nick = new_nick;
            
            sender->send_msg(message);
        }
    } else {
        Message* need_params = new Message();
        need_params->command.set_cmd("461");
        need_params->params.addMiddleParam(message->command.get_id());
        need_params->params.setTrailing("Not enough parameters.");

        sender->send_msg(need_params);

        delete need_params;
    }
}

void Handlers::confirm(Message* message, Hub* server, Connection* sender) {
    sender->confirmReceive();
}

void Handlers::join(Message* message, Hub* server, Connection* sender) {
    if (sender->nick != message->prefix.getNick()) {
        Message* no_nick = new Message();

        no_nick->command.set_cmd("431");
        no_nick->params.setTrailing("No nickname given.");

        sender->send_msg(no_nick);

        delete no_nick;

        return;
    }

    std::vector<std::string> params = message->params.getMiddleContent();

    if (params.size() == 1) {
        std::string name = params[0];
        
        if (name.size() != 0 && name[0] != '#') {
            Message* bad_mask = new Message();

            bad_mask->command.set_cmd("476");
            bad_mask->params.addMiddleParam(name);
            bad_mask->params.setTrailing("Bad channel mask.");
            
            sender->send_msg(bad_mask);

            delete bad_mask;

        } else {
            Channel* channel = sender->cur_channel;

            if (channel) {
                channel->remove(sender);
                
                if (channel->members.empty()) {
                    server->channels.erase(channel->name);
                    delete channel;
                }
            }
            
            if(server->channels.find(name) == server->channels.end()) 
                server->channels[name] = new Channel(name, sender);

            server->channels[name]->connect(sender);
            

            sender->send_msg(message);
        }
    } else {
        Message* need_params = new Message();
        need_params->command.set_cmd("461");
        need_params->params.addMiddleParam(message->command.get_id());
        need_params->params.setTrailing("Not enough parameters.");

        sender->send_msg(need_params);

        delete need_params;
    }
}

void Handlers::kick(Message* message, Hub* server, Connection* sender) {
    std::vector<std::string> params = message->params.getMiddleContent();

    if (params.size() == 2) {
        std::string ch_name = params[0];
        std::string nick = params[1];

        if (ch_name.size() == 0 || (ch_name.size() != 0 && ch_name[0] != '#')) {
            Message* bad_mask = new Message();

            bad_mask->command.set_cmd("476");
            bad_mask->params.addMiddleParam(ch_name);
            bad_mask->params.setTrailing("Bad channel mask.");
            
            sender->send_msg(bad_mask);

            delete bad_mask;

            return;
        }

        if (server->channels.count(ch_name) == 0) {
            Message* no_channel = new Message();

            no_channel->command.set_cmd("403");
            no_channel->params.addMiddleParam(ch_name);
            no_channel->params.setTrailing("No such channel.");

            sender->send_msg(no_channel);

            delete no_channel;

            return;
        }

        Channel* channel = server->channels[ch_name];

        if (channel != sender->cur_channel) {
            Message* not_on_channel = new Message();

            not_on_channel->command.set_cmd("442");
            not_on_channel->params.addMiddleParam(ch_name);
            not_on_channel->params.setTrailing("You're not on that channel.");

            sender->send_msg(not_on_channel);

            delete not_on_channel;

            return;
        }

        if(channel != nullptr && channel->admin == sender) {

            Connection* kicked = channel->find(nick);

            if (kicked != nullptr) {
                channel->remove(kicked);

                if (channel->members.empty()) {
                    server->channels.erase(channel->name);
                    delete channel;
                }

                Message* kick = new Message();

                kick->command.set_cmd("KICK");
                kick->params.addMiddleParam(ch_name);
                kick->params.setTrailing(message->params.getTrailing());

                kicked->send_msg(kick);

                delete kick;
            } else {
                Message* not_in_channel = new Message();
                not_in_channel->command.set_cmd("441");
                not_in_channel->params.addMiddleParam(nick);
                not_in_channel->params.addMiddleParam(ch_name);
                not_in_channel->params.setTrailing("They aren't on that channel.");

                sender->send_msg(not_in_channel);

                delete not_in_channel;
            }
        } else {
            Message* error_op = new Message();

            error_op->command.set_cmd("482");
            error_op->params.addMiddleParam(ch_name);
            error_op->params.setTrailing("You're not channel operator.");
            
            sender->send_msg(error_op);

            delete error_op;
        }
    } else {
        Message* need_params = new Message();
        need_params->command.set_cmd("461");
        need_params->params.addMiddleParam(message->command.get_id());
        need_params->params.setTrailing("Not enough parameters.");

        sender->send_msg(need_params);

        delete need_params;
    }
}

void Handlers::whois(Message* message, Hub* server, Connection* sender) {
    std::vector<std::string> params = message->params.getMiddleContent();

    if (params.size() == 1) {
        if (sender->cur_channel && sender->cur_channel->admin == sender) {
            std::string nick = params[0];

            Connection* who = sender->cur_channel->find(nick);

            if (who != nullptr) {
                Message* who_user = new Message();

                who_user->command.set_cmd("311");
                who_user->params.addMiddleParam(who->nick);
                who_user->params.addMiddleParam(who->ip_addr);

                sender->send_msg(who_user);

                delete who_user;
            } else {
                Message* no_nick = new Message();

                no_nick->command.set_cmd("401");
                no_nick->params.addMiddleParam(nick);
                no_nick->params.setTrailing("No such nick.");
            }
            
        } else {
            Message* error_op = new Message();

            error_op->command.set_cmd("482");
            error_op->params.addMiddleParam(sender->cur_channel->name);
            error_op->params.setTrailing("You're not channel operator.");

            sender->send_msg(error_op);

            delete error_op;
        }
    } else {
        Message* need_params = new Message();
        need_params->command.set_cmd("461");
        need_params->params.addMiddleParam(message->command.get_id());
        need_params->params.setTrailing("Not enough parameters.");

        sender->send_msg(need_params);

        delete need_params;
    }
}

/*
    There is no doc in RFC for mute message in IRC.
    So, we created our own doc, following kick description.

    Mute command
        Command: MUTE
        Parameters: <channel> <nick> [<comment>]

    Numeric Replies:

        ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
        ERR_BADCHANMASK                 ERR_CHANOPRIVSNEEDED
        ERR_NOTONCHANNEL

    Examples:

        MUTE #Russian John :Only Russian is allowed
        MUTE #Games noobmaster69 
*/
void Handlers::mute(Message* message, Hub* server, Connection* sender) {
    std::vector<std::string> params = message->params.getMiddleContent();

    if (params.size() == 2) {
        std::string ch_name = params[0];
        std::string nick = params[1];

        if (ch_name.size() == 0 || (ch_name.size() != 0 && ch_name[0] != '#')) {
            Message* bad_mask = new Message();

            bad_mask->command.set_cmd("476");
            bad_mask->params.addMiddleParam(ch_name);
            bad_mask->params.setTrailing("Bad channel mask.");
            
            sender->send_msg(bad_mask);

            delete bad_mask;

            return;
        }

        if (server->channels.count(ch_name) == 0) {
            Message* no_channel = new Message();

            no_channel->command.set_cmd("403");
            no_channel->params.addMiddleParam(ch_name);
            no_channel->params.setTrailing("No such channel.");

            sender->send_msg(no_channel);

            delete no_channel;

            return;
        }

        Channel* channel = server->channels[ch_name];

        if (channel != sender->cur_channel) {
            Message* not_on_channel = new Message();

            not_on_channel->command.set_cmd("442");
            not_on_channel->params.addMiddleParam(ch_name);
            not_on_channel->params.setTrailing("You're not on that channel.");

            sender->send_msg(not_on_channel);

            delete not_on_channel;

            return;
        }

        if(channel != nullptr && channel->admin == sender) {

            Connection* muted = channel->find(nick);

            if (muted != nullptr) {
                channel->mute(muted);

                Message* feedback = new Message();

                feedback->command.set_cmd("MUTE");
                feedback->params.addMiddleParam(nick);

                // broadcast to channel (maybe?)
                sender->send_msg(feedback);

                delete feedback;

                Message* warning = new Message();
                
                warning->command.set_cmd("MUTEWARN");
                warning->params.setTrailing(message->params.getTrailing());

                muted->send_msg(warning);

                delete warning;
            } else {
                Message* not_in_channel = new Message();
                not_in_channel->command.set_cmd("441");
                not_in_channel->params.addMiddleParam(nick);
                not_in_channel->params.addMiddleParam(ch_name);
                not_in_channel->params.setTrailing("They aren't on that channel.");

                sender->send_msg(not_in_channel);

                delete not_in_channel;
            }
        } else {
            Message* error_op = new Message();

            error_op->command.set_cmd("482");
            error_op->params.addMiddleParam(ch_name);
            error_op->params.setTrailing("You're not channel operator.");
            
            sender->send_msg(error_op);

            delete error_op;
        }
    } else {
        Message* need_params = new Message();
        need_params->command.set_cmd("461");
        need_params->params.addMiddleParam(message->command.get_id());
        need_params->params.setTrailing("Not enough parameters.");

        sender->send_msg(need_params);

        delete need_params;
    }
}

/*
    There is no doc in RFC for unmute message in IRC.
    So, we created our own doc, following kick description.

    Unmute command
        Command: UNMUTE
        Parameters: <channel> <nick>

    Numeric Replies:

        ERR_NEEDMOREPARAMS              ERR_NOSUCHCHANNEL
        ERR_BADCHANMASK                 ERR_CHANOPRIVSNEEDED
        ERR_NOTONCHANNEL

    Examples:

        UNMUTE #Russian John
        UNMUTE #Games noobmaster69 
*/ 
void Handlers::unmute(Message* message, Hub* server, Connection* sender) {
    std::vector<std::string> params = message->params.getMiddleContent();

    if (params.size() == 2) {
        std::string ch_name = params[0];
        std::string nick = params[1];

        if (ch_name.size() == 0 || (ch_name.size() != 0 && ch_name[0] != '#')) {
            Message* bad_mask = new Message();

            bad_mask->command.set_cmd("476");
            bad_mask->params.addMiddleParam(ch_name);
            bad_mask->params.setTrailing("Bad channel mask.");
            
            sender->send_msg(bad_mask);

            delete bad_mask;

            return;
        }

        if (server->channels.count(ch_name) == 0) {
            Message* no_channel = new Message();

            no_channel->command.set_cmd("403");
            no_channel->params.addMiddleParam(ch_name);
            no_channel->params.setTrailing("No such channel.");

            sender->send_msg(no_channel);

            delete no_channel;

            return;
        }

        Channel* channel = server->channels[ch_name];

        if (channel != sender->cur_channel) {
            Message* not_on_channel = new Message();

            not_on_channel->command.set_cmd("442");
            not_on_channel->params.addMiddleParam(ch_name);
            not_on_channel->params.setTrailing("You're not on that channel.");

            sender->send_msg(not_on_channel);

            delete not_on_channel;

            return;
        }

        if(channel != nullptr && channel->admin == sender) {

            Connection* unmuted = channel->find(nick);

            if (unmuted != nullptr) {
                channel->unmute(unmuted);

                Message* feedback = new Message();

                feedback->command.set_cmd("UNMUTE");
                feedback->params.addMiddleParam(nick);

                sender->send_msg(feedback);

                delete feedback;

                Message* warning = new Message();
                
                warning->command.set_cmd("UNMUTEWARN");
                warning->params.setTrailing(message->params.getTrailing());

                unmuted->send_msg(warning);

                delete warning;
            } else {
                Message* not_in_channel = new Message();
                not_in_channel->command.set_cmd("441");
                not_in_channel->params.addMiddleParam(nick);
                not_in_channel->params.addMiddleParam(ch_name);
                not_in_channel->params.setTrailing("They aren't on that channel.");

                sender->send_msg(not_in_channel);

                delete not_in_channel;
            }
        } else {
            Message* error_op = new Message();

            error_op->command.set_cmd("482");
            error_op->params.addMiddleParam(ch_name);
            error_op->params.setTrailing("You're not channel operator.");
            
            sender->send_msg(error_op);

            delete error_op;
        }
    } else {
        Message* need_params = new Message();
        need_params->command.set_cmd("461");
        need_params->params.addMiddleParam(message->command.get_id());
        need_params->params.setTrailing("Not enough parameters.");

        sender->send_msg(need_params);

        delete need_params;
    }
}