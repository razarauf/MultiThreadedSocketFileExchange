# Multithreaded socket file exchange

The implementation is used to exchange files between the server and the client.

The client connects with the server using a hostname and a port number and exchanges files with the server using an assigned socket.

The server hosts on an available port number and listens to clients’ requests through an assigned socket. The server also implements pthread and forks, which allows the server to process multiple clients’ requests in parallel.

Please note the program is not completely finished yet. 