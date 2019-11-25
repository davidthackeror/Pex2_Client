//
// Author: Capt G. Parks Masters
// Date: 6 Oct 2019
//

#ifndef TCP_MP3SERVER_TCP_FUNCITONS_H
#define TCP_MP3SERVER_TCP_FUNCITONS_H

// Structure is used to track the details of the current connection.
// This data is initialized during TCPConnect (3-way handshake)
// Must be provided to TCPSend/Receive so that subsequent packets have
//   the correct header information, and errors can be handled appropriately.
struct tcp_info{
    int my_seq;         //stores my INITIAL SEQ number established during TCPConect.  This never changes during a connection
    int remote_seq;     //stores server's INITIAL SEQ number established during TCP Connect.  This never changes during a connection
    int data_sent;      //used to determine what my SEQ # and server's ACK # should be
    int data_received;  //used to track what my ACK# and server's SEQ should be
    int remote_data_acknowledged;  //number of bytes server has received.  Updated
};

// Function to conduct 3-way handshake with server.  Establishes initial
//   SEQ and ACK numbers.  Returns a tcp_info structure to the MP3Client
//   that can later be passed to TCPSend/Receive.
struct tcp_info* TCPConnect(int sockfd,  struct sockaddr_in * servaddr){
    struct tcp_info initTCP;
    initTCP.my_seq = 1;
    initTCP.remote_seq = 1;
    initTCP.data_sent = 0;
    initTCP.data_received = 0;
    initTCP.remote_data_acknowledged = 0;
    char* initSend = "FLAGS\n2\nSEQ\n1\nACK\n9477\nAPPDATA\n0";
    //TODO sentto doesnt work from this function, probably has something to do with the servaddr works in main
    sendto(sockfd, (const char *) initSend, strlen(initSend), 0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    printf("Starting three way handshake\n");
    int n, len = sizeof(servaddr);
    char buffer[MAXLINE]; //buffer to store message from server
    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {
        perror("ERROR");
        printf("Errno: %d. ", errno);
        exit(EXIT_FAILURE);
    }
    buffer[n] = '\0'; //terminate message
    //display message received from the server
    printf("Server : %s\n", buffer);
    return &initTCP;
}


// Replaces all instances of "recvfrom" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer where the application data in the packet
//           will be stored (just like recvfrom()).  TCP header info is stripped
//           from the packet before it is returned.
//  connection_info: structure that contains info about current connection
int TCPReceive(int sockfd, char *appdata, int appdata_length, struct sockaddr_in *addr,
                     struct tcp_info *connection_info);
//TODO TCPReceive

// Replaces all instances of "sendto" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer that contains the application data the
//           MP3Client is trying to send.  TCP header information is added to
//           the beginning of this data before it is sent.
//  connection_info: structure that contains info about current connection
int TCPSend(int sockfd, char* appdata, int appdata_length, struct sockaddr_in * addr, struct tcp_info *connection_info);

//TODO TCPSend

#endif //TCP_MP3SERVER_TCP_FUNCITONS_H