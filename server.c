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
#include <sys/sendfile.h>

void *client_handler(void *);
void get_menu_choice(int,char*);
void send_conConfirm(int);
void send_studentID(int);
void send_serverTime(int);
void send_uname(int); 
void send_filenames(int,char *); 
static void SIGhandler(int , siginfo_t *, void *);
void get_filename_to_send(int);

struct timeval t1, t2;
int connfd = 0;
int listenfd = 0;
pthread_t sniffer_thread;

static void SIGhandler(int sig, siginfo_t *siginfo, void *context)
{

		close(connfd); 
		close(listenfd);

    gettimeofday(&t2, NULL); 

		double totalSeconds = (double) (t2.tv_usec - t1.tv_usec) / 1000000 + (double) (t2.tv_sec - t1.tv_sec) ;

		int seconds = ((int)totalSeconds % 60);
		int minutes = ((int)totalSeconds % 3600) / 60;
		int hours = ((int)totalSeconds % 86400) / 3600;
		int days = ((int)totalSeconds % (86400 * 30)) / 86400;

		printf("\n\nServer shutdown request received");
		printf ("\nTotal server up time = %d days %d hours %d minutes and %d seconds\n\n",days, hours ,minutes , seconds);


    exit(EXIT_SUCCESS);
}


int main(void)
{

	gettimeofday(&t1, NULL); 


    struct sigaction act;

    memset(&act, '\0', sizeof(act));


    act.sa_sigaction = &SIGhandler;

    act.sa_flags = SA_SIGINFO;

    if (sigaction(SIGINT, &act, NULL) == -1) {
	perror("sigaction");
	exit(EXIT_FAILURE);
    }

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


    puts("Waiting for incoming connections...");
    while (1) {
	printf("Waiting for a client to connect...\n\n");
	connfd =
	    accept(listenfd, (struct sockaddr *) &client_addr, &socksize);

	printf("\n\nConnection established with: %s\n", inet_ntoa(client_addr.sin_addr));
	printf("%s is connected on socket:  %d\n",inet_ntoa(client_addr.sin_addr),connfd);



	if (pthread_create
	    (&sniffer_thread, NULL, client_handler,
	     (void *) &connfd) < 0) {
	    perror("could not create thread");
	    exit(EXIT_FAILURE);
	}
		pthread_detach(sniffer_thread); 

	printf("Handler assigned\n\n");
    }

    exit(EXIT_SUCCESS);
} 

void *client_handler(void *socket_desc)
{
   
    int connfd = *(int *) socket_desc;

    send_conConfirm(connfd); 


	char choice[8];

    do {
				get_menu_choice(connfd,choice); 
				switch (*choice) { 
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
					printf("Disconection choice on socket: %d\n",connfd); 
	    	break;
				default:
	    		printf("Client on socket %d has been disconnected\n", connfd); 
					choice[0] = '6';
	   		break;
			}
    } while (*choice != '5' );



    shutdown(connfd, SHUT_RDWR); 
    close(connfd);

    printf("Thread %lu exiting\n", (unsigned long) pthread_self());

    return 0;
}  



void send_conConfirm(int socket)
{
    char conConfirm_string[] = "Server Connection Sucessfull..";

    size_t n = strlen(conConfirm_string) + 1;
    writen(socket, (unsigned char *) &n, sizeof(size_t));	 
    writen(socket, (unsigned char *) conConfirm_string, n);	 
} 



void send_studentID(int socNum)
{
      int fd;
    struct ifreq ifr; 

    fd = socket(AF_INET, SOCK_DGRAM, 0);


    ifr.ifr_addr.sa_family = AF_INET;


    strncpy(ifr.ifr_name, "eth0", IFNAMSIZ-1);

    ioctl(fd, SIOCGIFADDR, &ifr);

    close(fd);


    char output[25];
		strcpy(output, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));
		char *id = "-S1222204";
		strcat(output,id); 

		size_t n = strlen(output) + 1;
    writen(socNum, (unsigned char *) &n, sizeof(size_t));	 
    writen(socNum, (unsigned char *) output, n);	 
		printf("returned string of: %s\n\n",output);
  
} 


void send_serverTime(int socNum)
{
       time_t t;    

    if ((t = time(NULL)) == -1) {
	perror("time error");
	exit(EXIT_FAILURE);
    }

    
    struct tm *tm; 
    if ((tm = localtime(&t)) == NULL) {
	perror("localtime error");
	exit(EXIT_FAILURE);
    }    



    char output[30];
		strcpy(output, asctime(tm)); 

		size_t n = strlen(output) + 1;
    writen(socNum, (unsigned char *) &n, sizeof(size_t));	
    writen(socNum, (unsigned char *) output, n);	 
		printf("Returned string of: %s\n", asctime(tm));
 
} 


void send_uname(int socket) 
{
    struct utsname uts; 

    if (uname(&uts) == -1) {
	perror("uname error");
	exit(EXIT_FAILURE);
    }

    size_t payload_length = sizeof(uts);

    
    writen(socket, (unsigned char *) &payload_length, sizeof(size_t));	
    writen(socket, (unsigned char *) &uts, payload_length);	
		printf("System information returned\n\n");

}


void send_filenames(int socket,char *e) 
{


    struct dirent **sizelist;  
		int size = 0;
		int n; 
		int p; 
    if ((n = scandir("./upload", &sizelist, NULL, alphasort)) == -1) 
	{perror("scandir");
	size = 40;} 
    else {
	while (n--) {
    if (sizelist[n]->d_name[0] != '.' && sizelist[n]->d_name[strlen(sizelist[n]->d_name)-1] != '~') { 
			size = size + 1 + (strlen(sizelist[n]->d_name));		
			    }		
	    free(sizelist[n]);

	}
	free(sizelist);		
    }




    struct dirent **namelist;
    char fullstring[size]; 
	  fullstring[0] = '\0';
		const char s[2] = "*";
    if ((p = scandir("./upload", &namelist, NULL, alphasort)) == -1)
{
	perror("scandir");
	strcat(fullstring, "Server Directory is not set up!"); 
}  
  else {
	while (p--) {
    if (namelist[p]->d_name[0] != '.' && namelist[p]->d_name[strlen(namelist[p]->d_name)-1] != '~') { 
			strncat(fullstring,namelist[p]->d_name,255); 
			strncat(fullstring,s,2); 
			}
	    free(namelist[p]);	

	}
	free(namelist);	
    }


		

    size_t o = strlen(fullstring) + 1;
    writen(socket, (unsigned char *) &o, sizeof(size_t));	
    writen(socket, (unsigned char *) fullstring, o);	
		printf("Server files list returned\n\n");

}

 

void get_menu_choice(int socket,char *e)
{

    size_t payload_length;

    size_t n =
	readn(socket, (unsigned char *) &payload_length, sizeof(size_t)); 

		if(n != 0) 
		{

    printf("Incoming payload length of size: %zu (%zu bytes)\n", payload_length, n); 
    n = readn(socket, (unsigned char *) e, payload_length); 
		}
		else
		{
			*e = -1;
		}

}  
