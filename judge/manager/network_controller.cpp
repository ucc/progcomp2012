#include "network_controller.h"

NetworkController::NetworkController(const Piece::Colour & newColour, const char * newName) : Controller(newColour, newName), sfd(-1)
{
	struct protoent * tcp = getprotobyname("tcp");
 
    	sfd = socket(PF_INET, SOCK_STREAM, tcp->p_proto);
	if (sfd < 0)
	{
	//	fprintf(stderr, "NetworkController::NetworkController - couldn't create a TCP socket!");
		perror("NetworkController::NetworkController - creating TCP socket... ");
		return;
	}
}

NetworkController::~NetworkController()
{
	if (Valid())
	{
		if (shutdown(sfd, SHUT_RDWR) == -1)
		{
		//	fprintf(stderr, "NetworkController::~NetworkController - Can't shutdown socket %d!", sfd);
			perror("NetworkController::~NetworkController - shutting down socket... ");
			close(sfd);
			sfd = -1;
		}
	}
	close(sfd);
}

Server::Server(const Piece::Colour & newColour, const char * newName) : NetworkController(newColour, newName)
{
	struct sockaddr_in ipa;
	ipa.sin_family = AF_INET;
	ipa.sin_port = htons(NetworkController::port);	
	ipa.sin_addr.s_addr = INADDR_ANY;
	memset(&ipa,0, sizeof ipa);

	if (bind(sfd, (struct sockaddr*)&ipa, sizeof (ipa)) == -1) //dERP DERP DERP
	{
	//	fprintf(stderr, "Server::Server - Couldn't bind to socket! Abort\n");
		perror("Server::Server - binding to socket... ");
		close(sfd);
		sfd = -1;
		return;
	}

	//Listen for at most 1 connection
	if (listen(sfd, 1) == -1)
	{
		fprintf(stderr, "Server::Server - listening failed.\n");
		close(sfd);
		sfd = -1;
		return;
	}	

	//Accept the connection
	sfd = accept(sfd, NULL, NULL);
	if (sfd < 0)
	{
		fprintf(stderr, "Server::Server - couldn't accept connection.\n");
		close(sfd);
		sfd = -1;
		return;
	}

	//We are now ready to play!
}


Client::Client(const Piece::Colour & newColour, const char * newName, const char * server) : NetworkController(newColour, newName)
{
	struct sockaddr_in ipa;
	ipa.sin_family = AF_INET;
	ipa.sin_port = htons(NetworkController::port);	
	int Res = inet_pton(AF_INET, server, &ipa.sin_addr);

	if (Res < 0)
	{
		fprintf(stderr, "Client::Client - First parameter is not a valid address family!\n");
		close(sfd);
		sfd = -1;
		return;
	}
	else if (Res == 0)
	{
		fprintf(stderr, "Client::Client - Second parameter does not contain a valid IP Address!\n");
		close(sfd);
		sfd = -1;
		return;
	}
	
	if (connect(sfd, (struct sockaddr*)&sfd, sizeof sfd) == -1)
	{
		fprintf(stderr, "Client::Client - Connection to server at \"%s\" failed.\n", server);
		close(sfd);
		sfd = -1;
		return;
	}

	//We are now ready to play!
}
//EOF


