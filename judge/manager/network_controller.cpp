#include "network_controller.h"

NetworkController::NetworkController(const Piece::Colour & newColour, const char * newName) : Controller(newColour, newName), sfd(-1)
{
	//struct protoent * tcp = getprotobyname("tcp");
 
    	sfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sfd < 0)
	{
		perror("NetworkController::NetworkController - Error creating TCP socket");
		return;
	}
}

NetworkController::~NetworkController()
{
	if (Valid())
	{
		if (shutdown(sfd, SHUT_RDWR) == -1)
		{
			perror("NetworkController::~NetworkController - shutting down socket... ");
			close(sfd);
			sfd = -1;
		}
	}
	close(sfd);
}

void NetworkController::Message(const char * string)
{
	
}

Server::Server(const Piece::Colour & newColour, const char * newName) : NetworkController(newColour, newName)
{
	struct   sockaddr_in name;
	char   buf[1024];
	int    cc;

	
	name.sin_family = AF_INET;
	name.sin_addr.s_addr = htonl(INADDR_ANY);
	name.sin_port = htons(NetworkController::port);

	if (bind( sfd, (struct sockaddr *) &name, sizeof(name) ) < 0)
	{
		perror("Server::Server - Error binding socket");
		close(sfd); sfd = -1; return;
	}

	if (listen(sfd,1) < 0)
	{
		perror("Server::Server - Error listening on socket");
		close(sfd); sfd = -1; return;
	}
	int psd = accept(sfd, 0, 0);
	close(sfd);
	sfd = psd;
	if (sfd < 0)
	{
		perror("Server::Server - Error accepting connection");
		close(sfd); sfd = -1; return;
	}


	for(;;) 
	{
		cc=recv(sfd,buf,sizeof(buf), 0) ;
		if (cc == 0) exit (0);
		buf[cc] = '\0';
		printf("message received: %s\n", buf);
	}
}


Client::Client(const Piece::Colour & newColour, const char * newName, const char * address) : NetworkController(newColour, newName)
{
	struct	sockaddr_in server;
	struct  hostent *hp;


	server.sin_family = AF_INET;
	hp = gethostbyname("127.0.0.1");
	bcopy ( hp->h_addr, &(server.sin_addr.s_addr), hp->h_length);
	server.sin_port = htons(NetworkController::port);

	if (connect(sfd, (struct sockaddr *) &server, sizeof(server)) < 0)
	{
		perror("Client::Client - Error connecting to server at address %s");
		close(sfd); sfd = -1;
		return;
	}

        for (;;) {
	   send(sfd, "HI", 2,0 );
           sleep(2);
        }

}
//EOF


