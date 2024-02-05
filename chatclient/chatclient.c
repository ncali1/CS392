#/*******************************************************************************
#* Name        : chatclient.c
#* Author      : Nicholas Cali & Kyle Henderson
#* Date        : 7/5/2021
#* Description : small functional chatclient
#* Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
#******************************************************************************/


#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>
#include "util.h"

int client_socket = -1; 
char name[MAX_NAME_LEN + 1];
char inbuf[BUFLEN + 1];
char outbuf[MAX_MSG_LEN + 1];

int handle_stdin() {
    int msg = get_string(outbuf, MAX_MSG_LEN+1);
    if (msg == TOO_LONG){
        printf("Sorry, limit your message to %d characters.\n", MAX_MSG_LEN);
    }else if(send(client_socket, outbuf, strlen(outbuf), 0) < 0) {
        fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
    }else if(strcmp(outbuf, "bye") == 0){
        printf("Goodbye\n.");
        return 1;
    }
    return 0;
} 


int handle_client_socket() {
    int bytes_recieved = recv(client_socket, inbuf, BUFLEN-1, 0);
    if (bytes_recieved < 0){
        fprintf(stderr, "Warning: Failed to receive incoming message. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }
    if (bytes_recieved == 0){
        fprintf(stderr, "\nConnection to server has been lost.\n");
        return 1;
    }
    inbuf[bytes_recieved] = '\0';
    if (strcmp(inbuf, "bye") == 0){
        printf("\nServer initiated shutdown.\n.");
        return 1;
    }
    printf("\n%s\n", inbuf);
    return 0;
} 



int main(int argc, char *argv[]) {
    if (argc != 3){
        fprintf(stderr, "Usage: %s <server IP> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // modeled code from cilent.c from class
    int bytes_recvd, ip_conv, retval = EXIT_SUCCESS;    
    struct sockaddr_in server_addr;    
    socklen_t address = sizeof(struct sockaddr_in);
    char *address_string = "127.0.0.1";

    ip_conv = inet_pton(AF_INET, address_string, &server_addr.sin_addr);
    if (ip_conv == 0){
        fprintf(stderr, "Error: Invalid IP address '%s' .\n", address_string);
        retval = EXIT_FAILURE;
        goto EXIT;
    }else if(ip_conv < 0){
        fprintf(stderr, "Error: Failed to convert IP address. %s.\n", strerror(errno));        
        retval = EXIT_FAILURE;        
        goto EXIT;
    }
    
    // followed chatserver model
    int port;
    if (!parse_int(argv[2], &port, "port number")){
        return EXIT_FAILURE;
    }
    if (port < 1024 || port > 65535){
        fprintf(stderr, "Error: port must be in range [1024, 65535].\n");
    }

    while(1){
        printf("Username: ");
        fflush(stdout);
        

        memset(&server_addr, 0, address);    
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        

        int username;

        username = get_string(name, MAX_NAME_LEN+1);

        if (username != OK){
            if (username == TOO_LONG){
                printf("Sorry, limit your username to %d characters.\n", MAX_NAME_LEN);
            }
        }else{
            printf("Hello, %s. Let's try to connect to the server.\n", name);
        }

        if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            fprintf(stderr,  "Error: Failed to create socket. %s.\n", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        if (connect(client_socket,(struct sockaddr *) &server_addr, address) < 0){
            fprintf(stderr, "Error: Failed to connect to server. %s.\n", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }
        
        if ((bytes_recvd = recv(client_socket, inbuf, BUFLEN-1, 0)) < 0){
            fprintf(stderr, "Error: Failed to recieve message from server %s. \n", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        if (bytes_recvd == 0){
            printf("All connections are busy. Try again later.\n");
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        inbuf[bytes_recvd] = '\0';
        printf("\n%s\n\n", inbuf);


        //send username to server
        if (send(client_socket, name, strlen(name), 0) < 0){
            fprintf(stderr, "Error: Failed to send username to server %s. \n", strerror(errno));
            retval = EXIT_FAILURE;
            goto EXIT;
        }

        fd_set sockset;
        while (true){
            printf("[%s]: ", name);
            fflush(stdout);

            FD_ZERO(&sockset);
            FD_SET(client_socket, &sockset);
            FD_SET(STDIN_FILENO, &sockset);

            if (select(client_socket+1, &sockset, NULL, NULL, NULL) < 0){
                fprintf(stderr, "Error: select() failed. %s. \n", strerror(errno));
                retval = EXIT_FAILURE;
                goto EXIT;
            }

            if (FD_ISSET(client_socket, &sockset)){
                handle_client_socket();
            }

            if (FD_ISSET(STDIN_FILENO, &sockset)){
                handle_stdin();
            }
        }
    }    
        
EXIT:
    if (fcntl(client_socket, F_GETFD)>= 0){
        close(client_socket);
    }
    return retval;
   
}