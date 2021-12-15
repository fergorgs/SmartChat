CPP_VERSION = -std=c++17
PCH_SERVER = -include server/PCHServer.h
PCH_CLIENT = -include client/PCHClient.h

server_src = server/app.cpp server/Handlers.cpp server/Hub.cpp server/Connection.cpp server/MessageSendController.cpp server/Channel.cpp utils/RFCprotocol.cpp
server_obj = $(patsubst %.cpp,%.o,$(server_src))

client_src = client/app.cpp client/Client.cpp client/Handlers.cpp client/Executors.cpp client/ui/Screen.cpp utils/StrManip.cpp utils/RFCprotocol.cpp
client_obj = $(patsubst %.cpp,%.o,$(client_src))

utils/%.o: utils/%.cpp
	# Making Utils Object
	g++ -c -o $@ $(PCH_SERVER) $< $(CPP_VERSION) 

client/%.o: client/%.cpp
	# Making Client Object
	g++ -c -o $@ $(PCH_CLIENT) $< `pkg-config gtkmm-3.0 --cflags` $(CPP_VERSION) 

server/%.o: server/%.cpp
	# Making Server Object
	g++ -c -o $@ $(PCH_SERVER) $< -pthread $(CPP_VERSION) 

all: client/PCHClient.h.gch server/PCHServer.h.gch $(client_obj) $(server_obj) 
	g++ -o bin/app $(client_obj) `pkg-config gtkmm-3.0 --libs` $(CPP_VERSION)
	g++ -o bin/hub $(server_obj) -pthread $(CPP_VERSION)

server/PCHServer.h.gch: server/PCHServer.h
	g++ -c -o $@ $< -pthread $(CPP_VERSION)

client/PCHClient.h.gch: client/PCHClient.h
	g++ -c -o $@ $< `pkg-config gtkmm-3.0 --cflags` $(CPP_VERSION)
	
client: bin/app
	./bin/app
	
server: bin/hub
	./bin/hub