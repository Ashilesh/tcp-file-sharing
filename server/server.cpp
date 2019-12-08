#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>	
#include<unistd.h>		//read and write function
#include<string.h>
#include<cstdlib>	
#include<fstream>		//file handling operation

/*
	Server has two sockets.
	1. listening socket.s
	2. client socket.
*/
void ls(int, char[]);
void download_c(int , char[]);
void upload_c(int, char[]);

int main(){

	int serv_fd,opt = 1, client_fd;

	char msg[] = "Message received!\n";

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

	memset(buffer,'\0',sizeof(buffer));



	while(true)
	// read content send by client and store it into buffer.
	{
		read(client_fd, buffer, 1024);
		std::cout<<"Client - "<<buffer<<std::endl;

		if(strcmp(buffer,"exit") == 0){
			std::cout<<"exiting connection!\n";
			break;
		}
		else if(strncmp(buffer,"$ls",3) == 0){
			std::cout<<"list called!"<<std::endl;
			ls(client_fd, &buffer[1]);
		}
		else if(strncmp(buffer,"$upload",7) == 0){
			std::cout<<"upload";
			std::cout<<"file -"<<&buffer[8]<<std::endl;
			upload_c(client_fd, &buffer[8]);
		}
		else if(strncmp(buffer,"$download",9) == 0){
			download_c(client_fd, &buffer[10]);
		}
		else
			// send the msg to client.
			// alternate
			// send(client_fd, &msg , sizeof(msg), 0)
			write(client_fd, msg, strlen(msg));

		

		memset(buffer,'\0',strlen(buffer));
	}


	return 0;
}


void ls(int client_fd, char command[]){
	char buffer[1024] = {0};

	strcat(command, " > list.txt");

	system(command);

	std::ifstream fin;
	fin.open("list.txt");

	while(!fin.eof()){
		fin.getline(buffer, 1024);
		buffer[strlen(buffer)] = '\n';
		std::cout<<buffer<<std::endl;
		write(client_fd, buffer, strlen(buffer));
		memset(buffer,'\0',strlen(buffer));
	}

	char terminate[] = "#";

	write(client_fd, terminate, strlen(terminate));

	fin.close();
}

void download_c(int client_fd, char file[]){

	int buffer[1024] = {0};
	int buf = 0, bufffer_counter = 0, buf_2;
	std::ifstream fin;

	fin.open(file, std::ios::binary);

	if(!fin){
		buffer[0] = -2;
		write(client_fd, buffer, sizeof(buffer));
		std::cout<<"File not found!"<<std::endl;
	}
	else{
		while((buf = fin.get()) != -1 && !fin.eof()){
			
			if(bufffer_counter == 1024){
				write(client_fd, buffer, sizeof(buffer));

				std::cout<<"between write and read"<<std::endl;
				
				read(client_fd, &buf_2, sizeof(buf_2));

				std::cout<<" buf_2 : "<<buf_2<<std::endl;

				std::cout<<fin.tellg()<<std::endl;

				buffer[0] = buf;
				bufffer_counter = 1;
			}
			else{
				buffer[bufffer_counter++] = buf;
			}
		}

		buffer[bufffer_counter % 1024] = -1;

		write(client_fd, buffer, sizeof(buffer));

		

		std::cout<<"transmission ended !"<<std::endl;
	}

	fin.close();
}

void upload_c(int client_sock, char file[]){

	std::cout<<"in upload"<<std::endl;

	int buffer[1024] = {0}, buffer_counter = 0;
	std::ofstream fout;

	read(client_sock, buffer, sizeof(buffer));



	if(buffer[0] == -2)
		std::cout<<"No File exist!"<<std::endl;
	else{
		
		fout.open(file);
		std::cout<<"buffer -"<<buffer[0]<<std::endl;

		while(buffer[buffer_counter] != -1){

			fout.put((char)buffer[buffer_counter++]);

			if(buffer_counter == 1024){
				read(client_sock, buffer, sizeof(buffer));
				buffer_counter = 0;
			}
		}

		std::cout<<"file received !"<<std::endl;
		fout.close();
	}

}