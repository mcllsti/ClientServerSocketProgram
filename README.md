# ClientServerSocketProgram
A client server socket program written in C for university.

# TCP Client-Server System User Manual

This document contains need to know information regarding the running and operation of the
included TCP Client-Server System.
This system contains two components; the server code and the client code. It is designed to
currently be both run on the same computer but once roll out is ready the system will run the server
code on a computer while clients can connect from external systems.
The server can handle multiple clients connecting at once using threads to do so. Each client can
interact with the server in an isolated manor without other clients affecting them. Clients can
perform a number of menu based self-explained tasks to interact with the system once connected

# Server

To run the server system: Navigate to the directory containing the server code. Type “make” to
ensure the system is compiled and then type “./server” to run the code.
Once run, the system can be left running indefinitely as no external input is needed to deal with
clients. Clients can connect and disconnect at will. Any connection or interaction with a client
terminal will be logged on the terminal screen. This screen will clear when the server ends.
To end the server, send a SIGINT signal to the server or press ctrl+c on the terminal window which
also sends the same signal. The server will output the total server uptime and then disconnect.

# Client

To run the client system: Nagivate to the directory containing the client code. Type “make” to
ensure the system is compiled and then type “./client” to run the code.
Providing the server is running and able to connect then the client will automatically connect. Once
connected a menu will be showen which takes in a number to correspond to a function:
0 – Displays the menu options again
1 – Requests the server to return an output of current client IP and a student ID joined together
2 – Returns the server time and date
3 – Returns a list of server information
4 – Returns a list of files in a upload directory on the server
5- Exits the program.
Clients can press any number and enter and the function will then execute and return to the menu
options. If the server disconnects then the client will terminate after a key press.
Clients can disconnect by typing 5 or by killing the process. 
