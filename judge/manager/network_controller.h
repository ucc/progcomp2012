#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>



#include "controller.h"

#ifndef NETWORK_CONTROLLER_H
#define NETWORK_CONTROLLER_H


class NetworkController : public Controller
{
	public:
		NetworkController(const Piece::Colour & newColour, const char * newName = "no-name");
		virtual ~NetworkController();

		virtual bool Valid() {return sfd != -1;}

		virtual void Message(const char * string) {fprintf(stderr, "NetworkController unimplemented!\n"); assert(false);}
		virtual MovementResult QuerySetup(const char * opponentName, std::string setup[]) {fprintf(stderr, "NetworkController unimplemented!\n"); assert(false);}
		virtual MovementResult QueryMove(std::string & buffer) {fprintf(stderr, "NetworkController unimplemented!\n"); assert(false);}

	protected:
		int sfd; int cfd;
		static const int port = 666;	
};

class Server : public NetworkController
{
	public:
		Server(const Piece::Colour & newColour, const char * newName = "no-name");
		virtual ~Server() {}
};

class Client : public NetworkController
{
	public:
		Client(const Piece::Colour & newColour, const char * newName = "no-name", const char * server="127.0.0.1");
		virtual ~Client() {}
};

#endif //NETWORK_CONTROLLER_H

//EOF
