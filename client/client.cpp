#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>	//inet_pton function
#include<fstream>

void ls(int);
void download(int,char[]);
void upload(int, char[]);

int main(){

	int client_sock;

	struct sockaddr_in serv_addr;

	char msg[1024]= {0};

	char buffer[1024] = {0};

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

		if(strncmp(msg,"$ls",3) == 0){
			ls(client_sock);
		}

		else if(strncmp(msg,"$download",9) == 0){
			download(client_sock, &msg[10]);
		}

		else if(strcmp(msg,"exit") == 0){
			std::cout<<"exiting connection!\n"<<std::endl;
			break;
		}

		else if(strncmp(msg,"$upload",7) == 0){
			upload(client_sock, &msg[8]);
		}
		else{
			read(client_sock, buffer, 1024);
			buffer[strlen(buffer)] = '\0';
			std::cout<<"\nServer: "<<buffer;
		}

		memset(msg,'\0',strlen(msg));

	}

	return 0;
}

void ls(int client_sock){

	char buffer[1024] = {0};
	bool cont = true;

	std::cout<<"--------File------\n";

	while(cont){
		memset(buffer,'\0',strlen(buffer));

		read(client_sock, buffer, 1024);
		
		for(int i = 0 ; i < strlen(buffer); ++i){
			if(buffer[i] == '#'){

				cont = false;
				break;
			}
			else
				std::cout<<buffer[i];
		}
			
	}
}

void download(int client_sock, char file[]){

	std::cout<<"in download"<<std::endl;

	// int buffer[1024] = {0}, 
	int buffer_counter = 0, buf_2 = 100, end_byte;
	
	char buffer[1024] = {0}, search[] = "END";

	std::ofstream fout;

	read(client_sock, buffer, sizeof(buffer));



	if(buffer[0] == -2)
		std::cout<<"No File exist!"<<std::endl;
	else{
		
		fout.open(file);

		while(buffer[1023] == 'n'){

			fout.put((char)buffer[buffer_counter++]);

			// std::cout<<"data :"<<buffer[buffer_counter]<<std::endl;

			if(buffer_counter == 1023){

				write(client_sock, &buf_2, sizeof(buf_2));

				read(client_sock, buffer, sizeof(buffer));

				// std::cout<<"between read and write"<<std::endl;
				
				buffer_counter = 0;
			}
		}

		if(buffer[1023] == 'E')
			end_byte = 1022;

		else if(buffer[1023] == 'N')
			end_byte = 1021;

		else if(buffer[1023] == 'D')
			end_byte = 1020;

		else{
			//need to imporove this
			bool cont = true;
			int i;

			while(cont){

				if(buffer[buffer_counter++] == 'E'){
					
					end_byte = buffer_counter - 1;

					for(i = 1 ; i < 3 && search[i] == buffer[buffer_counter++] ; i++);

					if(i == 3)
						cont = false;
				}
			}

			buffer_counter = 0;
		}

		if(end_byte == -1)
			end_byte++;

		while(buffer_counter != end_byte){
			
			fout.put((char)buffer[buffer_counter++]);
		}

		std::cout<<(int)buffer[buffer_counter]<<" this"<<std::endl;

		std::cout<<"file received !"<<std::endl;
		fout.close();
	}


}

void upload(int client_fd, char file[]){

	int buffer[1024] = {0};
	int buf = 0, bufffer_counter = 0;
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