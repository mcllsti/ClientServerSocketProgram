/*
#===================================================================================
#
# FILE: server.c - multi-threaded server using readn() and writen()
#
# USAGE: --
#
# DESCRIPTION: Server side code for Multi-threaded TCP server and client system -
# Launches and waits for clients to connect through TCP and will assign them a thread.
# Allows clients to proform a list of tasks through a menu system.
#	
#
# NOTES:--
# AUTHOR: Daryl McAllister , S1222204
# VERSION: 1.0
# CREATED: 10.10.2017 - 11.12.17
#===================================================================================
*/

//Includes
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>
#include "rdwrn.h"
#include <time.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/time.h>    


// Function Declerations
void *client_handler(void *);
void get_menu_choice(int,char*);
void send_conConfirm(int);
void send_studentID(int);
void send_serverTime(int);
void send_uname(int); 
void send_filenames(int,char *); 
static void SIGhandler(int , siginfo_t *, void *);
void get_filename_to_send(int);

//Globals that allow acess in the signal handler
struct timeval t1, t2;
int connfd = 0;
int listenfd = 0;
pthread_t sniffer_thread;

/*
#=== FUNCTION ================================================================
# NAME: SIGhandler
# DESCRIPTION: Handles the intteruption of SIGINT which
# Is a call for the server to close connetions, print the runtime
#then end.
#===============================================================================
*/
static void SIGhandler(int sig, siginfo_t *siginfo, void *context)
{

		close(connfd); //closes the connection and listening sockets
		close(listenfd);



    gettimeofday(&t2, NULL); // gets the end time to calculate server uptime 

		//Does math to work out the server uptime in readable format
		double totalSeconds = (double) (t2.tv_usec - t1.tv_usec) / 1000000 + (double) (t2.tv_sec - t1.tv_sec) ;

		int seconds = ((int)totalSeconds % 60);
		int minutes = ((int)totalSeconds % 3600) / 60;
		int hours = ((int)totalSeconds % 86400) / 3600;
		int days = ((int)totalSeconds % (86400 * 30)) / 86400;

		printf("\n\nServer shutdown request received");
		printf ("\nTotal server up time = %d days %d hours %d minutes and %d seconds\n\n",days, hours ,minutes , seconds);


	//Exits sucessfully as only called on wished shutdown
    exit(EXIT_SUCCESS);
}

/*
#=== FUNCTION ================================================================
# NAME: Main
# DESCRIPTION: Main program body.
# Deals with clients connection and calling of signal handler
#===============================================================================
*/
int main(void)
{

	gettimeofday(&t1, NULL); // Sets the inital start time to help calculate server uptime later


	//Signal handler code
    struct sigaction act;

    memset(&act, '\0', sizeof(act));

    // this is a pointer to a function
    act.sa_sigaction = &SIGhandler;

    // the SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler
    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) == -1) {
	perror("sigaction");
	exit(EXIT_FAILURE);
    }
	//end of signal handler code



	//setting up of sockets for connection
    struct sockaddr_in serv_addr;
    struct sockaddr_in client_addr;
    socklen_t socksize = sizeof(struct sockaddr_in);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(50001);

    bind(listenfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if (listen(listenfd, 10) == -1) {
	perror("Failed to listen");
	exit(EXIT_FAILURE);
    }
    // end socket setup

    //Accept and incoming connection
    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n\n");
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);

	printf("\n\nConnection established with: %s\n", inet_ntoa(client_addr.sin_addr));
	printf("%s is connected on socket:  %d\n",inet_ntoa(client_addr.sin_addr),connfd);


        // third parameter is a pointer to the thread function, fourth is its actual parameter
	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}
		pthread_detach(sniffer_thread); //detaches thread to help free up resources without joining

	printf("Handler assigned\n\n");
    }

    // never reached...
    exit(EXIT_SUCCESS);
} // end main()

/*
#=== FUNCTION ================================================================
# NAME: client_handler
# DESCRIPTION: thread function - one instance of each for each connected client
# Runs the menu which allows clients to interact with the server
#===============================================================================
*/
void *client_handler(void *socket_desc)
{
    //Get the socket descriptor
    int connfd = *(int *) socket_desc;

    send_conConfirm(connfd); // calls the function to send connection confirmation string


	char choice[8];

    do {
				get_menu_choice(connfd,choice); //calls menu choice function
				switch (*choice) { //switch for the users choice
				case '1':
					printf("Executing IP/ID return for socket: %d\n",connfd);
	    		send_studentID(connfd);
	    	break;
				case '2':
					printf("Executing server time return for socket: %d\n",connfd);
	    		send_serverTime(connfd);
	    	break;
				case '3':
					printf("Executing sys info return for socket: %d\n",connfd);
	    		send_uname(connfd);
	    	break;
				case '4':
					printf("Executing file list return for socket: %d\n",connfd);
	    		send_filenames(connfd,choice);
	    	break;
				case '5':
					printf("Disconection choice on socket: %d\n",connfd); //Disconnection handling
	    	break;
				default:
	    		printf("Client on socket %d has been disconnected\n", connfd); //Defaults to client disconect on unwatned choice to stop errors
					choice[0] = '6';
	   		break;
			}
    } while (*choice != '5' );






    shutdown(connfd, SHUT_RDWR); //Shutdown and close socket
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    return 0;
}  // end client_handler()


/*
#=== FUNCTION ================================================================
# NAME: send_conConfirm
# DESCRIPTION: Sends a connetion confirmation message to the client
#===============================================================================
*/
void send_conConfirm(int socket)
{
    char conConfirm_string[] = "Server Connection Sucessfull..";

    size_t n = strlen(conConfirm_string) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	 //writes payload length
    writen(socket, (unsigned char *) conConfirm_string, n);	 //writes payload
} 

/*
#=== FUNCTION ================================================================
# NAME: send_studentID
# DESCRIPTION: Sends a concatanated string to the client.
# The string consists of a hardcoded studentID that is concatanated with the client IP
#===============================================================================
*/
void send_studentID(int socNum)
{
      int fd;
    struct ifreq ifr; //sets up ifreq struct to recieve client IP

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    /* I want to get an IPv4 IP address */
    ifr.ifr_addr.sa_family = AF_INET;

    /* I want an IP address attached to "eth0" */
    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);

    /* Display result */
    char output[25];
		strcpy(output, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
		char *id = "-S1222204";
		strcat(output,id); //Concatanates string

		size_t n = strlen(output) + 1;
    writen(socNum, (unsigned char *) &n, sizeof(size_t));	 //writes payload length
    writen(socNum, (unsigned char *) output, n);	  //writes payload
		printf("returned string of: %s\n\n",output);
  
} 

/*
#=== FUNCTION ================================================================
# NAME: send_serverTime
# DESCRIPTION: get the local time on the server and sends it as a 
# string to the client
#===============================================================================
*/
void send_serverTime(int socNum)
{
       time_t t;    // time for time working

    if ((t = time(NULL)) == -1) {
	perror("time error");
	exit(EXIT_FAILURE);
    }

    // localtime() is in standard library 
    struct tm *tm; //Time struct
    if ((tm = localtime(&t)) == NULL) {
	perror("localtime error");
	exit(EXIT_FAILURE);
    }    


    /* Display result */

    char output[30];
		strcpy(output, asctime(tm)); //copys time to the output 

		size_t n = strlen(output) + 1;
    writen(socNum, (unsigned char *) &n, sizeof(size_t));	//writes payload length
    writen(socNum, (unsigned char *) output, n);	  //writes payload
		printf("Returned string of: %s\n", asctime(tm));
 
} 

/*
#=== FUNCTION ================================================================
# NAME: client_handler
# DESCRIPTION: Declares the utsname struct that holds system information
# then sends the full struct to the client for client side processing.
#===============================================================================
*/
void send_uname(int socket) 
{
    struct utsname uts; //Declares utsname struct for system info

    if (uname(&uts) == -1) {
	perror("uname error");
	exit(EXIT_FAILURE);
    }

    size_t payload_length = sizeof(uts);

    // send new version of payload struct back to client, again length followd by actual payload
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));	//writes payload length
    writen(socket, (unsigned char *) &uts, payload_length);	//writes payload
		printf("System information returned\n\n");

}

/*
#=== FUNCTION ================================================================
# NAME: send_filenames
# DESCRIPTION: Reads in all the names of files in the upload directory 
# concatinates thease names into a large string with a seperator character
# that will be deconcatanted on the client side for a file list.
#===============================================================================
*/
void send_filenames(int socket,char *e) 
{

//First loop to dynamically get the size that will be required for the character variable later
//so that more memory is not used than nessisary
    struct dirent **sizelist; //declares dirent struct to help deal with 
		int size = 0; //will hold the size to help declare size of char variable later
		int n; //loop variable
		int p; //loop variable
    if ((n = scandir("./upload", &sizelist, NULL, alphasort)) == -1) //scans the upload directory
	{perror("scandir");
	size = 40;} //sets size to 40 if error doing so as this will be enough to hold error message
    else {
	while (n--) {
    if (sizelist[n]->d_name[0] != '.' && sizelist[n]->d_name[strlen(sizelist[n]->d_name)-1] != '~') { //excludes hidden files
			size = size + 1 + (strlen(sizelist[n]->d_name));		
			    }		
	    free(sizelist[n]);

	}
	free(sizelist);		//frees struct
    }
//end first size getting loop


//start second loop section that will get the file names
    struct dirent **namelist;
    char fullstring[size]; //declares fullstring useing the size that has been gotten from first loop
	  fullstring[0] = '\0';
		const char s[2] = "*"; //declares seperator chracter for strtok on client
    if ((p = scandir("./upload", &namelist, NULL, alphasort)) == -1)
{
	perror("scandir");
	strcat(fullstring, "Server Directory is not set up!"); //sets error message of no directory
}  
  else {
	while (p--) {
    if (namelist[p]->d_name[0] != '.' && namelist[p]->d_name[strlen(namelist[p]->d_name)-1] != '~') { //excludes hidden directories
			strncat(fullstring,namelist[p]->d_name,255); //concats the name along with allowing maximum size of file name so no erors
			strncat(fullstring,s,2); //concats seporator character
			}
	    free(namelist[p]);	

	}
	free(namelist);		//frees struct
    }
//Ends second loop

		

    size_t o = strlen(fullstring) + 1;
    writen(socket, (unsigned char *) &o, sizeof(size_t));	//writes payload length
    writen(socket, (unsigned char *) fullstring, o);	 //writes payload
		printf("Server files list returned\n\n");


}

 
/*
#=== FUNCTION ================================================================
# NAME: client_handler
# DESCRIPTION: Gets the clients menu choice that has been sent as a string and
# will be used in the client handler to call a function
#===============================================================================
*/
void get_menu_choice(int socket,char *e)
{

    size_t payload_length;

    size_t n =
	readn(socket, (unsigned char *) &payload_length, sizeof(size_t)); //gets the payload length

		if(n != 0) //ensures length is not zero as this is an errors
		{

    printf("Incoming payload length of size: %zu (%zu bytes)\n", payload_length, n); 
    n = readn(socket, (unsigned char *) e, payload_length); //gets the choice

		}
		else
		{
			*e = -1; //returns -1 which will default the switch to disconnect client
		}

}  
