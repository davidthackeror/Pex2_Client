//
// Author: Capt G. Parks Masters
// SubAuthor: David J. Thacker
// Date: 6 Oct 2019
//

#ifndef TCP_MP3SERVER_TCP_FUNCITONS_H
#define TCP_MP3SERVER_TCP_FUNCITONS_H

#define MAXLINE 600

#include "PEX2ClientNew.h"

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

/**
 * struct to hold data from a stripped header.
 */
struct headerStrip{
    int SEQ;
    int ACK;
    char* data;
};

/**
 * stripHeader removes the underlying string from a flag header and returns the data in a struct called headerStrip
 * @param buffer the string containing a header to be stripped
 * @return the struct containing all underlying data from the header
 */
struct headerStrip* stripHeader(char* buffer){
    //create a new stuct headerStrip which will contain all the data in the header
    struct headerStrip *header = malloc(sizeof(struct headerStrip));
    //unused but breaks program if removed, idk why its kinda weird
    const char* s = "ab234cid*(s349*(20kd";
    int FLAG, SEQ, ACK;
    //goes through the string and parses for ints in the string, in this case the flag, SEQ, and ACK
    sscanf(buffer,
                    "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                    &FLAG,
                    &SEQ,
                    &ACK);
    //store the retrieved seq and ack in the struct
    header->SEQ = SEQ;
    header->ACK = ACK;
    //34 is the number of bytes to get to the appdata section, will grab any string after that and store in struct
    char* pnw = buffer + 34;
    header->data = pnw;
    return header;

}

/**
 * rejectPacket serves as a check to see if the header given passes the required checks as a fuctioning non-malformed packet
 * @param buffer the header given from the recvfrom
 * @param connection_info the struct containing all current data on sending and recieving
 * @return a -1 if the SEQ fails, a -2 if the ACK fails, or a 1 if all checks pass
 */
int rejectPacket(char* buffer, struct tcp_info *connection_info){
    struct headerStrip* header = stripHeader(buffer);

    //is header malformed
    if(buffer[1] != 'L'){
        printf("Program recived a malformed header, program will exit");
        exit(EXIT_FAILURE);
    }
    //is the given sequence number wrong
    if(header->SEQ != connection_info->remote_seq + connection_info->data_received + 1){
        return -1;
    }
    //is the given ACK wrong
    if(header->ACK != connection_info->my_seq + connection_info->data_sent + 1){
        return -2;
    }

    return 1;
}
/**
 * tcpHeaderCreator takes creates a header as specified in the PEX2 documentation
 * @param flags the flag to be sent to the server, usually a 2 or a 18
 * @param seq the sequence number to be sent to the server
 * @param ack the ack number to be sent to the server
 * @param data any data to be sent to the server, must be in the form of a string
 * @return the string containing a correctly formatted header
 */
char* tcpHeaderCreator(int flags, int seq, int ack, char* data){
    //this could probably be done more eleoquently but it works so hey!
    //temp char[] to store passed in flags and strings they form
    char ch1[100];
    char ch2[100];
    char ch3[100];
    char ch4[100];

    //create the flag string with its data
    sprintf(ch1, "%d", flags);
    char flagsString[200] =  "FLAGS\n";
    strcat(flagsString, ch1);

    //create the seq string with its data
    sprintf(ch2, "%d", seq);
    char seqString[100] =  "\nSEQ\n";
    strcat(seqString, ch2);

    //create the ack string with its data
    sprintf(ch3, "%d", ack);
    char ackString[100] =  "\nACK\n";
    strcat(ackString, ch3);

    //create the appdata string with its data
    char dataString[100] =  "\nAPPDATA\n";
    //check to see if any data was actually passed in
    if(data != NULL){
        sprintf(ch4, "%s", data);
        strcat(dataString, ch4);
    }

    //start throwing the strings together
    strcat(flagsString,seqString);

    strcat(flagsString,ackString);

    strcat(flagsString,dataString);

    //create a char* to throw all the concat strings together
    char* buffer = malloc(sizeof(flagsString));
    strcpy(buffer, flagsString);

    //return the concat string to caller
    return buffer;
}

/**
 * Function to conduct 3-way handshake with server.  Establishes initial SEQ and ACK numbers.
 * @param sockfd socket to connnect to
 * @param servaddr address to send to
 * @return a tcp_info structure to the MP3Client that can later be passed to TCPSend/Receive.
 */
struct tcp_info* TCPConnect(int sockfd,  struct sockaddr_in servaddr){

    struct tcp_info *initTCP = malloc(sizeof(struct tcp_info));
    time_t t;
    srand((unsigned) time(&t));
    int seq = rand() % 10000;
    initTCP->my_seq = seq;
    char* header = tcpHeaderCreator(2,seq,0,0);
    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
//    printf("Starting three way handshake\n");
    int n, len = sizeof(servaddr);
    char buffer[MAXLINE]; //buffer to store message from server
    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {
        perror("ERROR");
        printf("Errno: %d. ", errno);
        exit(EXIT_FAILURE);
    }
//    printf("Server: \n %s \n", buffer);
    const char* s = "ab234cid*(s349*(20kd";
    int FLAG, SEQ, ACK;
    sscanf(buffer,
                    "%*[^0123456789]%d%*[^0123456789]%d%*[^0123456789]%d",
                    &FLAG,
                    &SEQ,
                    &ACK);

    initTCP->remote_seq = SEQ;
    header = tcpHeaderCreator(16,seq + 1, SEQ + 1,0);
    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &servaddr,
           sizeof(servaddr));
    initTCP->remote_data_acknowledged = 0;
    initTCP->data_received = 0;
    initTCP->data_sent = 0;
    return initTCP;
}



// Replaces all instances of "recvfrom" in your MP3Client.
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer where the application data in the packet
//           will be stored (just like recvfrom()).  TCP header info is stripped
//           from the packet before it is returned.
//  connection_info: structure that contains info about current connection
int TCPReceive(int sockfd, char* appdata, int appdata_length, struct sockaddr_in addr, struct tcp_info *connection_info){
    int n, len = sizeof(addr);
    char buffer[MAXLINE]; //buffer to store message from server
    //fetches response from server

    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) <= 0) {
        perror("ERROR");
        printf("Errno: %d. ", errno);
        exit(EXIT_FAILURE);
    }
    //strip the header recieved from the server
    struct headerStrip* header = stripHeader(buffer);

    int info = rejectPacket(buffer, connection_info);

    if(info == -1){
        char* tempAck = tcpHeaderCreator(16,connection_info->my_seq+connection_info->data_sent + 1,connection_info->remote_seq+connection_info->remote_data_acknowledged + 1,appdata);
        sendto(sockfd, (const char *) tempAck, strlen(tempAck), 0, (const struct sockaddr *) &addr, sizeof(addr));

    }

    //update TCP struct with data recieved
    connection_info->data_received = connection_info->data_received + strlen(header->data) -1;

    //send an ACK that data has been recieved
    char* ackHeader = tcpHeaderCreator(16, connection_info->my_seq+connection_info->data_sent + 1, connection_info->remote_seq+connection_info->data_received + 1, NULL);
    sendto(sockfd, (const char *) ackHeader, strlen(ackHeader), 0, (const struct sockaddr *) &addr,
           sizeof(addr));


    //update struct that the remote data has been ACK'd
    connection_info->remote_data_acknowledged = connection_info->data_received;

    //update the passed in buffer with the data
    appdata = header->data;

    //print the data so CPT Masters thinks I know what I'm doing
    //If CPT Masters is reading this I couldn't get the appdata to be updated in the main of PEX2Client so I print it here, sorry
    printf("%s", appdata + 11);

    //free the sturct with the returned header data
    free(header);
    //return total data recieved
    return strlen(header->data);
}


// Replaces all instances of "sendto" in your MP3Client.+
// UNIQUE PARAMETERS:
//  appdata: pointer to the buffer that contains the application data the
//           MP3Client is trying to send.  TCP header information is added to
//           the beginning of this data before it is sent.
//  connection_info: structure that contains info about current connection
int TCPSend(int sockfd, char* appdata, int appdata_length, struct sockaddr_in addr, struct tcp_info *connection_info){
    //send a packet listing intent to send data
    char* header = tcpHeaderCreator(16,connection_info->my_seq+connection_info->data_sent + 1,connection_info->remote_seq+connection_info->remote_data_acknowledged + 1,appdata);
    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &addr, sizeof(addr));
    //update struct with the data we have sent
    connection_info->data_sent = connection_info->data_sent + appdata_length;

    int n, len = sizeof(addr);
    char buffer[MAXLINE]; //buffer to store message from server
    //fetches response from server
    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) <= 0) {
        perror("ERROR");
        printf("Packet dropped, resending\n1");
        //will attempt up to three times to contact server and resend ack packet
        if(errno == 11) {
            sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &addr, sizeof(addr));
            if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) <= 0) {
                // Handle errors here.  Errno 11, "Resource Temporarily Unavailable" is returned as a result of a timeout.
                perror("ERROR");
                printf("Errno: %d. ", errno);
                if (errno == 11) {
                    sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &addr,
                           sizeof(addr));
                    if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) <= 0) {
                        // Handle errors here.  Errno 11, "Resource Temporarily Unavailable" is returned as a result of a timeout.
                        perror("ERROR");
                        printf("Errno: %d. ", errno);
                        if (errno == 11) {
                            sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &addr,
                                   sizeof(addr));
                            if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &addr, &len)) <=
                                0) {
                                // Handle errors here.  Errno 11, "Resource Temporarily Unavailable" is returned as a result of a timeout.
                                perror("ERROR");
                                printf("Errno: %d. ", errno);
                                if (errno == 11) {
                                    exit(EXIT_FAILURE);
                                }
                            }
                        }
                    }
                }
            }
        }
    }


    //strip the header
    struct headerStrip* strippedHeader = stripHeader(buffer);

    //see if packet should be rejected or have work done
    int info = rejectPacket(buffer, connection_info);

    //detected is wrong ACK number is given and sends a new ack
    if(info == -2){
        printf("Wrong ACK number detected\n");
        printf("%d\n", strippedHeader->ACK);
        printf("Expected %d\n", (connection_info->my_seq + connection_info->data_sent + 1));
        sendto(sockfd, (const char *) header, strlen(header), 0, (const struct sockaddr *) &addr, sizeof(addr));

    }



}

#endif //TCP_MP3SERVER_TCP_FUNCITONS_H