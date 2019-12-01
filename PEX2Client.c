/** PEX2Client.c
 * ===========================================================
 * Name: CS468, Fall 2019
 * Project: PEX1_Client
 * Purpose: Implementation of Client Server Music Streaming
 *          Communication
 * Of note: Dr. Coffman said to never include a name so
 * ===========================================================
 */

#include <w32api/minwindef.h>
#include "PEX2ClientNew.h"


#define timeout_time 5


int main() {
    int sockfd; //Socket descriptor, like a file-handle

    char *LIST_REQUEST = "LIST_REQUEST"; //message to send to server
    char songInput[100];
//    char START_STREAM[150] = "START_STREAM\nBilly Joel - We Didn't Start the Fire.mp3";
    struct sockaddr_in servaddr;  //we don't bind to a socket to send UDP traffic, so we only need to configure server address
    printf("Please enter one of the following commands: \n");
    //declares the input variable of the string so that it doesn't die
    char *inputS;
    //throwaway pointer
    char *ptr;
    //the long int that is validated with the choices
    long input;
    //declares that the socket is closed
    bool socketOpen = false;
        //clear any input previously sent in
        fflush(stdout);
        //list out for the users the choices
        printf("'1' = List Songs\n");
        printf("'2' = Exit\n");
        //take user input in the form of a string
        //convert the string to an int for comparison
        //Program opens socket and receives communication from server for songs stored on the server


            // Creating socket file descriptor
            if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
            //declares that the socket is open
            socketOpen = true;

            // Filling server information
            servaddr.sin_family = AF_INET; //IPv4
            servaddr.sin_port = htons(
                    PORT); // port, converted to network byte order (prevents little/big endian confusion between hosts)
            servaddr.sin_addr.s_addr = INADDR_ANY; //localhost

            //Sending message to server
            struct tcp_info *tcpStruct = TCPConnect(sockfd, servaddr);
            printf("%d", tcpStruct->my_seq);
            // Receive message from client

//            //close the socket to reestablish connection later if needed
            close(sockfd);


    printf("Exiting program and closing the socket");
    close(sockfd);
    return 0;
}

