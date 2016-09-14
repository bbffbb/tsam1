#include <assert.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

/*
The pac
*/
/*            
    pac = top  
           2byte|    2byte|max512byte | 1byte      
              op|errorcode|errormsg   | 0
    */

struct pacInfo
{
    /*The data we need to*/
    int sockfd;
    char message[516];
    char payload[512];
    int sizeOfData;
    struct sockaddr server, client;
    char options[2];
};

//values for time
struct timeval timeStart;
struct timeval timeStop;
extern int timeval;
int timepassed;
int maxtime = 500;


void sendErr( int errorType,struct pacInfo pac)
{
    
    memset(&pac.message[0],0,sizeof(pac.message)); // clears array
    //I foundthis union in piazza. 
    union {unsigned short opCode; char arr[2];} converted; //makes tuple for convertions
    converted.opCode = htons(5); // networkbyte order
    pac.message[0] = converted.arr[0];
    pac.message[1] = converted.arr[1];

    converted.opCode = htons(errorType); // networkbyte order
    pac.message[2] = converted.arr[0];
    pac.message[3] = converted.arr[1];
    //TODO need to check with error msg with char* errorMsg need to reconsider this sento msg
    //memcpy(&pac.message[4],&errorMsg,sizeof(errorMsg));

    /*  
    n = Specifies the size of the message in bytes.
    len = Specifies the length of the sockaddr structure pointed to by the dest_addr argument.
    4 = the size of hedder i cant se a reson why i would not just have that hardcoded. 
    */
    sendto(pac.sockfd, pac.message, (size_t) sizeof(4), 0,(struct sockaddr *) &pac.client, sizeof(pac.client));  
}


// LETS SEND SUM DATA !
// OMG HOW DO I SEND DATA ?
/*
 what do we need in the void fuction ? 
    1.1 we need the file name -check
    1.2 we need the structure for the packet - check
    1.3 we need the number of the packet - l8er
    1.3 send the pac -CHECK! 
*/

void sendPac ( unsigned short pacNum, char srcFile[], struct pacInfo pac)
{
   
    FILE *fileTosend = fopen(srcFile, pac.options);
    memset(&pac.message[0],0,sizeof(pac.message));// clears array message
    memset(&pac.payload[0],0,sizeof(pac.payload));//clears array payload
    if ( fileTosend == NULL) // this if is off need to check if there is time
    {
        sendErr(1, pac);
        return; 
    }
    if ( fileTosend != NULL)
    {
        if (pacNum > 1)
        {
            fseek (fileTosend, (pacNum-1)*512, SEEK_SET );
        }
    pac.sizeOfData = fread(&pac.payload, 1, 512,fileTosend);
    fclose(fileTosend);
    }

    strcpy(&pac.message[4], pac.payload);
    union {unsigned short opCode; char arr[2];} converted;
    pac.message[0] = converted.arr[0];
    pac.message[1] = converted.arr[1];

    converted.opCode = htons(pacNum); // networkbyte order
    pac.message[2] = converted.arr[0];
    pac.message[3] = converted.arr[1];
    sendto(pac.sockfd, pac.message, (size_t) sizeof(516), 0,(struct sockaddr *) &pac.client,(socklen_t) sizeof(pac.client));
    
}
/*
 well now we can send stuff but just one time lets devide the data.
 I think I will only need a counter for the data but lets see 
*/
// data management
unsigned short next(char message[])
{    // this is straight from piazza
    union {unsigned short opCode; char arr[2];} converted;
    converted.arr[0] = message[2];
    converted.arr[1] = message[1];
    converted.opCode = ntohs(converted.opCode);
    return converted.opCode + 1;
}

int main(int argc, char *argv[])
{

    /*
    firs input: portnumber.
    second input: name of dir.
    will be done on server. 
    */
    //fprintf(stdout, "argc is%d", argc);
    if (argc  != 3 )
    {
       perror("Please enter ./tftpd.c portnumer directory");
       exit(1);
    }
    if (chdir(argv[2]) != 0)// dir not found that client has asked for
    {
        perror("Directory is not found");
        exit(1);
    }
    int port = atoi(argv[1]);
    // new instans of pacInfo
    struct pacInfo pac;
    pac.options[0] = 'r';
    pac.options[1] = ' ';
    char namOfSrc = [30];

    // is that all for ther server port or am i missin something

    /* Create and bind a UDP socket */
    pac.sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&pac.server, 0, sizeof(pac.server));
    pac.server.sin_family = AF_INET;

    /* Network functions need arguments in network byte order instead
     * of host byte order. The macros htonl, htons convert the
     * values.
     */
    pac.server.sin_addr.s_addr = htonl(INADDR_ANY);
    pac.server.sin_port = htons(port);
    bind(pac.sockfd, (struct sockaddr *) &pac.server, (socklen_t) sizeof(pac.server));

    for (;;) {
                // new instans of timeval
                struct timval tv;
                // set for scoket
                fd_set rfds;
                int returnValue;

                //most of what is left in main is done in deamatimi.
                FD_ZERO(&rfds);
                FD_SET(pac.sockfd, &rfds);

                // Time management
                tv.tv_sec = 10;
                tv.tv_usec = 0;
                returnValue = select(pac.sockfd + 1, &rfds, NULL, NULL, &tv);
                if( returnValue == -1)
                {
                    perror("Oops something whent wrong fix");

                }
                else if (returnValue > 0)
                {
                    assert(FD_ISSET(pac.sockfd, &rfds)); // Right src of data found. 
                    socklen_t len = (socklen_t) sizeof(pac.client);
                    recvfrom(pac.sockfd, pac.message,sizeof(pac.message) - 1, 0,(struct sockaddr *) &pac.client,&len);
                    int  number = (int)pac.message[1];
                    switch (number)
                    {
                        case 1:
                            strcpy(namOfSrc, &pac.message[2]);
                            if(strstr(namOfSrc, "../") != NULL)
                            {
                            sendErr(1, pac);
                            }
                            if(pac.message[strlen(namOfSrc) + 3] == 'o')
                            {
                            pac.options[1] = 'b';
                            }
                            sendPac(1, namOfSrc, pac);
                            break;
                        case 2: //Put
                          // Send ERROR pac
                            sendErr(2,pac);
                            break;
                        case 3://Data
                            // Send Erro pac
                            sendErr(2,pac);
                            break;
                        case 4: //Ack
                            sendPac(next(pac.message), namOfSrc, pac);
                            break;
                        case 5://Error
                            sendErr(2,pac); // þurfum að ákveða hvaða error við viljum
                            // nokkuð viss um að við eigum ekki að gera neitt.
                            break;
                        default: 
                         sendErr(4,pac);
                        break;// client fault

                    }

                }
                else
                {   
                    fprintf(stdout, "10 sec passed and no message\n");
                    fflush(stdout);
                }
    }
}
