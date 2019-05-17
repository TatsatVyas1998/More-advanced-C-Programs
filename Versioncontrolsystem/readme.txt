version Control System

In this project i have used TCP server client connection to implement a Version control system simillar to github. This was created on Ubuntu and it uses tar to compress files
when sending from server to client or vice versa. On server side it uses threads of communicate with multiple clients and mutex synchronization to avoid any file curruption.
when the server starts it creates a main thread to listen on the socket and create other threads to communicate with different clients. 

It takes certain command on client side to communicate with server and make changes to any projects.

configure <ip> <port> : this takes in ip and port of the server that you are trying to communicate to. When you pass this command it creates a file with ip and port of the server
which is used for all the other commands. tar ubuntu
create <project> : creates a project on server and client side. 
add <project> <filename> : adds the file to the project. 
commit <project> : creates a commit file which contains all the differences between the server project and client project.
push <project> : needs a commit file to run and it pushes the project to the server with all the changes that you have made. when it is making all the changes it locks the project 
directory on the server side so no other client can make changes to it. 
update <project> : creates an update file with all the changes between server and client project.
upgrade <project> : needs update file to run. it updates the client's project version and matches with the server. 
checkout <project> : gets the project from the server if client doesn't have it. 
history <project> : prints all the changes made to the project since creation. 
rollback <project> <version#> : replaces the project version with the specified verion of the project. 
destroy <project> : completely removes the project from server and client side. it removes all the old versions of the project on server. 
remove <project> <file> : removes the file from the cilent side. 
