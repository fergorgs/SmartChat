#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHServer.h"
#endif

#include "Connection.h"

Message* Connection::read(int& ret) {
    auto now = std::chrono::system_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::seconds>(now -  last_timestamp).count();
    if(!message_queue.empty() && dur > 1) {
        if(retryCount++ == 5) {
            ret = 0;
            return nullptr;
        }
        
        write(nullptr);
    }
    char m[4096];
    memset(m, 0, 4096);

    int peek = recv(socket, m,  4096 * sizeof(char), MSG_PEEK);

    if (peek != 4096) {
        if (peek > 0)
            std::cout << "PEEKING" << std::endl;

        ret = -1;
        errno = EAGAIN;
        return nullptr;
    }

    ret = recv(socket, m, 4096 * sizeof(char), 0);
    std::cout << "RECVING" << std::endl;


    std::string s(m);

    Message* msg = nullptr;
    if(ret > 0) 
        msg = new Message(std::ref(s));

    return msg;
}

void Connection::write(MessageSendController* msg) {
    if(msg) message_queue.push(msg);
    if(!msg || message_queue.size() == 1) {
        send(socket, message_queue.front()->getBuffer(),  4096 * sizeof(char), 0);
        last_timestamp = std::chrono::system_clock::now();
    }
}

void Connection::confirmReceive() {
    retryCount = 0;
    message_queue.front()->deduct();
    message_queue.pop();
}

Connection::Connection(int s, std::string ip) : socket(s), ip_addr(ip), cur_channel(nullptr), file_size(-1), file_name() {
    std::cout << "Connection constructed\n";
    fcntl(socket, F_SETFL, O_NONBLOCK);
}; 

// void Connection::pong() {
   
// }

void Connection::send_msg(Message* m) {
    send(socket, m->serializeMessage().c_str(),  4096 * sizeof(char), 0);
}

Connection::~Connection() {
    if(cur_file && cur_file.is_open()) cur_file.close();
    while(!message_queue.empty()) message_queue.front()->deduct(), message_queue.pop();
    shutdown(socket, SHUT_RDWR);
    std::cout << "Connection destructed\n";
}
