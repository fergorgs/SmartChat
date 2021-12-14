#ifndef HANDLERS_H
#define HANDLERS_H

#include "Connection.h"
#include "Channel.h"
#include "MessageSendController.h"
#include "../utils/RFCprotocol.h"
#include "Hub.h"

#define MAX_FILE_SIZE 8192
#define FILE_UPLOAD_RATE 1024

class Hub;

namespace Handlers {
    void say(Message*, Hub*, Connection*);
    void file_upload(Message*, Hub*, Connection*);
    void file_contents(Message*, Hub*, Connection*);
    void file_download(Message*, Hub*, Connection*);
    void ping(Message*, Hub*, Connection*);
    void nick(Message*, Hub*, Connection*);
    void confirm(Message*, Hub*, Connection*);
    void join(Message*, Hub*, Connection*);
    void kick(Message*, Hub*, Connection*);
    void whois(Message*, Hub*, Connection*);
    void mute(Message*, Hub*, Connection*);
    void unmute(Message*, Hub*, Connection*);
};

#endif // HANDLERS_H