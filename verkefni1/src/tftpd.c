#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <strings.h>
/*
firs input: portnumber.
second input: name of dir.
will be done on server. 
*/

//values for time
struct timeval timeStart;
struct timeval timeStop;
extern int timeval;
int timepassed;
int maxtime = 500;

struct pacInfo
{
    /*The data we need to*/
    int sockfd;
    char message[516];
    struct sockaddr client;
};
/*
static void timererror (time)
{
// hér þarf að útfæra time error klasann ss. ef við förum fyrir maxtime 
}

// Time of start of trans
static void startTime()
{
	(void)gettimeofday(&timeStart, NULL);
}

// Time of end tran
static void stopTime()
{
	(void)gettimeofday(&timeStop, NULL);
}
*/


void sendErr( int errorType,struct pacInfo packet)
{
    /*            
    packet = 
           2byte|    2byte|max512byte | 1byte      
              op|errorcode|errormsg   | 0
    */

    memset(&packet.message[0],0,sizeof(packet.message)); // clears array
    union {unsigned short opCode; char arr[2];} converted; //makes tuple for convertions
    converted.opCode = htons(5); // networkbyte order
    packet.message[0] = converted.arr[0];
    packet.message[1] = converted.arr[1];

    converted.opCode = htons(errorType); // networkbyte order
    packet.message[2] = converted.arr[0];
    packet.message[3] = converted.arr[1];
    //TODO need to check with error msg with char* errorMsg
    //memcpy(&packet.message[4],&errorMsg,sizeof(errorMsg));


        /*  
        n = Specifies the size of the message in bytes.
        len = Specifies the length of the sockaddr structure pointed to by the dest_addr argument.
        */
    sendto(packet.sockfd, packet.message, (size_t) sizeof(4), 0,
           (struct sockaddr *) &packet.client, sizeof(packet.client));  
}

int main(int argc, char *argv[])
{
	//fprintf(stdout, "argc is%d", argc);
	if (argc  != 3 )
	{
	   perror("Please enter ./tftpd.c portnumer directory");
	   exit(1);
	}

    struct pacInfo packet;
    struct sockaddr_in server;
    /* Create and bind a UDP socket */
    packet.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;

    /* Network functions need arguments in network byte order instead
     * of host byte order. The macros htonl, htons convert the
     * values.
     */
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(57596);
    bind(packet.sockfd, (struct sockaddr *) &server, (socklen_t) sizeof(server));

    for (;;) {
        /* Receive up to one byte less than declared, because it will
         * be NUL-terminated later.
         */
        socklen_t len = (socklen_t) sizeof(packet.client);
        ssize_t n = recvfrom(packet.sockfd, packet.message, sizeof(packet.message) - 1,
                             0, (struct sockaddr *) &packet.client, &len);

        packet.message[n] = '\0';
        fprintf(stdout, "Received:\n%d\n", (int)packet.message[1]);
        fflush(stdout);
        int  number = (int)packet.message[1];
        switch (number)
        {
            case 1: //Get
                fprintf(stdout, "Recived %d",number);
                // chekc if dir is accesable && send first packet
                break;
            case 2: //Put
                fprintf(stdout, "Recived %d",number);
                // Send ERROR packet
                sendErr(2,packet);
                break;
            case 3://Data
                fprintf(stdout, "Recived %d",number);
                // Send Erro packet
                sendErr(2,packet);
                break;
            case 4: //Ack
                fprintf(stdout, "Recived %d",number);
                // block number resived  && send next packet if not last
                break;
            case 5://Error
                fprintf(stdout, "Recived %d",number);
                sendErr(2,packet); // þurfum að ákveða hvaða error við viljum
                // nokkuð viss um að við eigum ekki að gera neitt.
                break;
            default: 
             sendErr(4,packet);
            break;// client fault

        }
/*
        sendto(packet.sockfd, packet.message, (size_t) n, 0,
               (struct sockaddr *) &packet.client, len);
               */
    }


	return 0;
}
