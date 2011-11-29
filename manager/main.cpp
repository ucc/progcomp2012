#include <stdlib.h>
#include <stdio.h>

#include "common.h"

#include "controller.h"
#include "stratego.h"

using namespace std;



#define theBoard Board::theBoard

#ifdef GRAPHICS
	bool CheckForQuitWhilstWaiting(int wait);
#endif //GRAPHICS

Controller * red;
Controller * blue;

void cleanup();
int main(int argc, char ** argv)
{
	assert(argc == 3);
	
	for (int y = 5; y < 9; ++y)
	{
		for (int x = 3; x < 5; ++x)
		{
			theBoard.AddPiece(x,y,Piece::BOULDER, Piece::NONE);
		}
		for (int x = 9; x < 11; ++x)
		{
			theBoard.AddPiece(x,y,Piece::BOULDER, Piece::NONE);
		}
	}

	red = new Controller(Piece::RED, argv[1]);
	blue = new Controller(Piece::BLUE, argv[2]);
	atexit(cleanup);

	Board::MovementResult redSetup = red->Setup(argv[2]);
	Board::MovementResult blueSetup = blue->Setup(argv[1]);
	if (redSetup != Board::OK)
	{
		fprintf(stderr, "Blue wins by DEFAULT!\n");
		red->SendMessage("ILLEGAL");
		blue->SendMessage("DEFAULT");
		exit(EXIT_SUCCESS);
	}
	if (blueSetup != Board::OK)
	{
		fprintf(stderr, "Red wins by DEFAULT!\n");
		red->SendMessage("DEFAULT");
		blue->SendMessage("ILLEGAL");
		exit(EXIT_SUCCESS);
	}

	Board::MovementResult result = Board::OK;
	system("clear");
	int count = 1;

	#ifdef GRAPHICS
		if (!Graphics::Initialised())
			Graphics::Initialise("Stratego", theBoard.Width()*32, theBoard.Height()*32);
		
	#endif //GRAPHICS

	string buffer;

	red->SendMessage("START");
	Colour turn = Piece::RED;
	while (Board::LegalResult(result))
	{

		fprintf(stderr, "This is move %d...\n", count);
		fprintf(stderr,"---RED's turn---\n");
		turn = Piece::RED;
		result = red->MakeMove(buffer);
		red->SendMessage(buffer);
		blue->SendMessage(buffer);

		if (!Board::LegalResult(result))
			break;
		#ifdef GRAPHICS
			Board::theBoard.Draw();
			if (CheckForQuitWhilstWaiting(0.2))
			{
				red->SendMessage("QUIT");
				blue->SendMessage("QUIT");
				exit(EXIT_SUCCESS);
			}
		#endif //GRAPHICS
		fprintf(stderr,"---BLUE's turn---\n");
		turn = Piece::BLUE;
		result = blue->MakeMove(buffer);
		blue->SendMessage(buffer);
		red->SendMessage(buffer);

		if (!Board::LegalResult(result))
			break;

		

		#ifdef GRAPHICS
			Board::theBoard.Draw();
			if (CheckForQuitWhilstWaiting(0.2))
			{
				red->SendMessage("QUIT");
				blue->SendMessage("QUIT");
				exit(EXIT_SUCCESS);
			}
		#else
			Board::theBoard.Print(stderr);
			sleep(1);
			system("clear");
		#endif //GRAPHICS
		
		++count;
	}

	printf("Final board state\n");
	#ifdef GRAPHICS
			Board::theBoard.Draw();
			if (CheckForQuitWhilstWaiting(4))
			{
				red->SendMessage("QUIT");
				blue->SendMessage("QUIT");
				exit(EXIT_SUCCESS);
			}
	#else
		Board::theBoard.Print(stderr);
	#endif //GRAPHICS
	sleep(2);


	if (turn == Piece::RED)
	{
		fprintf(stderr,"Game ends on RED's turn - REASON: ");	
	}
	else if (turn == Piece::BLUE)
	{
		fprintf(stderr,"Game ends on BLUE's turn - REASON: ");
	}
	else
	{
		fprintf(stderr,"Game ends on ERROR's turn - REASON: ");
			
	}
	switch (result)
	{
		case Board::NO_BOARD:
			fprintf(stderr,"Board does not exit?!\n");
			break;
		case Board::INVALID_POSITION:
			fprintf(stderr,"Coords outside board\n");
			break;
		case Board::NO_SELECTION:
			fprintf(stderr,"Move does not select a piece\n");
			break;
		case Board::NOT_YOUR_UNIT:
			fprintf(stderr,"Selected piece belongs to other player\n");
			break;
		case Board::IMMOBILE_UNIT:
			fprintf(stderr,"Selected piece is not mobile (FLAG or BOMB)\n");
			break;
		case Board::INVALID_DIRECTION:
			fprintf(stderr,"Selected unit cannot move that way\n");
			break;
		case Board::POSITION_FULL:
			fprintf(stderr,"Attempted move into square occupied by allied piece\n");
			break;
		case Board::VICTORY:
			fprintf(stderr,"Captured the flag\n");
			break;
		case Board::BAD_RESPONSE:
			fprintf(stderr,"Unintelligable response\n");
			break;
		case Board::NO_MOVE:
			fprintf(stderr,"Did not make a move (may have exited)\n");
			break;
	}



	exit(EXIT_SUCCESS);
	
	return 0;
}

#ifdef GRAPHICS

bool CheckForQuitWhilstWaiting(int wait)
{


	TimerThread timer(wait*1000000); //Wait in seconds
	timer.Start();
	while (!timer.Finished())
	{
		SDL_Event  event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					timer.Stop();
					return true;
					break;
			}
		}
	}
	timer.Stop();
	return false;
}

void cleanup()
{
	delete red;
	delete blue;
}

#endif //GRAPHICS
