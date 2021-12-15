# Internet Relay Chat - SSC0142
This repository is internet relay chat project for SSC0142 course (ICMC - USP).

## How to Run
**!!! THIS PROJECT HAS ONLY BEEN TESTED IN UBUNTU 18.04 & 19.10 & 20.04 !!!**  
**!!! COMPILER USED IS g++ (Ubuntu 9.3.0-10ubuntu2) 9.3.0, WITH c++17 !!!**
**!!! THE PROGRAM HAS USER INTERFACE, SO DOESN'T RUN ON INTERFACELESS OS !!!**

### Compilation
* 1st step: Install gtkmm lib on your OS
    * This can be easily acheived by running the following command:
    ```
        $ apt install libgtkmm-3.0-dev
    ```

* 2nd step: compile all programs
    * That is easier than the first step, because we prepared a Makefile, so you just need to run:
    ```
        $ make all
    ```

### Running Client
* To run the client, simply type:
    ```
        $ make client
    ```

    * Note: it is no longer necessary to run the server program first. The server must be running only to connect the client program.

* **Implemented Client Commands**  
Once the Client is running, you have the following commands at your disposal:
    * **/connect**: use this command to connect to the server. It will only work if the server program is running. You must be connected to execute all other commands (except /quit, of course). This command accept connections to remove computers. To connect locally just do */connnect*; to connect remotely you need to specify a Domain/IP address (*/connect [ip/domain]*). The server to be connected must be running on port 6667.
    * **/join**: used to connect to a channel. You need to pass a channel name, with the mask #[name].
    * **/kick**: this command kicks an user from a channel. Need to pass [nick] of the user to be kicked. You need to be channel OP to perform this command.
    * **/mute**: this command mutes an user from a channel. Need to pass [nick] of the user to be muted. You need to be channel OP to perform this command.
    * **/nick**: being connected to the server, this command makes you define or change your nickname. If you've just connected, your nickname isn't defined and you must define your nickname to be able to send messages in chat.
    * **/ping**: this command sends a "PING" to the server and waits for "PONG" response. It is used to measure the socket latency. You need to be connected to do this.
    * **/quit**: this command terminates the application. It has the same effect as clicking on the famous red 'x' in the top window bar.
    * **/unmute**: this command unmutes an user from a channel. Need to pass [nick] of the user to be unmuted. You need to be channel OP to perform this command.
    * **/whois**: this command returns the ip of the user with the [nick] sent. Need to pass target user [nick]. You need to be channel OP to perform this command.

### Running Server
* To run the server application, simply:
    ```
        $ make server
    ```
* The server application doesn't interact with the user. It only logs some messages of connections being made or broken, and message management.  
* To close this application the user just have to send an interrupt (SIGINT, Ctrl-C) via terminal.

### Disclaimers
* The server now can handle more than 2 connections and doesn't close all applications on shutdown, just disconnects all client applications. Then, the client can connect only after the server is running again.
* For now the threshold to break the message into parts is 4000 chars. This is due to addition of header data, such as the command used and user's nickname. In the next submissions, this size threshold may change to a bigger or smaller value, to avoid wasting space.
* The server has a confirmation of received message now: when the client receive a message it sends back a "ACK". Then the server sends the next message. If the server fail to read "ACK" five times, then the connection is lost. For each attempt to read "ACK", if it fails, it sends the message again to the client app. 

## Group Members
* Alexandre Galocha Pinto Junior (10734706) [git](https://github.com/alexandregjr)  
* Eduardo Pirro (10734665) [git](https://github.com/EdPirro)  
* [Fernando Gorgulho Fayet](10734407) [git](https://github.com/fergorgs)
* Tijolo
