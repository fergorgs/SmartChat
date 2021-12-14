#ifndef MESSAGESENDCONTROLLER_H
#define MESSAGESENDCONTROLLER_H

class MessageSendController {
    private:
        std::string buffer;
        int clients;
    public:
        const char* getBuffer();
        void setBuffer(std::string);
        void deduct();
        MessageSendController(int);
        ~MessageSendController();
};

#endif // MESSAGESENDCONTROLLER_H