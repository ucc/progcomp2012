#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "stratego.h"
#include <string>

/**
 * Class to control a player for Stratego
 * Abstract base class
 */

class Controller
{
	public:
		Controller(const Piece::Colour & newColour) : colour(newColour) {}
		virtual ~Controller() {}

		MovementResult Setup(const char * opponentName);

		MovementResult MakeMove(std::string & buffer);



		void Message(std::string & buffer) {Message(buffer.c_str());}
		virtual void Message(const char * string) = 0;

		virtual MovementResult QuerySetup(const char * opponentName, std::string setup[]) = 0;
		virtual MovementResult QueryMove(std::string & buffer) = 0;

		const Piece::Colour colour; 


};





#endif //CONTROLLER_H


