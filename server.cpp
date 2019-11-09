#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>	
#include<unistd.h>		//read and write function
#include<string.h>		

/*
	Server has two sockets.
	1. listening socket.s
	2. client socket.
*/


int main(){

	int serv_fd,opt = 1, client_fd;

	char* msg;

	struct sockaddr_in address;
	//structure for handling internet addresses.

	int addr_len = sizeof(address);

	char buffer[1024] = {0};

	serv_fd = socket(AF_INET, SOCK_STREAM, 0);
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

	if(serv_fd == 0){
		std::cout<<"Error - Socket Creation failed.";
		exit(EXIT_FAILURE);
		//EXIT_FAILURE has value of 8.
	}

	if(setsockopt(serv_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	/*
		set socket options.
		this is optional but helps in reuse of address and port.
		int setsockopt(int socket,int level, int option_name, const void *option_value,
		socklen_t option_len)
		
		setsockopt shall set the option specified by 'option_name' argument at the
		level specified by 'level' to the value pointed by 'option_value'.
	*/
	{

		std::cout<<"Error - problem in setting options for socket.";
		exit(EXIT_FAILURE);
	}

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	//INADDR_ANY (0.0.0.0) - binds the socket to all availabel interfaces.
	//use inet_addr("127.0.0.1") [INADDR_LOOPBACK] for binding to localhost only.

	address.sin_port = htons(6001);
	//converting byte ordering to 'big endian', ie converting into network
	//byte order as our system is 'little endian'.
	//4f52 big endian - 4f 52 little endian - 52 4f having postion 1 and 2 respectively.

	if(bind(serv_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
	/*
		bind the socket to the address and port no specified in address.
	*/
	{
		std::cout<<"Error - problem in binding.";
		exit(EXIT_FAILURE);
	}	

	if(listen(serv_fd, 3) < 0)
	/*
		the second value defines max length to which serv_fd pending connection
		will grow.If the limit exceeded then client receives error of ECONNREFUSED.

	*/
	{
		std::cout<<"Error - problem in listening.";
		exit(EXIT_FAILURE);
	}

	if((client_fd = accept(serv_fd,(struct sockaddr *)&address, (socklen_t*)&addr_len)) < 0)
	/*
		accept function extracts first connection request on the queue of pending 
		connection for the listening socket. It returns a new file descriptor reffering
		to new socket.
		accept is system call.
		if the accept is marked as blocking then it blocks a caller until new connection
		is present.
	*/
	{
		std::cout<<"Error - problem in accepting client connection.";
		exit(EXIT_FAILURE);
	}

	std::cout<<"Connection established! \n";

	//Connection between client and server is established at this point.

	while(true)
	// read content send by client and store it into buffer.
	{
		read(client_fd, buffer, 1024);
		std::cout<<"Client - "<<buffer<<std::endl;

		for(int i = 0; i < strlen(buffer); i++)
			std::cout<<(int)buffer[i]<<"-";

		


		std::cout<<std::endl;

		if(strcmp(buffer,"exit") == 0){
			std::cout<<"exiting connection!\n";
			break;
		}

		msg = (char*)"server acknowledge!\n\0";
		std::cout<<"above write.";
		write(client_fd, msg, strlen(msg));
		// send the msg to client.
		// alternate
		// send(client_fd, &msg , sizeof(msg), 0)

		memset(buffer,'\0',sizeof(buffer));
	}


	return 0;
}
