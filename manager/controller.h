#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "stratego.h"
#include "program.h"

/**
 * Class to control an AI program for a game of Stratego
 * Inherits most features from the Program class
 */

class Controller : public Program
{
	public:
		Controller(const Piece::Colour & newColour, const char * executablePath) : Program(executablePath), colour(newColour) {}
		virtual ~Controller() {}

		MovementResult Setup(const char * opponentName); //Requests the AI program for the initial positioning of its pieces.

		MovementResult MakeMove(std::string & buffer); //Queries the AI program for a response to the state of Board::theBoard

		const Piece::Colour colour; //Colour identifying the side of the AI program.


};

#endif //CONTROLLER_H


