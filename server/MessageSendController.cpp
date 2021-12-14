#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHServer.h"
#endif

#include "MessageSendController.h"

void MessageSendController::deduct() {
    // if(clients == 1) ;
    if(!--clients) delete this;
}

const char* MessageSendController::getBuffer() {
    return buffer.c_str();
}

void MessageSendController::setBuffer(std::string b) {
    buffer.assign(b.c_str());
}

MessageSendController::MessageSendController(int c) : clients(c) { 
} 

MessageSendController::~MessageSendController() {
    std::cout << "message destructed\n";
}