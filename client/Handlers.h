#ifndef HANDLERS_H
#define HANDLERS_H

#include "Client.h"
#include "../utils/RFCprotocol.h"

namespace Handlers {
    // COMMAND REPLIES

    void channel_join_handler(Client* client, Message* msg);        // JOIN
    void kicked_from_channel_handler(Client* client, Message* msg); // KICK
    void mute_handler(Client* client, Message* msg);                // MUTE
    void mute_warning_handler(Client* client, Message* msg);        // MUTEWARN
    void nickname_change_handler(Client* client, Message* msg);     // NICK
    void pong_handler(Client* client, Message* msg);                // PONG
    void message_handler(Client* client, Message* msg);             // SAY
    void unmute_handler(Client* client, Message* msg);              // UNMUTE
    void unmute_warning_handler(Client* client, Message* msg);      // UNMUTEWARN
    void init_download_handler(Client* client, Message* msg);       // INIT_DOWN
    void file_contents_handler(Client* client, Message* msg);       // FILE_CONTENTS

    // NUMERIC REPLIES

    void who_is_response_handler(Client* client, Message* msg);     // 311

    // NUMERIC REPLIES: ERRORS
    
    void no_such_nick_handler(Client* client, Message* msg);        // 401
    void no_such_channel_handler(Client* client, Message* msg);     // 403
    void no_nick_given_handler(Client* client, Message* msg);       // 431
    void nickname_in_use_handler(Client* client, Message* msg);     // 433
    void user_not_in_channel_handler(Client* client, Message* msg); // 441
    void not_on_channel_handler(Client* client, Message* msg);      // 442
    void need_more_params_handler(Client* client, Message* msg);    // 461
    void bad_channel_mask_handler(Client* client, Message* msg);    // 476
    void channel_op_needed_handler(Client* client, Message* msg);   // 482
    void operation_denied_handler(Client* client, Message* msg);    // 700
    void download_error_handler(Client* client, Message* msg);      // 422
}

#endif