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

#define INPUTSIZ 10

void get_systemInfo(int);
void get_conConfirm(int socket);
void display_menu();
void send_menu_option(int, char);
void get_studentID(int);
void get_serverTime(int);
void get_fileNames(int);
void get_file_to_send(int);


int main(void)
{

    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("Error - could not create socket");
	exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;

    serv_addr.sin_port = htons(50001);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)  {
	perror("Error - connect failed");
	exit(1);
    } else
       printf("Connected to server...\n");


    get_conConfirm(sockfd); 
		
		char input;
    char name[INPUTSIZ];
		display_menu(); 

    do {
			printf("option> ");
			fgets(name, INPUTSIZ, stdin);
			name[strcspn(name, "\n")] = 0;
			input = name[0]; 
			if (strlen(name) > 1)
	   		 input = 'x';	
				switch (input) { 
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
	    		send_menu_option(sockfd,input); 
	    	break;
				default:
	    		printf("Invalid choice - 0 displays options...!\n"); 
	   		break;
			}
    } while (input != '5');




    close(sockfd);

    exit(EXIT_SUCCESS);

} 



void display_menu()
{
    printf("0. Display menu\n");
    printf("1. IP and ID Return\n");
    printf("2. Server Time Return\n");
    printf("3. System Information\n");
    printf("4. Display server files\n");
    printf("5. Exit\n\n");
}


void get_systemInfo(int socket)  
{
		struct utsname uts; 
    size_t payload_length;

    
    readn(socket, (unsigned char *) &payload_length, sizeof(size_t));	  
    readn(socket, (unsigned char *) &uts, payload_length); 

		
    printf("Node name:    %s\n", uts.nodename);
    printf("System name:  %s\n", uts.sysname);
    printf("Release:      %s\n", uts.release);
    printf("Version:      %s\n", uts.version);
    printf("Machine:      %s\n\n", uts.machine);

} 


void send_menu_option(int socket, char input)  
{

    size_t payload_length = sizeof(input);

    
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));	
    writen(socket, (unsigned char *) &input, payload_length);	 		

} 


void get_conConfirm(int socket)
{
    char conConfirm_string[32];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	
    readn(socket, (unsigned char *) conConfirm_string, k); 

    printf("%s\n\n", conConfirm_string);

} 


void get_studentID(int socket)
{
    char studentID[26];
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	
    readn(socket, (unsigned char *) studentID, k); 

    printf("The requested IP and ID is: %s\n\n", studentID);
} 


void get_serverTime(int socket)
{

    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	
		char dateTime[k];
    readn(socket, (unsigned char *) dateTime, k); 

    printf("The server DateTime is: %s\n\n", dateTime);
} 


void get_fileNames(int socket)
{
    size_t k;

    readn(socket, (unsigned char *) &k, sizeof(size_t));	
		char filenames[k];
    readn(socket, (unsigned char *) filenames, k); 

if((strlen(filenames)) > 1) 
{

   char *token;
		const char s[2] = "*"; 
   printf("The following files are found on the server: \n");
   token = strtok(filenames, s);
   

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



 
