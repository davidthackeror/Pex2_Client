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
#include "PEX2Client.h"
#include "tcp_funcitons.h"


#define timeout_time 5


int main() {
    int sockfd; //Socket descriptor, like a file-handle

    char buffer[MAXLINE]; //buffer to store message from server

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
    do {
        //clear any input previously sent in
        fflush(stdout);
        //list out for the users the choices
        printf("'1' = List Songs\n");
        printf("'2' = Download Song\n");
        printf("'3' = Exit\n");
        //take user input in the form of a string
        scanf("%s", inputS);
        //convert the string to an int for comparison
        input = strtol(inputS, &ptr, 10);
        //Program opens socket and receives communication from server for songs stored on the server
        if (input == 1) {

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

            int n, len = sizeof(servaddr);
            //Sending message to server
            sendto(sockfd, (const char *) LIST_REQUEST, strlen(LIST_REQUEST), 0, (const struct sockaddr *) &servaddr,
                   sizeof(servaddr));
            printf("Requesting Song List.\n");

            // Receive message from client
            if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {
                perror("ERROR");
                printf("Errno: %d. ", errno);
                exit(EXIT_FAILURE);
            }
            buffer[n] = '\0'; //terminate message
            //display message received from the server
            printf("Server : %s\n", buffer);
            //close the socket to reestablish connection later if needed
            close(sockfd);
        }

        //if the user wants to select a song then enter this
        if (input == 2) {

            //declares a string to send to the server to start a song stream
            char START_STREAM[150] = "START_STREAM\n";
            // Creating socket file descriptor and binds the socket
            if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
            struct timeval timeout; //structure to hold our timeout

            timeout.tv_sec = 5; //5 second timeout
            timeout.tv_usec = 0; //0 milliseconds

            if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout, sizeof(timeout)) < 0) {
                perror("setsockopt failed");
                exit(EXIT_FAILURE);
            }

            //Tells the client side which IP version it shoudl run off of
            servaddr.sin_family = AF_INET; //IPv4
            servaddr.sin_port = htons(
                    PORT); // port, converted to network byte order (prevents little/big endian confusion between hosts)
            servaddr.sin_addr.s_addr = INADDR_ANY; //localhost

            int n, len = sizeof(servaddr);


            printf("Please input the name of the song to be streamed: ");
            //This clears the input buffer from the users choice of what to do above
            getchar();
            //creates the songInput variable
            char songInput[150];
            //gets user inptu from stdin and stores it in songinput
            fgets(songInput, sizeof(songInput), stdin);
            //removes the null terminator from the string which messes with the transmission to the server with the song name
            int i = 0;
            while (songInput[i] != '\0') {
                i++;
            }
            songInput[i - 1] = '\0';

            printf("Fetching Song.\n");
            //Concatenates the START_STREAM\n command with the chosen song name
            strcat(START_STREAM, songInput);
            printf("%s\n", START_STREAM);
            printf("Sending Start Response\n");
            printf("Waiting For Response.\n");
            //makes a file pointer called fptr
            FILE *fptr;
            //declares space where the fileName will go
            char fileName[100];
            //Allows for a file path
            char path[100] = "./";
            //place the songInput with a relative path
            strcat(path, songInput);
            //place the songInput as a fileName to be sent
            strcpy(fileName, songInput);


            //opens or creates a file for reading or writing
            fptr = fopen(fileName, "a+");
            /*tells the server that we want to connect on socket sockfd, gives it the message to be sent and the length of
             * that message, sets any error flags to none.
             */
            sendto(sockfd, (const char *) START_STREAM, strlen(START_STREAM), 0, (const struct sockaddr *) &servaddr,
                   sizeof(servaddr));
            int frameCounter = 0;
            // Receive message from client
            while (1) {
                memset(buffer, '\0', MAXLINE);
                //recvfrom returns the number of characters in the message from the server
                if ((n = recvfrom(sockfd, (char *) buffer, MAXLINE, 0, (struct sockaddr *) &servaddr, &len)) < 0) {
                    perror("ERROR");
                    printf("Errno: %d. ", errno);
                    printf("Socket Timeout or other unexpected error\n");
                    break;
                }
                //15 characters is the length of the COMMAND_ERROR\n returned from a server
                if (n == 15) {
                    printf("That song does not exist or the server has encountered an error.\n");
                    break;
                }
                //If there is nothing left to read from the server break
                char *tempBuffer = malloc(sizeof(char *));

                //copies the first 11 chars to a temp buffer to compare if it indicates the stream is done
                memcpy(tempBuffer, buffer, 11);

                //STRING_DONE is the command sent from the server to show that the server is done sending messages
                char *STRING_DONE = "STREAM_DONE";

                //if the server hasn't sent stream done write to a file
                if (strcmp(tempBuffer, STRING_DONE) != 0) {
                    //truncate the string so that
                    char *buffRemoved = buffer + 12;
                    printf("Received frame #%d containing %d bytes.\n", frameCounter, n);
                    //print each character in the buffer, printing strings could cause errors?
                    for (int j = 0; j < n - 12; ++j) {
                        fprintf(fptr, "%c", buffRemoved[j]);
                    }
                    frameCounter++;
                } else {
                    printf("Transmission Finished.\n");
                    free(tempBuffer);
                    break;
                }

            }
            //Closes the file and free each section
            //One of these kills the program and I don't know why or how
            fclose(fptr);
//            close(sockfd);
        }


    } while (input != 3);
    printf("Exiting program and closing the socket");
    close(sockfd);
    return 0;
}

