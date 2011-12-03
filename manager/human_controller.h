#ifndef HUMAN_CONTROLLER_H
#define HUMAN_CONTROLLER_H

#include "controller.h"

/**
 * Class to control a human player playing Stratego
 */
class Human_Controller : public Controller
{
	public:
		Human_Controller(const Piece::Colour & newColour, const bool enableGraphics) : Controller(newColour, "human"), graphicsEnabled(enableGraphics) {}
		virtual ~Human_Controller() {}

		virtual MovementResult QuerySetup(const char * opponentName, std::string setup[]);
		virtual MovementResult QueryMove(std::string & buffer); 
		virtual void Message(const char * message) {fprintf(stderr, "Recieved message \"%s\" from manager.\n", message);}
	
	private:
		const bool graphicsEnabled;


};

#endif //AI_CONTROLLER_H
