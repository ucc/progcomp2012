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
Colour turn;

void cleanup();

void BrokenPipe(int sig);

int main(int argc, char ** argv)
{
	assert(argc == 3);
	
	
	for (int y = 4; y < 6; ++y)
	{
		for (int x = 2; x < 4; ++x)
		{
			theBoard.AddPiece(x,y,Piece::BOULDER, Piece::NONE);
		}
		for (int x = 6; x < 8; ++x)
		{
			theBoard.AddPiece(x,y,Piece::BOULDER, Piece::NONE);
		}
	}
	

	red = new Controller(Piece::RED, argv[1]);
	blue = new Controller(Piece::BLUE, argv[2]);
	atexit(cleanup);
	signal(SIGPIPE, BrokenPipe);

	MovementResult redSetup = red->Setup(argv[2]);
	MovementResult blueSetup = blue->Setup(argv[1]);
	if (redSetup != MovementResult::OK)
	{
		fprintf(stderr, "Blue wins by DEFAULT!\n");
		red->SendMessage("ILLEGAL");
		blue->SendMessage("DEFAULT");
		exit(EXIT_SUCCESS);
	}
	if (blueSetup != MovementResult::OK)
	{
		fprintf(stderr, "Red wins by DEFAULT!\n");
		red->SendMessage("DEFAULT");
		blue->SendMessage("ILLEGAL");
		exit(EXIT_SUCCESS);
	}

	MovementResult result(MovementResult::OK);
	system("clear");
	int count = 1;

	#ifdef GRAPHICS
		if (!Graphics::Initialised())
			Graphics::Initialise("Stratego", theBoard.Width()*32, theBoard.Height()*32);
		
	#endif //GRAPHICS

	string buffer;

	red->SendMessage("START");
	turn = Piece::RED;
	while (Board::LegalResult(result))
	{

		
		turn = Piece::RED;
		fprintf(stderr, "%d RED: ", count);
		result = red->MakeMove(buffer);
		red->SendMessage(buffer);
		blue->SendMessage(buffer);
		fprintf(stderr, "%s\n", buffer.c_str());
		if (!Board::LegalResult(result))
			break;
		#ifdef GRAPHICS
			Board::theBoard.Draw();
			if (CheckForQuitWhilstWaiting(0.5))
			{
				red->SendMessage("QUIT");
				blue->SendMessage("QUIT");
				exit(EXIT_SUCCESS);
			}
		#endif //GRAPHICS
		
		turn = Piece::BLUE;
		fprintf(stderr, "%d BLU: ", count);
		result = blue->MakeMove(buffer);
		blue->SendMessage(buffer);
		red->SendMessage(buffer);
		fprintf(stderr, "%s\n", buffer.c_str());

		if (!Board::LegalResult(result))
			break;

		

		#ifdef GRAPHICS
			Board::theBoard.Draw();
			if (CheckForQuitWhilstWaiting(0.5))
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
	switch (result.type)
	{
		case MovementResult::NO_BOARD:
			fprintf(stderr,"Board does not exit?!\n");
			break;
		case MovementResult::INVALID_POSITION:
			fprintf(stderr,"Coords outside board\n");
			break;
		case MovementResult::NO_SELECTION:
			fprintf(stderr,"Move does not select a piece\n");
			break;
		case MovementResult::NOT_YOUR_UNIT:
			fprintf(stderr,"Selected piece belongs to other player\n");
			break;
		case MovementResult::IMMOBILE_UNIT:
			fprintf(stderr,"Selected piece is not mobile (FLAG or BOMB)\n");
			break;
		case MovementResult::INVALID_DIRECTION:
			fprintf(stderr,"Selected unit cannot move that way\n");
			break;
		case MovementResult::POSITION_FULL:
			fprintf(stderr,"Attempted move into square occupied by allied piece\n");
			break;
		case MovementResult::VICTORY:
			fprintf(stderr,"Captured the flag\n");
			break;
		case MovementResult::BAD_RESPONSE:
			fprintf(stderr,"Unintelligable response\n");
			break;
		case MovementResult::NO_MOVE:
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

void BrokenPipe(int sig)
{
	if (turn == Piece::RED)
	{
		fprintf(stderr,"Game ends on RED's turn - REASON: Broken pipe\n");
		blue->SendMessage("DEFAULT");	
	}
	else if (turn == Piece::BLUE)
	{
		fprintf(stderr,"Game ends on BLUE's turn - REASON: Broken pipe\n");
		red->SendMessage("DEFAULT");
	}
	else
	{
		fprintf(stderr,"Game ends on ERROR's turn - REASON: Broken pipe\n");
			
	}
			Board::theBoard.Draw();
			while (true)
			{
				if (CheckForQuitWhilstWaiting(4000))
				{
				red->SendMessage("QUIT");
				blue->SendMessage("QUIT");
				exit(EXIT_SUCCESS);
				}
			}
	exit(EXIT_SUCCESS);
}
	
#endif //GRAPHICS
