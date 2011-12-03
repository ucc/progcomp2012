#include "game.h"

using namespace std;



Game* Game::theGame = NULL;

Game::Game(const char * redPath, const char * bluePath, const bool enableGraphics, double newStallTime, const bool allowIllegal, FILE * newLog, const  Piece::Colour & newReveal) : red(NULL), blue(NULL), turn(Piece::RED), theBoard(10,10), graphicsEnabled(enableGraphics), stallTime(newStallTime), allowIllegalMoves(allowIllegal), log(newLog), reveal(newReveal), turnCount(0)
{
	static bool gameCreated = false;
	if (gameCreated)
	{
		if (log != NULL)
			fprintf(log, "ERROR - Game has already been created!\n");
		exit(EXIT_FAILURE);
	}
	gameCreated = true;
	Game::theGame = this;
	signal(SIGPIPE, Game::HandleBrokenPipe);


	if (graphicsEnabled && (!Graphics::Initialised()))
			Graphics::Initialise("Stratego", theBoard.Width()*32, theBoard.Height()*32);

	if (strcmp(redPath, "human") == 0)
		red = new Human_Controller(Piece::RED, graphicsEnabled);
	else
		red = new AI_Controller(Piece::RED, redPath);
	
	
	if (strcmp(bluePath, "human") == 0)
		blue = new Human_Controller(Piece::BLUE, graphicsEnabled);
	else
		blue = new AI_Controller(Piece::BLUE, redPath);


}

Game::~Game()
{
	fprintf(stderr, "Killing AI\n");
	delete red;
	delete blue;

	if (log != NULL && log != stdout && log != stderr)
		fclose(log);
}

bool Game::Setup(const char * redName, const char * blueName)
{

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


	MovementResult redSetup = red->Setup(blueName);
	MovementResult blueSetup = blue->Setup(redName);

	if (redSetup != MovementResult::OK)
	{	
		if (blueSetup != MovementResult::OK)
		{
			if (log != NULL)
				fprintf(log, "BOTH players give invalid setup!\n");
			red->Message("ILLEGAL");
			blue->Message("ILLEGAL");
		}
		else
		{
			if (log != NULL)
				fprintf(log, "Player RED gave an invalid setup!\n");
			red->Message("ILLEGAL");
			blue->Message("DEFAULT");
		}
		return false;
	}
	else if (blueSetup != MovementResult::OK)
	{
		if (log != NULL)
			fprintf(log, "Player BLUE gave an invalid setup!\n");
		red->Message("DEFAULT");
		blue->Message("ILLEGAL");	
		return false;
	}
	return true;

}

void Game::Wait(double wait)
{
	if (wait <= 0)
		return;

	TimerThread timer(wait*1000000); //Wait in seconds
	timer.Start();

	if (!graphicsEnabled)
	{
		while (!timer.Finished());
		timer.Stop();
		return;
	}


	while (!timer.Finished())
	{
		SDL_Event  event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					timer.Stop();
					exit(EXIT_SUCCESS);
					break;
			}
		}
	}
	timer.Stop();
	
}

void Game::HandleBrokenPipe(int sig)
{
	if (theGame->turn == Piece::RED)
	{
		theGame->logMessage("Game ends on RED's turn - REASON: ");
		theGame->blue->Message("DEFAULT");	
	}
	else if (theGame->turn == Piece::BLUE)
	{
	
		theGame->logMessage("Game ends on BLUE's turn - REASON: ");
		theGame->red->Message("DEFAULT");
	}
	else
	{
		theGame->logMessage("Game ends on ERROR's turn - REASON: ");
			
	}
	
	theGame->logMessage("SIGPIPE - Broken pipe (AI program may have segfaulted)\n");



	if (Game::theGame->graphicsEnabled && theGame->log == stdout)
	{
		theGame->logMessage("CLOSE WINDOW TO EXIT\n");
		Game::theGame->theBoard.Draw(Piece::BOTH);
		while (true)
		{
			SDL_Event  event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_QUIT:
						exit(EXIT_SUCCESS);
						break;
				}
			}			
		}
	}
	else
	{
		if (theGame->log == stdout)
		{
			theGame->logMessage( "PRESS ENTER TO EXIT\n");
			theGame->theBoard.Print(theGame->log);
			while (fgetc(stdin) != '\n');
		}
	}
	

	exit(EXIT_SUCCESS);
}

void Game::PrintEndMessage(const MovementResult & result)
{
	if (turn == Piece::RED)
	{
		logMessage("Game ends on RED's turn - REASON: ");	
	}
	else if (turn == Piece::BLUE)
	{
		logMessage("Game ends on BLUE's turn - REASON: ");
	}
	else
	{
		logMessage("Game ends on ERROR's turn - REASON: ");
			
	}
	switch (result.type)
	{
		case MovementResult::OK:
			logMessage("Status returned OK, unsure why game halted...\n");
			break;
		case MovementResult::DIES:
			logMessage("Status returned DIES, unsure why game halted...\n");
			break;
		case MovementResult::KILLS:
			logMessage("Status returned KILLS, unsure why game halted...\n");
			break;
		case MovementResult::BOTH_DIE:
			logMessage("Status returned BOTH_DIE, unsure why game halted...\n");
			break;
		case MovementResult::NO_BOARD:
			logMessage("Board does not exit?!\n");
			break;
		case MovementResult::INVALID_POSITION:
			logMessage("Coords outside board\n");
			break;
		case MovementResult::NO_SELECTION:
			logMessage("Move does not select a piece\n");
			break;
		case MovementResult::NOT_YOUR_UNIT:
			logMessage("Selected piece belongs to other player\n");
			break;
		case MovementResult::IMMOBILE_UNIT:
			logMessage("Selected piece is not mobile (FLAG or BOMB)\n");
			break;
		case MovementResult::INVALID_DIRECTION:
			logMessage("Selected unit cannot move that way\n");
			break;
		case MovementResult::POSITION_FULL:
			logMessage("Attempted move into square occupied by allied piece\n");
			break;
		case MovementResult::VICTORY:
			logMessage("Captured the flag\n");
			break;
		case MovementResult::BAD_RESPONSE:
			logMessage("Unintelligable response\n");
			break;
		case MovementResult::NO_MOVE:
			logMessage("Did not make a move (may have exited)\n");
			break;
		case MovementResult::COLOUR_ERROR:
			logMessage("Internal controller error - COLOUR_ERROR\n");
			break;
		case MovementResult::ERROR:
			logMessage("Internal controller error - Unspecified ERROR\n");
			break;

	}

	if (graphicsEnabled && log == stdout)
	{
		logMessage("CLOSE WINDOW TO EXIT\n");
		theBoard.Draw(Piece::BOTH);
		while (true)
		{
			SDL_Event  event;
			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
					case SDL_QUIT:
						exit(EXIT_SUCCESS);
						break;
				}
			}			
		}
	}
	else
	{
		if (log == stdout)
		{
			logMessage("PRESS ENTER TO EXIT\n");
			while (fgetc(stdin) != '\n');
		}
	}

}



MovementResult Game::Play()
{
	
	MovementResult result = MovementResult::OK;
	turnCount = 1;
	string buffer;
	


	red->Message("START");
	logMessage("START");
	while (Board::LegalResult(result))
	{

		
		turn = Piece::RED;
		logMessage( "%d RED: ", turnCount);
		result = red->MakeMove(buffer);
		red->Message(buffer);
		blue->Message(buffer);
		logMessage( "%s\n", buffer.c_str());
		if (!Board::LegalResult(result))
			break;
		if (graphicsEnabled)
			theBoard.Draw(reveal);
		Wait(stallTime);
		
		turn = Piece::BLUE;
		logMessage( "%d BLU: ", turnCount);
		result = blue->MakeMove(buffer);
		blue->Message(buffer);
		red->Message(buffer);
		logMessage( "%s\n", buffer.c_str());

		if (!Board::LegalResult(result))
			break;

		

		if (graphicsEnabled)
			theBoard.Draw(reveal);
		Wait(stallTime);
		
		++turnCount;
	}

	
	return result;

		

}

/**
 * Logs a message to the game's log file if it exists
 * @param format the format string
 * @param additional parameters - printed using va_args
 * @returns the result of vfprintf or a negative number if the log file does not exist
 */
int Game::logMessage(const char * format, ...)
{
	if (log == NULL)
		return -666;
		va_list ap;
	va_start(ap, format);

	int result = vfprintf(log, format, ap);
	va_end(ap);

	return result;
}
