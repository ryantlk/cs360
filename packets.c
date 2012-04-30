#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "movePacket.h"

#define SENDER	 0
#define RECEIVER 1
#define TIME_OUT 20
#define DEFAULT_PORT			49999
#define DEFAULT_BLOCK_SIZE		1024
#define DEFAULT_BLOCK_COUNT		1000
#define DEFAULT_OS_RECV_BUFFER	65536
#define DEFAULT_OS_XMIT_BUFFER	8096

typedef struct packet{
    int position;
    unsigned char buffer[1025];
    int checksum;
    int size;
	int success;
}packet;

void transmit(char* host,int port,int size,int count,int recvBuffer,int sendBuffer, char* filename) {
	char* errorString = NULL;
	packet* thepacket = malloc(sizeof(packet));
	if (packetSetupActive(host,port,&errorString) < 0) {
		fprintf(stderr,"Error initializing active end: %s\n",errorString);
		exit(1);
	}
	if (packetSetOptions(TIME_OUT,recvBuffer,sendBuffer,&errorString) < 0) {
		fprintf(stderr,"Error setting options: %s\n",errorString);
		exit(1);
	}
	int thefile = open(filename, O_RDONLY);
	packet* recvpacket = malloc(sizeof(packet));
	int errors = 0;
	unsigned int i = 0;
	int j;
	int n;
	int m;
	while((j = read(thefile, thepacket->buffer, size)) > 0){
		i++;
		printf("%d\n", j);
		int sum = 0;
		for(int k = 0; k < 1025; k++){
			sum += thepacket->buffer[k];
		}
		sum = sum % (1 << 8);
		thepacket->size = j;
		thepacket->position = i;
		thepacket->checksum = sum;
	    printf("Sending packet %d\n",i);
		for(int z = 0; z < 10; z++){
			if ((n = packetSend(thepacket,sizeof(packet),&errorString)) < 0) {
				fprintf(stderr,"Transmission Error Encountered: %s\n",errorString);
				break;
			}
			if(n == 0)
				m = 1;
			if ((n = packetGet(recvpacket,sizeof(packet),&errorString)) < 0) {
				fprintf(stderr,"Error getting response message: %s\n",errorString);
			}
			if(n >= 0 && m == 1)
				break;
			if(i == 9){
				fprintf(stderr, "Error getting response message: %s\n",errorString);
				exit(1);
			}
		}
		if(recvpacket->success == 0){
			lseek(thefile, -size, SEEK_CUR); //rewinding to resend last packet since it was corrupted
			continue;
		}
		if (recvpacket->position != i)
			errors++;
	}
	printf("%d\tPackets sent.\n",i);
	printf("%d\tResponse errors.\n",errors);
}

void receive(int port, int size,int count,int recvBuffer,int sendBuffer, char* filename) {
	char* errorString = NULL;
	if (packetSetupPassive(port,&errorString) < 0) {
		fprintf(stderr,"Error initializing recevier: %s\n",errorString);
		exit(1);
	}
	if (packetSetOptions(TIME_OUT,recvBuffer,sendBuffer,&errorString) < 0) {
		fprintf(stderr,"Error setting options: %s\n",errorString);
		exit(1);
	}
	unsigned char* bitmap = calloc(count,1);
	packet* thepacket = malloc(sizeof(packet));
	int badPackets = 0;
	int outOfOrder = 0;
	int lastPacketNum = 0;
	int i;
	int n;
	int errors = 0;
	int sum = 0;
	int thefile = open(filename, O_WRONLY | O_CREAT);
	for (i = 0; i < count; i++) {
		for(int j = 0; j < 10; j ++){ //repeats 10 times breaks out if packet is recieved and response is sent correctly
			n = packetGet(thepacket,sizeof(packet),&errorString);
			if (n < 0) {
				fprintf(stderr,"Reception Error Encountered: %s\n",errorString);
				continue;
			}
			sum = 0;
			for(int k = 0; k < 1025; k++){
				sum += thepacket->buffer[k];
			}
			sum = sum % (1<<8);
			printf("%d %d\n", thepacket->checksum, sum);
			if (thepacket->checksum != sum){
				errors++;
				i--;
				thepacket->success = 0;
				break;
			}
			else
				thepacket->success = 1;
			if (errors) badPackets++;
			if (packetSend(thepacket,sizeof(packet),&errorString) < 0) {
				if (n < 0) {
					fprintf(stderr,"Error sending acknowledgement: %s\n",errorString);
					continue;
				}
			}
			if(j == 9){
				printf("Timeout");
				exit(1);
			}
			break;
		}
		if(thepacket->checksum != sum)
			continue;
		printf("writing");
		write(thefile, thepacket->buffer, thepacket->size);
		unsigned int packetNum = thepacket->position;
		if (packetNum < count + 1) bitmap[packetNum - 1]++;
		printf("Got packet %u\n",packetNum);
		if (packetNum != lastPacketNum + 1) outOfOrder++;
		lastPacketNum = packetNum;
	}
	printf("%d\tPackets received.\n",i);
	printf("%d\tPackets out of order.\n",outOfOrder);
	printf("%d\tPackets with data errors.\n",badPackets);
	int missing = 0;
	for (i = 0; i < count; i++) if (!bitmap[i]) missing++;
	free(bitmap);
	printf("%d\tMissing packets.\n",missing);
}

int getParam(int arg, char* argv[]) {
	int num;
	if (sscanf(argv[arg],"%i",&num) == 1) return num;
	fprintf(stderr,"Error: expected an integer parameter, but found '%s'\n",argv[arg]);
	exit(1);
}

int main(int argc, char* argv[]) {
	int arg = 1;
	int direction = -1;
	int portNumber = DEFAULT_PORT;
	int blockSize = DEFAULT_BLOCK_SIZE;
	int blockCount = DEFAULT_BLOCK_COUNT;
	int xmitBuffer = DEFAULT_OS_RECV_BUFFER;
	int recvBuffer = DEFAULT_OS_XMIT_BUFFER;
	char* hostName = NULL;
	char* pathname = NULL;
	
	if (argv[arg] && strcmp(argv[arg], "-h") == 0) {
		printf("\tUsage:\t%s -S|-R [-P portnumber] [-B blockSize] [-N blockcount] [-[X|I] buffersize] hostname\n\n",argv[0]);
		printf("\t\t-S or -R: indicates whether this process is a sender or receiver.\n");
		printf("\t\tportnumber: is the port on which the process will connect to hostname.\n");
		printf("\t\tblockSize: the number of bytes to receive/send at a time.\n");
		printf("\t\tblockcount:	the number of blocks to send/receive.\n");
		printf("\n\t\tThis program can operate as a sender or receiver.\n");
		printf("\t\tAs a receiver, it will connect to the indicated hostname\n");
		printf("\t\tand port and attempt to read blockcount number of packets\n");
		printf("\t\t(each of size blockSize, or less).  It will check for\n");
		printf("\t\tdata integrity (via predetermined data) and correct ordering and\n");
		printf("\t\twill report its results when blockcount packets have arrived.\n");
		printf("\t\tAs a sender, this program will send blockcount packets of size\n");
		printf("\t\tblockSize to portnumber on hostname.  It will include a sequence\n");
		printf("\t\tnumber, the packet size and sequential data.\n");
		printf("\t\tbuffersize is the number of bytes to be allocated by the OS\n");
		printf("\t\tfor incoming packets not yet read, or if sending, for outgoing\n");
		printf("\t\tpackets that have not yet been transmitted onto the network.\n\n");
		printf("\t\tIf portnumber, blockSize or blockcount are not specified, they will\n");
		printf("\t\thave default values of 49999, 1024 and 1000 respectively.\n");
		exit(0);
	}
	if (argv[arg] && argv[arg][0] == '-' && argv[arg][1] =='S') {
		direction = 0;
	} else if (argv[arg] && argv[arg][0] == '-' && argv[arg][1] == 'R') {
		direction = 1;
	} else {
		fprintf(stderr,"Error: Expecting '-' and S/R option as first parameter.\n");
		exit(1);
	}
	arg++;
	while (argv[arg] && argv[arg][0] == '-') {
		switch (argv[arg][1]) {
			case 'P':
				portNumber = getParam(++arg,argv);
				break;
			case 'B':
				blockSize = getParam(++arg,argv);
				break;
			case 'N':
				blockCount = getParam(++arg,argv);
				break;
			case 'X':
				xmitBuffer = getParam(++arg,argv);
				break;
			case 'I':
				recvBuffer = getParam(++arg,argv);
			default: {
				fprintf(stderr,"Error: unrecognized parameter option '%c'\n",argv[arg][1]);
				exit(1);
			}
		}
		arg++;
	}
	pathname = argv[arg];
	printf("%s\n", pathname);
	arg++;
	if (direction == SENDER && argv[arg] != NULL) {
		hostName = argv[arg];
	} else if (direction == SENDER) {
		fprintf(stderr,"Error: hostname not specified\n");
		exit(1);
	}
	
	if (blockSize < 8) blockSize = 8;
	if (blockCount < 1) blockCount = 1;
	
	if (direction == SENDER) printf("Sending ");
	else if (direction == RECEIVER) printf("Receiving ");
	printf("%d packets of size %d ",blockCount,blockSize);
	if (direction == SENDER) printf("to ");
	else if (direction == RECEIVER) printf("from ");
	printf("host ? port %d with an OS buffers of %d/%d.\n",portNumber,recvBuffer,xmitBuffer);
	
	if (direction == SENDER) {
		transmit(hostName,portNumber,blockSize,blockCount,recvBuffer,xmitBuffer, pathname);
	} else {
		receive(portNumber,blockSize,blockCount,recvBuffer,xmitBuffer, pathname);
	}
	printf("Done.\n");
	return 0;
}
