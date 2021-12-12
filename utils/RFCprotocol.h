#ifndef RFC_PROTOCOL_H
#define RFC_PROTOCOL_H

#define NUL 0
#define LF 10
#define CR 13
#define SPACE 32

using namespace std;

vector<string> parseString(string original, string del);


class Prefix{

    private:
    
    string serverName;
    string nick;
    string user;
    string host;

    public:

    Prefix();

    int setServerName(string serverName);
    int setNick(string nick);
    int setUser(string user);
    int setHost(string host);

    string getServerName();
    string getNick();
    string getUser();
    string getHost();

};


class Command{

    private:

        string cmd;
        // string word;
        // int number;

    public:

        Command();

        void set_cmd(string cmd);

        // int setWord(string word);
        // int setNumber(int number);

        string get_id();

        // string getWord();
        // int getNumber();
};


class Param{

    private:

    vector<string> middle;
    string trailing;

    public:

    Param();

    int addMiddleParam(string param);
    int setTrailing(string trailing);

    vector<string> getMiddleContent();
    string getTrailing();
};


class Message{

    public:
    
    Prefix prefix;
    Command command;
    Param params;

    Message();

    Message(string& serializedMessage);
    
    string serializeMessage();

    void listMessageComponents();
};

#endif