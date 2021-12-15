#ifdef VS_HACK
    // This is not included on compilation, just in VS Code
    // to make IntelliSense work
    #include "PCHClient.h"
#endif

#include "Client.h"

#include "../utils/StrManip.h"
#include "Executors.h"
#include "Handlers.h"
#include "ui/Screen.h"

#define SERVER_DOOR 6667
#define LOG 1

void Client::reset() {
    channel = "";
    nickname = "";
    connected = false;
}

// Private functions
void Client::create_connection(std::string& ip_addr) {
    // Socket opening
    hub_socket = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in hub_address;
    hub_address.sin_family = AF_INET;

    if (ip_addr.empty())
        hub_address.sin_addr.s_addr = INADDR_ANY;
    else {
        struct addrinfo hints, *list = NULL;
        struct sockaddr_in *ip;

        memset(&hints, 0, sizeof hints);

        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if (getaddrinfo(ip_addr.c_str(), NULL, &hints, &list)) return;

        struct addrinfo *host;
        for (host = list; host != NULL; host = host->ai_next) {
            hub_address = *(struct sockaddr_in *)(host->ai_addr);
        }

        freeaddrinfo(list);

        if (LOG) std::cout << "ip: " << inet_ntoa(hub_address.sin_addr) << std::endl;
    }

    hub_address.sin_port = htons(SERVER_DOOR);

	int connection_status = connect(hub_socket, (struct sockaddr *) &hub_address, sizeof(hub_address));
	
	// If there is connection error, shutdown
	if (connection_status) {
        Screen::log_message("Failed to connect (" + to_string(errno) + ").", Screen::LogType::ERROR);
        if (LOG) std::cout << "CLIENT_LOG: Failed to connect to hub" << std::endl;
        return;
    }

	// Non-blocking I/O flag is set
    fcntl(hub_socket, F_SETFL, O_NONBLOCK);

    connected = true;

    Screen::log_message("You are connected.", Screen::LogType::SUCCESS);
    Screen::log_message("Use /join [channel] to join a channel.", Screen::LogType::WARNING);

    if (LOG) std::cout << "CLIENT_LOG: Connected to hub" << std::endl;
}

void Client::send_message(Message* msg) {
	std::string serialized_msg = msg->serializeMessage();

	if (LOG) std::cout << "CLIENT_LOG: Sending >" << serialized_msg << std::endl;

    if (msg->command.get_id() == "PING")
        ping_time = std::chrono::steady_clock::now();

    char clear_msg[4096];
    memset(clear_msg, 0, 4096);
    strncpy(clear_msg, serialized_msg.c_str(), 4096);

	send(hub_socket, clear_msg, 4096, 0);
}

void Client::parse_command(std::string& str) {
    if (str[0] == '/') {
        size_t arg0 = str.find(" ");
        std::string cmd = trim(str.substr(1, arg0));
        for (char& c : cmd) c = tolower(c); // to lower case

        std::string args = "";
        if (arg0 != std::string::npos) 
            args = trim(str.substr(arg0 + 1));

        if (executors.count(cmd) > 0)
            executors[cmd](this, std::ref(args));
        else
            Screen::log_message("It seems like this command doesn't exist.", Screen::LogType::ERROR);

    } else {
        executors["say"](this, str);
    }
}

// Function to reveive a message from the HUB (timeout handler, called each 100 milliseconds)
bool Client::receiver() {
    if (!connected) return true;

    while (true) {
		// if receive data from socket, write in screen
		char c_msg[4096];

        int peek = recv(hub_socket, c_msg,  4096 * sizeof(char), MSG_PEEK);

        if (peek != 4096) {
            if (peek > 0 && LOG)
                std::cout << "PEEKING" << std::endl;

            return true;
        }

		int res = recv(hub_socket, &c_msg, 4096, 0);
        
        std::chrono::steady_clock::time_point recv_time = std::chrono::steady_clock::now();

		std::string msg(c_msg);

		if (res > 0) {
			if (LOG) std::cout << "CLIENT_LOG: Message (" << msg << ")" << std::endl;

			Message* msg_obj = new Message(std::ref(msg));

            std::string cmd_id = msg_obj->command.get_id();
            for (char& c : cmd_id) c = tolower(c); // to lower case

            if (msg_obj->command.get_id() == "PONG")
                pong_time = recv_time;

            if (handlers.count(cmd_id) > 0)
                handlers[cmd_id](this, msg_obj);
            else
                Screen::log_message("Couldn't handle message received from server.", Screen::LogType::ERROR);

            delete msg_obj;
		} else {
			if (res == 0){
				if (LOG) std::cout << "CLIENT_LOG: Disconnected from HUB (" << to_string(errno) << ")." << std::endl;
                
                reset();

                Screen::log_message("Disconnected from server.", Screen::LogType::ERROR);
			} else if (res < 0 && errno != EAGAIN && errno != EWOULDBLOCK) 
				if (LOG) std::cout << "CLIENT_LOG: Error receiving message from hub (" << errno << ")" << std::endl; 

			break;
		}
	}
	return true;
}

void Client::quit() {
    if (connected) {
        shutdown(hub_socket, SHUT_RDWR);
        close(hub_socket);
    }

    delete Screen::window;

    if (LOG) std::cout << "CLIENT_LOG: Disconnected from HUB" << std::endl;
    if (LOG) std::cout << "CLIENT_LOG: Shutting Client down" << std::endl;

    exit(0);
}

Client::Client() {
    channel = "";
    nickname = "";

    ping_time = std::chrono::steady_clock::now();
    pong_time = std::chrono::steady_clock::now();

    connected = false;

    file_size = -1;
    downloading = false;
    download_file_name = "";

    // Setup Screen & Signals
    Screen::setup(this);

    // Setup executors
    executors["quit"] = Executors::quit_executor;
    executors["connect"] = Executors::connect_executor;
    executors["nick"] = Executors::nick_executor;
    executors["ping"] = Executors::ping_executor;
    executors["say"] = Executors::say_executor;
    executors["join"] = Executors::join_executor;
    executors["kick"] = Executors::kick_executor;
    executors["whois"] = Executors::who_is_executor;
    executors["mute"] = Executors::mute_executor;
    executors["unmute"] = Executors::unmute_executor;
    executors["upload"] = Executors::file_upload_executor;
    executors["download"] = Executors::file_download_executor;

    handlers["join"] = Handlers::channel_join_handler;
    handlers["kick"] = Handlers::kicked_from_channel_handler;
    handlers["mute"] = Handlers::mute_handler;
    handlers["mutewarn"] = Handlers::mute_warning_handler;
    handlers["nick"] = Handlers::nickname_change_handler;
    handlers["pong"] = Handlers::pong_handler;
    handlers["say"] = Handlers::message_handler;
    handlers["unmute"] = Handlers::unmute_handler;
    handlers["unmutewarn"] = Handlers::unmute_warning_handler;
    handlers["init_download"] = Handlers::init_download_handler;
    handlers["file_contents"] = Handlers::file_contents_handler;

    // numerics
    handlers["311"] = Handlers::who_is_response_handler;
    
    handlers["401"] = Handlers::no_such_nick_handler;
    handlers["403"] = Handlers::no_such_channel_handler;
    handlers["422"] = Handlers::download_error_handler;
    handlers["431"] = Handlers::no_nick_given_handler;
    handlers["433"] = Handlers::nickname_in_use_handler;
    handlers["441"] = Handlers::user_not_in_channel_handler;
    handlers["442"] = Handlers::not_on_channel_handler;
    handlers["461"] = Handlers::need_more_params_handler;
    handlers["476"] = Handlers::bad_channel_mask_handler;
    handlers["482"] = Handlers::channel_op_needed_handler;
    handlers["700"] = Handlers::operation_denied_handler;


    if (LOG) std::cout << "CLIENT_LOG: Started app" << std::endl;
}

Client::~Client() {
    shutdown(hub_socket, SHUT_RDWR);
	close(hub_socket);

    if(cur_file && cur_file.is_open()) cur_file.close();

    delete Screen::window;

    if (LOG) std::cout << "CLIENT_LOG: Disconnected from HUB" << std::endl;
    if (LOG) std::cout << "CLIENT_LOG: Shutting Client down" << std::endl;
}