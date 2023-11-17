# SOCKS 4
## Introduction
SOCKS is similar to a proxy (i.e., intermediary-program) that acts as both server and client for the purpose of making requests on behalf of other clients. Because the SOCKS protocol is independent of application protocols, it can be used for many different services: telnet, ftp, WWW, etc.  
There are two types of the SOCKS operations, namely CONNECT and BIND. You have to implement both of them in this project. We will use Boost.Asio library to accomplish this project.

## SOCKS 4 Implementation
After the SOCKS server starts listening, if a SOCKS client connects, use fork() to tackle with it.
Each child process will do:  
1. Receive SOCKS4 REQUEST from the SOCKS client  
2. Get the destination IP and port from SOCKS4 REQUEST  
3. Check the firewall (socks.conf), and send SOCKS4 REPLY to the SOCKS client if rejected  
4. Check CD value and choose one of the operations  
- CONNECT (CD=1)  
    1. Connect to the destination
    2. Send SOCKS4 REPLY to the SOCKS client
    3. Start relaying traffic on both directions  
- BIND (CD=2)
    1. Bind and listen a port
    2. Send SOCKS4 REPLY to SOCKS client to tell which port to connect
    3. (SOCKS client tells destination to connect to SOCKS server)
    4. Accept connection from destination and send another SOCKS4 REPLY to SOCKS client 
    5. Start relaying traffic on both directions
If the SOCKS server decides to reject a request from a SOCKS client, the connection will be closed immediately.  