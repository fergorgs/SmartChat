#include "RFCprotocol.h"

//parseString
//splits a string into substrings usen a given substring as key
//RECEIVES: string to split, subtring to use as key
//RETURNS: o vector object with the splited substrings
vector<string> parseString(string original, string del){

    vector<string> parsed = vector<string>();

    while (original.find(del) != string::npos)
    {
        string sub = original.substr(0, original.find(del));
        parsed.push_back(sub);

        original = original.substr(original.find(del)+del.length(), original.length()-original.find(del)-del.length());
    }

    parsed.push_back(original);
    
    return parsed;
}




//Prefix
//constructor of class Prefix
Prefix::Prefix(){

    serverName = "";
    nick = "";
    user = "";
    host = "";    
}




//Prefix::setters
//return 0 if suceded
//      -1 if it fails, due to the fact that "serverName" and "nick"
//      can't both have non-NULL values at the same time
int Prefix::setServerName(string serverName){
    if(nick == "" && user == "" && host == ""){
        this->serverName = serverName;
        return 0;
    }
    else
        return -1;
    
}
int Prefix::setNick(string nick){
    if(serverName == ""){
        this->nick = nick;
        return 0;
    }
    else
        return -1;
    
}
int Prefix::setUser(string user){
    if(serverName == ""){
        this->user = user;
        return 0;
    }
    else
        return -1;
}
int Prefix::setHost(string host){
    if(serverName == ""){
        this->host = host;
        return 0;
    }
    else
        return -1;
}




//Prefix::getters
string Prefix::getServerName() { return serverName; }
string Prefix::getNick() { return nick; }
string Prefix::getUser() { return user; }
string Prefix::getHost() { return host; }




//Command
//constructor of class Command
Command::Command(){

    cmd = "";
    // word = "";
    // number = -1;
}


//Command::setters
//return 0 if suceded
//      -1 if it fails, due to the fact that "word" and "number"
//      can't both have non-NULL values at the same time
void Command::set_cmd(string cmd){
    this->cmd = cmd;
}


//Command::setters
//return 0 if suceded
//      -1 if it fails, due to the fact that "word" and "number"
//      can't both have non-NULL values at the same time
// int Command::setWord(string word){
//     if(number = -1){
//         this->word = word;
//         return 0;
//     }
//     else
//         return -1;
// }
// int Command::setNumber(int number){
//     if(word == ""){
//         this->number = number;
//     return 0;
//     }
//     else
//         return -1;
// }




//Command::getters
string Command::get_id() { return this->cmd; }

// string Command::getWord() { return word; }
// int Command::getNumber() { return number; }




//Param
//constructor of class Param
Param::Param(){

    middle = vector<string>();
    trailing = "";
}




//Param::addMiddleParam
//given a string, ads it to the vector of middle params
//the string must not strat with ':' nor contain NUL, LF, CR or SPACE
//RECEIVES: the string to add
//return 0 if suceded
//      -1 if it fails because the string if null
//      -2 if it fails because the string starts with ':'
//      -3 if it fails because the string has an invalid character
int Param::addMiddleParam(string param){

    if(param.length() == 0)
        return -1;
    if(param[0] == ':')
        return -2;
    if(param[param.length()-1] == NUL || param.find(LF) != string::npos 
        || param.find(CR) != string::npos || param.find(SPACE) != string::npos)
        return -3;
    else
    {
        middle.push_back(param);
        return 0;
    }
    
}
//Param::setTrailing
//given a stirng sets the trailing
//the string must not contain NUL, LF or CR
//RECEIVES: a string to add to trailing
//RETURNS: 0 if succeded
//         -1 if it fails because the string has an invalid character
int Param::setTrailing(string trailing){

    if(trailing.length() > 0){
        // if (trailing[trailing.length()-1] == NUL || trailing.find(LF) != string::npos 
        //     || trailing.find(CR) != string::npos)
        //     return -1;
        // else
        // {
            this->trailing = trailing;
            return 0;
        // }
    }
    else
    {
        this->trailing = "";
        return 0;
    }
}




//Param::getters
vector<string> Param::getMiddleContent() { return middle; }
string Param::getTrailing() { return trailing; }




//Message()
//constructs an empty Message class
Message::Message(){

    prefix = Prefix();
    command = Command();
    params = Param();
}




//Message(serializedMessage)
//constructs a Message class starting from a given RFC Message
Message::Message(string& serializedMessage){

    string msg = serializedMessage;

    int i = 0;

    //PARSING THE STRING
    string trailing = msg.substr(msg.find(" :")+1, msg.length()-(msg.find(" :")+1));
    msg = msg.substr(0, msg.find(" :"));

    vector<string> segs = parseString(msg, " ");
    segs.push_back(trailing);

    /*for(int i = 0; i < segs.size(); i++)
        cout << "||" << segs[i] << "||" << endl;*/

    //GET PREFIX
    if(segs[0][0] == ':')
    {
        //if there is user and host
        if(segs[0].find("!") > 0 && segs[0].find("@") > 0)
        {
            prefix.setNick(segs[0].substr(1, segs[0].find("!")-1));
            prefix.setUser(segs[0].substr(segs[0].find("!")+1, segs[0].find("@")-(segs[0].find("!")+1)));
            prefix.setHost(segs[0].substr(segs[0].find("@")+1, segs[0].length()-(segs[0].find("@")+1)));
        }
        //if there is only user
        else if(segs[0].find("!") > 0)
        {
            prefix.setNick(segs[0].substr(1, segs[0].find("!")-1));
            prefix.setUser(segs[0].substr(segs[0].find("!")+1, segs[0].length()-(segs[0].find("!")+1)));
        }
        //if there is only host
        else if(segs[0].find("@") > 0)
        {
            prefix.setNick(segs[0].substr(1, segs[0].find("@")-1));
            prefix.setHost(segs[0].substr(segs[0].find("@")+1, segs[0].length()-(segs[0].find("@")+1)));
        }
        //if there is nither user nor host
        else
            prefix.setNick(segs[0].substr(1, segs[0].length()-1));

        i++;
    }

    //GET COMMAND
    command.set_cmd(segs[i]);
    // //if the command is a number
    // if(isdigit(segs[i][0]))
    //     command.setNumber(stoi(segs[i]));
    // //if the command starts with a letter
    // else
    //     command.setWord(segs[i]);

    i++;

    
    //GET PARAMS
    //getting middle arguments
    while (segs[i][0] != ':')
    {
        int res;
        //cout << "size: " << segs[i].length() << " ||" << segs[i] << "||" << endl;
        res = params.addMiddleParam(segs[i]);
        //if(res != 0)
        //    cout << "error: " << res << endl;
        i++;
    }

    //getting trailing
    params.setTrailing(segs[i].substr(1, segs[i].length()-1));
}




//Message:serializeMessage
//RETURNS the content of the Message serialized and ready to transmit
string Message::serializeMessage(){

    string finalMessage = "";

    //PREFIX
    //Checks to see if there is a prefix at all
    if(prefix.getServerName() != "")
        finalMessage = ":" + prefix.getServerName() + " ";

    else if(prefix.getNick() != "")
    {
        finalMessage = ":" + prefix.getNick();

        if(prefix.getUser() != "")
            finalMessage += "!" + prefix.getUser();
        if(prefix.getHost() != "")
            finalMessage += "@" + prefix.getHost();

        finalMessage += " ";
    }


    //COMMAND
    finalMessage += command.get_id() + " ";
    // //Checks if the command is written or numeric
    // if(command.getWord() != "")
    //     finalMessage += (command.getWord() + " ");
    // else
    //     finalMessage += (to_string(command.getNumber()) + " ");
    
    
    //PARAMS

    for(int i = 0; i < params.getMiddleContent().size(); i++)
        finalMessage += (params.getMiddleContent()[i] + " ");

    finalMessage += (":" + params.getTrailing());


    //Message TERMINATOR
    string cr = "" + to_string(CR);
    string lf = "" + to_string(LF);
    //finalMessage += (" " + cr + lf);           <---------- ta bugado

    return finalMessage;
}




//Message::listMessageComponents
//for debug purposes, lists the Message components
void Message::listMessageComponents(){

    cout << "PREFIX" << endl;

    cout << "ServerName: " << prefix.getServerName() << endl;
    cout << "Nick: " << prefix.getNick() << endl;
    cout << "User: " << prefix.getUser() << endl;
    cout << "Host: " << prefix.getHost() << endl;

    cout << endl;

    cout << "COMMAND" << endl;

    cout << "ID: " << command.get_id() << endl;
    // cout << "Word: " << command.getWord() << endl;
    // cout << "Number: " << command.getNumber() << endl;

    cout << endl;

    cout << "PARAMS" << endl;

    cout << "Middle: " << endl;

    cout << "..middle size: " << params.getMiddleContent().size() << endl;

    for(int i = 0; i < params.getMiddleContent().size(); i++)
        cout << "\t" << params.getMiddleContent()[i] << endl;

    cout << "Trailing: " << params.getTrailing() << endl;

    cout << "---END OF MESSAGE---" << endl << endl;
}
