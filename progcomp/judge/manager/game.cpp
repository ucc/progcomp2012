#include "game.h"

using namespace std;



Game* Game::theGame = NULL;
bool Game::gameCreated = false;

Game::Game(const char * redPath, const char * bluePath, const bool enableGraphics, double newStallTime, const bool allowIllegal, FILE * newLog, const  Piece::Colour & newReveal, int newMaxTurns, bool newPrintBoard) : red(NULL), blue(NULL), turn(Piece::RED), theBoard(10,10), graphicsEnabled(enableGraphics), stallTime(newStallTime), allowIllegalMoves(allowIllegal), log(newLog), reveal(newReveal), turnCount(0), input(NULL), maxTurns(newMaxTurns), printBoard(newPrintBoard)
{
	gameCreated = false;
	if (gameCreated)
	{
		fprintf(stderr, "Game::Game - Error - Tried to create more than one Game!\n");
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
		blue = new AI_Controller(Piece::BLUE, bluePath);


}

Game::Game(const char * fromFile, const bool enableGraphics, double newStallTime, const bool allowIllegal, FILE * newLog, const  Piece::Colour & newReveal, int newMaxTurns, bool newPrintBoard) : red(NULL), blue(NULL), turn(Piece::RED), theBoard(10,10), graphicsEnabled(enableGraphics), stallTime(newStallTime), allowIllegalMoves(allowIllegal), log(newLog), reveal(newReveal), turnCount(0), input(NULL), maxTurns(newMaxTurns), printBoard(newPrintBoard)
{
	gameCreated = false;
	if (gameCreated)
	{
		fprintf(stderr, "Game::Game - Error - Tried to create more than one Game!\n");
		exit(EXIT_FAILURE);
	}
	gameCreated = true;
	Game::theGame = this;
	signal(SIGPIPE, Game::HandleBrokenPipe);


	if (graphicsEnabled && (!Graphics::Initialised()))
			Graphics::Initialise("Stratego", theBoard.Width()*32, theBoard.Height()*32);

	input = fopen(fromFile, "r");

	red = new FileController(Piece::RED, input);
	blue = new FileController(Piece::BLUE, input);


}

Game::~Game()
{
	
	delete red;
	delete blue;

	if (log != NULL && log != stdout && log != stderr)
		fclose(log);

	if (input != NULL && input != stdin)
		fclose(input);
}

/**
 * Attempts to setup the board and controllers
 * @param redName the name of the red AI
 * @param blueName the name of the blue AI
 * @returns A colour, indicating if there were any errors
	Piece::NONE indicates no errors
	Piece::BOTH indicates errors with both AI
	Piece::RED / Piece::BLUE indicates an error with only one of the two AI
 */
Piece::Colour Game::Setup(const char * redName, const char * blueName)
{

	if (!red->Valid())
	{
		logMessage("Controller for Player RED is invalid!\n");
		if (!red->HumanController())
			logMessage("Check that program \"%s\" exists and has executable permissions set.\n", redName);
	}
	if (!blue->Valid())
	{
		logMessage("Controller for Player BLUE is invalid!\n");
		if (!blue->HumanController())
			logMessage("Check that program \"%s\" exists and has executable permissions set.\n", blueName);
	}
	if (!red->Valid())
	{
		if (!blue->Valid())
			return Piece::BOTH;
		return Piece::RED;
	}
	else if (!blue->Valid())
	{
		return Piece::BLUE;
	}

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


	Piece::Colour result = Piece::NONE;
	if (redSetup != MovementResult::OK)
	{	
		if (blueSetup != MovementResult::OK)
		{
			logMessage("BOTH players give invalid setup!\n");
			result = Piece::BOTH;
		}
		else
		{
			//logMessage("Player RED gave an invalid setup!\n");
			result = Piece::RED;
		}
		
	}
	else if (blueSetup != MovementResult::OK)
	{
		//logMessage("Player BLUE gave an invalid setup!\n");
		result = Piece::BLUE;
	}


	logMessage("%s RED SETUP\n", red->name.c_str());
	if (redSetup == MovementResult::OK)
	{
		for (int y=0; y < 4; ++y)
		{
			for (int x=0; x < theBoard.Width(); ++x)
			{
				if (theBoard.GetPiece(x, y) != NULL)
					logMessage("%c", Piece::tokens[(int)(theBoard.GetPiece(x, y)->type)]);
				else
					logMessage(".");
			}
			logMessage("\n");
		}	
	}
	else
	{
		logMessage("INVALID!\n");
	}

	logMessage("%s BLUE SETUP\n", blue->name.c_str());
	if (blueSetup == MovementResult::OK)
	{
		for (int y=0; y < 4; ++y)
		{
			for (int x=0; x < theBoard.Width(); ++x)
			{
				if (theBoard.GetPiece(x, theBoard.Height()-4+y) != NULL)
					logMessage("%c", Piece::tokens[(int)(theBoard.GetPiece(x, theBoard.Height()-4+y)->type)]);
				else
					logMessage(".");
			}
			logMessage("\n");
		}	
	}
	else
	{
		logMessage("INVALID!\n");
	}

	
	return result;

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
	if (theGame == NULL)
	{
		fprintf(stderr, "ERROR - Recieved SIGPIPE during game exit!\n");
		exit(EXIT_FAILURE);
	}
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

	if (Game::theGame->printBoard)
		Game::theGame->theBoard.PrintPretty(stdout, Piece::BOTH);

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
	if (turnCount == 0)
	{
		logMessage("Game ends in the SETUP phase - REASON: ");
	}
	else
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
			logMessage("Attempted move into square occupied by neutral or allied piece\n");
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
		case MovementResult::DRAW_DEFAULT:
			logMessage("Game declared a draw after %d turns\n", turnCount);
			break;
		case MovementResult::DRAW:
			logMessage("Game declared a draw because neither player has mobile pieces\n");
			break;
		case MovementResult::SURRENDER:
			logMessage("This player has surrendered!\n");
			break;
		case MovementResult::BAD_SETUP:
			switch (turn)
			{
				case Piece::RED:
					logMessage("An illegal setup was made by RED\n");
					break;
				case Piece::BLUE:
					logMessage("An illegal setup was made by BLUE\n");
					break;
				case Piece::BOTH:
					logMessage("An illegal setup was made by BOTH players\n");
					break;
				case Piece::NONE:
					logMessage("Unknown internal error.\n");
					break;
			}
			break;

	}

	if (printBoard)
	{
		system("clear");
		fprintf(stdout, "%d Final State\n", turnCount);
		theBoard.PrintPretty(stdout, Piece::BOTH);
		fprintf(stdout, "\n");
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

	Piece::Colour toReveal = reveal;
	
	
	


	red->Message("START");
	


	while (!Board::HaltResult(result) && (turnCount < maxTurns || maxTurns < 0))
	{
		if (red->HumanController())
			toReveal = Piece::RED;
		if (printBoard)
		{
			system("clear");
			if (turnCount == 0)
				fprintf(stdout, "START:\n");
			else
				fprintf(stdout, "%d BLUE:\n", turnCount);
			theBoard.PrintPretty(stdout, toReveal);
			fprintf(stdout, "\n\n");
		}

		if (graphicsEnabled)
			theBoard.Draw(toReveal);
		
		turn = Piece::RED;
		logMessage( "%d RED: ", turnCount);
		result = red->MakeMove(buffer);
		red->Message(buffer);
		blue->Message(buffer);
		logMessage( "%s\n", buffer.c_str());
		if (Board::HaltResult(result))
			break;

		if (stallTime > 0)
			Wait(stallTime);
		else
			ReadUserCommand();

		if (blue->HumanController())
			toReveal = Piece::BLUE;
		if (printBoard)
		{
			system("clear");
			fprintf(stdout, "%d RED:\n", turnCount);
			theBoard.PrintPretty(stdout, toReveal);
			fprintf(stdout, "\n\n");
		}
		if (graphicsEnabled)
			theBoard.Draw(toReveal);

		
		
		turn = Piece::BLUE;
		logMessage( "%d BLU: ", turnCount);
		result = blue->MakeMove(buffer);
		blue->Message(buffer);
		red->Message(buffer);
		logMessage( "%s\n", buffer.c_str());

		if (Board::HaltResult(result))
			break;

		

		

		if (stallTime > 0)
			Wait(stallTime);
		else
			ReadUserCommand();
	
		if (theBoard.MobilePieces(Piece::BOTH) == 0)
			result = MovementResult::DRAW;

		++turnCount;
	}

	if ((maxTurns >= 0 && turnCount >= maxTurns) && result == MovementResult::OK)
	{
		result = MovementResult::DRAW_DEFAULT;
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

/**
 * Waits for a user command
 * Currently ignores the command.
 */
void Game::ReadUserCommand()
{
	fprintf(stdout, "Waiting for user to press enter...\n");
	string command("");
	for (char c = fgetc(stdin); c != '\n' && (int)(c) != EOF; c = fgetc(stdin))
	{
		command += c;
	}
}

MovementResult FileController::QuerySetup(const char * opponentName, std::string setup[])
{

	char c = fgetc(file);
	name = "";
	while (c != ' ')
	{
		name += c;
		c = fgetc(file);
	}

	while (fgetc(file) != '\n');

	for (int y = 0; y < 4; ++y)
	{
		setup[y] = "";
		for (int x = 0; x < Game::theGame->theBoard.Width(); ++x)
		{
			setup[y] += fgetc(file);
		}

		if (fgetc(file) != '\n')
		{
			return MovementResult::BAD_RESPONSE;
		}
	}
	return MovementResult::OK;

	
}

MovementResult FileController::QueryMove(std::string & buffer)
{
	char buf[BUFSIZ];

	fgets(buf, sizeof(buf), file);
	char * s = (char*)(buf);
	while (*s != ':' && *s != '\0')
		++s;

	s += 2;
	
	buffer = string(s);
	return MovementResult::OK;
}


