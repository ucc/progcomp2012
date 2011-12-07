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
		Controller(const Piece::Colour & newColour, const char * newName = "no-name") : colour(newColour), name(newName) {}
		virtual ~Controller() {}

		MovementResult Setup(const char * opponentName);

		MovementResult MakeMove(std::string & buffer);

		virtual bool HumanController() const {return false;} //Hacky... overrides in human_controller... avoids having to use run time type info

		void Message(const std::string & buffer) {Message(buffer.c_str());}
		virtual void Message(const char * string) = 0;

		virtual MovementResult QuerySetup(const char * opponentName, std::string setup[]) = 0;
		virtual MovementResult QueryMove(std::string & buffer) = 0;
		virtual bool Valid() const {return true;}

		const Piece::Colour colour; 

		std::string name;


};





#endif //CONTROLLER_H


