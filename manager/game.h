#ifndef MAIN_H
#define MAIN_H

#include "stratego.h"
#include "ai_controller.h"
#include "human_controller.h"



/**
 * Class to manage the game
 */
class Game
{
	public:
		Game(const char * redPath, const char * bluePath, const bool enableGraphics, double newStallTime = 1.0, const bool allowIllegal=false, FILE * newLog = NULL, const Piece::Colour & newRevealed = Piece::BOTH);
		virtual ~Game();

		

		void Wait(double wait); 

		bool Setup(const char * redName, const char * blueName);
		MovementResult Play();
		void PrintEndMessage(const MovementResult & result);
		

		static void HandleBrokenPipe(int signal);
		
		
		const Piece::Colour Turn() const {return turn;}
		int TurnCount() const {return turnCount;}

		static Game * theGame;
	private:
		int logMessage(const char * format, ...);
		Controller * red;
		Controller * blue;
		Piece::Colour turn;
		
	public:
		Board theBoard;
	private:
		const bool graphicsEnabled;
		double stallTime;
	public:
		const bool allowIllegalMoves;

	private:
		FILE * log;
		Piece::Colour reveal;
		int turnCount;
		
};


#endif //MAIN_H
