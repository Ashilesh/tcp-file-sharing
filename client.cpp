#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>	//inet_pton function

int main(){

	int client_sock;

	struct sockaddr_in serv_addr;

	char msg[1024];

	char buffer[1024];

	client_sock = socket(AF_INET, SOCK_STREAM, 0);
	/*
		Socket Creation

			Socket is API between application and network.

			serv_fd is a socket descriptor(like a file handle)

			int socket(int domain, int type, int protocol)
			domain = communication domain
					IPv4 - AF_INET, IPv6 - AF_INET6
			type = communication type
					TCP - SOCK_STREAM, UDP - SOCK_DGRAM,
					SOCK_SEQPACKET - provide sequenced, reliable and 
					bidirectional connection-mode transmission path for
					record.
			protocol = protocol value for IP which is 0.
						it will appear in protocol field of 
						in IP header of packet.

						check /etc/protocols and 
						man protocols
	*/

	if(client_sock == 0)
	{
		std::cout<<"Error - socket creation failed!";
		exit(EXIT_FAILURE);
		//EXIT_FAILURE has value of 8.
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(6001);
	//converting byte ordering to 'big endian', ie converting into network
	//byte order as our system is 'little endian'.
	//4f52 big endian - 4f 52 little endian - 52 4f having postion 1 and 2 respectively

	if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
	/*
		convert IPv4 and IPv6 address from text to binary

		int inet_pton(int address_family, const char* source, void* destination)

		this function converts 'source' into network address structure and then 
		copies the network address structure to 'destination'.
 	*/
	{
		std::cout<<"Error - Invalid address!";
		return -1;
	}

	if(connect(client_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout<<"Error - Connection failed";
		return -1;
	}

	while(true)
	{

		std::cout<<"Client: ";
		std::cin.getline(msg,sizeof(msg));

		write(client_sock, msg, strlen(msg));

		if(strcmp(msg,"exit") == 0){
			std::cout<<"exiting connection!\n";
			break;
		}

		read(client_sock, buffer, 1024);

		std::cout<<"\nServer: "<<buffer;

	}

	return 0;
}