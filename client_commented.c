/*
#===================================================================================
#
# FILE: client.c - client system for multi-threaded server using readn() and writen()
#
# USAGE: --
#
# DESCRIPTION: Client side code for Multi-threaded TCP server and client system -
# Launches and connects to predetermned server. Allows interactive menu system to
# Call server functions that are returned to the client	
#
# NOTES:--
# AUTHOR: Daryl McAllister , S1222204
# VERSION: 1.0
# CREATED: 10.10.2017 - 11.12.17
#===================================================================================
*/

//Includes
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include "rdwrn.h"
#include <sys/utsname.h>

//Used for the input decleration of size
#define INPUTSIZ 10


//Function Declarations
void get_systemInfo(int);
void get_conConfirm(int socket);
void display_menu();
void send_menu_option(int, char);
void get_studentID(int);
void get_serverTime(int);
void get_fileNames(int);
void get_file_to_send(int);


/*
#=== FUNCTION ================================================================
# NAME: Main
# DESCRIPTION: Main function
# Connects to server and deals with main switch statement
#===============================================================================
*/
int main(void)
{

//Socket set up and server connection 
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;

    // IP address and port of server we want to connect to
    serv_addr.sin_port = htons(50001);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // try to connect...
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");
//end connection area


    get_conConfirm(sockfd); //gets server connection string
		
		char input;
    char name[INPUTSIZ];
		display_menu(); //displays menu 

    do {
			printf("option> ");
			fgets(name, INPUTSIZ, stdin);	// get the value from input
			name[strcspn(name, "\n")] = 0;
			input = name[0]; //takes only the first character 
			if (strlen(name) > 1)
	   		 input = 'x';	// set invalid if input more 1 char
				switch (input) { //switch to call correct function on input
				case '0':
	   			display_menu();
	    	break;
				case '1':
	    		send_menu_option(sockfd,input);
					get_conConfirm(sockfd);
	    	break;
				case '2':
	    		send_menu_option(sockfd,input);
					get_serverTime(sockfd);
	    	break;
				case '3':
					send_menu_option(sockfd,input);
					get_systemInfo(sockfd);
	    	break;
				case '4':
	    		send_menu_option(sockfd,input);
					get_fileNames(sockfd);
	    	break;
				case '5':
	    		send_menu_option(sockfd,input); //sends the end message to disconnect 
	    	break;
				default:
	    		printf("Invalid choice - 0 displays options...!\n"); //defaults to an error choice
	   		break;
			}
    } while (input != '5');


    // *** make sure sockets are cleaned up

    close(sockfd);

    exit(EXIT_SUCCESS);

} // end main()


/*
#=== FUNCTION ================================================================
# NAME: display_menu
# DESCRIPTION: Just displays the menu out to the user
#===============================================================================
*/
void display_menu()
{
    printf("0. Display menu\n");
    printf("1. IP and ID Return\n");
    printf("2. Server Time Return\n");
    printf("3. System Information\n");
    printf("4. Display server files\n");
    printf("5. Exit\n\n");
}

/*
#=== FUNCTION ================================================================
# NAME: get_systemInfo
# DESCRIPTION: Gets the system information struct from the server and outputs it
#===============================================================================
*/
void get_systemInfo(int socket)  
{
		struct utsname uts; //sets up uts struct
    size_t payload_length;

    // get the struct
    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));	 //gets payload length  
    readn(socket, (unsigned char *) &uts, payload_length); //gets payload 

		//prints out seperate information from uts struct
    printf("Node name:    %s\n", uts.nodename);
    printf("System name:  %s\n", uts.sysname);
    printf("Release:      %s\n", uts.release);
    printf("Version:      %s\n", uts.version);
    printf("Machine:      %s\n\n", uts.machine);

} 

/*
#=== FUNCTION ================================================================
# NAME: send_menu_option
# DESCRIPTION: Sends the inputted menu choice to the server
#===============================================================================
*/
void send_menu_option(int socket, char input)  
{

    size_t payload_length = sizeof(input);

    // send the char
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));	//gets payload length
    writen(socket, (unsigned char *) &input, payload_length);	 		//gets payload 

} 



// how to receive a string
void get_conConfirm(int socket)
{
    char conConfirm_string[32];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	//gets payload  length
    readn(socket, (unsigned char *) conConfirm_string, k); //gets payload 

    printf("%s\n\n", conConfirm_string);

} 

/*
#=== FUNCTION ================================================================
# NAME: get_studentID
# DESCRIPTION: gets the requested string from the server and displays it
#===============================================================================
*/
void get_studentID(int socket)
{
    char studentID[26];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	//gets payload length
    readn(socket, (unsigned char *) studentID, k); //gets payload 

    printf("The requested IP and ID is: %s\n\n", studentID);
} 

/*
#=== FUNCTION ================================================================
# NAME: SIGhandler
# DESCRIPTION: gets the local server time from server and displays it
#===============================================================================
*/
void get_serverTime(int socket)
{

    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	//gets payload  length
		char dateTime[k];
    readn(socket, (unsigned char *) dateTime, k); //gets payload 

    printf("The server DateTime is: %s\n\n", dateTime);
} 

/*
#=== FUNCTION ================================================================
# NAME: SIGhandler
# DESCRIPTION: Gets the filenames from the server and breaks down the string
# and displays it as a list
#===============================================================================
*/
void get_fileNames(int socket)
{
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	//gets payload length
		char filenames[k];
    readn(socket, (unsigned char *) filenames, k); //gets payload 

if((strlen(filenames)) > 1) //if the string is populated
{

   char *token;
		const char s[2] = "*"; //sets seperator character
   printf("The following files are found on the server: \n");
   /* get the first token */
   token = strtok(filenames, s);
   
   /* walk through other tokens */
   while( token != NULL ) {
      printf( "%s\n", token );
    
      token = strtok(NULL, s);
   }
printf("\n");
}
else
{
printf("There is no files on the server\n\n");
}


} 

 
