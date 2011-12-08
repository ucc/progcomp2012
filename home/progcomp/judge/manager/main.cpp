#include <stdlib.h>
#include <stdio.h>






#include "game.h"

using namespace std;

Piece::Colour SetupGame(int argc, char ** argv);
void DestroyGame();
void PrintResults(const MovementResult & result, string & buffer);

int main(int argc, char ** argv)
{
	


	if (argc == 1)
	{
		fprintf(stderr, "Usage: stratego [options] red blue\n");
		fprintf(stderr, "       stratego --help\n");
		exit(EXIT_SUCCESS);
		
	}
	

	Piece::Colour setupError = SetupGame(argc, argv);
	MovementResult result = MovementResult::OK;
	if (setupError == Piece::NONE)
	{
		result = Game::theGame->Play();
	}
	else
	{
		result = MovementResult::BAD_SETUP;
		Game::theGame->ForceTurn(setupError);
	}
	
	Game::theGame->PrintEndMessage(result);

	string buffer = "";
	PrintResults(result, buffer);

	//Message the AI's the quit message
	Game::theGame->red->Message("QUIT " + buffer);
	Game::theGame->blue->Message("QUIT " + buffer);

	//Log the message
	if (Game::theGame->GetLogFile() != stdout)
		Game::theGame->logMessage("%s\n", buffer.c_str());

	fprintf(stdout, "%s\n", buffer.c_str());

	exit(EXIT_SUCCESS);
	return 0;
}

Piece::Colour SetupGame(int argc, char ** argv)
{
	char * red = NULL; char * blue = NULL; double timeout = 0.00001; bool graphics = false; bool allowIllegal = false; FILE * log = NULL;
	Piece::Colour reveal = Piece::BOTH; char * inputFile = NULL; int maxTurns = 5000; bool printBoard = false;
	for (int ii=1; ii < argc; ++ii)
	{
		if (argv[ii][0] == '-')
		{
			switch (argv[ii][1])
			{
				case 't':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected timeout value after -t switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "inf") == 0)
						timeout = -1;
					else
						timeout = atof(argv[ii+1]);
					++ii;
					break;
				case 'g':
					graphics = !graphics;
					break;
				case 'p':
					printBoard = !printBoard;
					break;
				case 'i':
					allowIllegal = true;
					break;

				case 'o':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected filename or \"stdout\" after -o switch!\n");
						exit(EXIT_FAILURE);
					}
					if (log != NULL)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected at most ONE -o switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "stdout") == 0)
						log = stdout;
					else
						log = fopen(argv[ii+1], "w");
					setbuf(log, NULL);
				
					++ii;
					break;	

				case 'r':
					if (reveal == Piece::BOTH)
						reveal = Piece::BLUE;
					else
						reveal = Piece::NONE;
					break;			
				case 'b':
					if (reveal == Piece::BOTH)
						reveal = Piece::RED;
					else
						reveal = Piece::NONE;
					break;
				case 'm':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected max_turns value after -m switch!\n");
						exit(EXIT_FAILURE);
					}
					if (strcmp(argv[ii+1], "inf") == 0)
						maxTurns = -1;
					else
						maxTurns = atoi(argv[ii+1]);
					++ii;
					break;
				case 'f':
					if (argc - ii <= 1)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected filename after -f switch!\n");
						exit(EXIT_FAILURE);
					}
					if (log != NULL)
					{
						fprintf(stderr, "ARGUMENT_ERROR - Expected at most ONE -f switch!\n");
						exit(EXIT_FAILURE);
					}
					red = (char*)("file");
					blue = (char*)("file");
					inputFile = argv[ii+1];
					++ii;
					break;
				case 'h':
					system("clear");	
					system("less manual.txt");
					exit(EXIT_SUCCESS);
					break;
				case '-':
					if (strcmp(argv[ii]+2, "help") == 0)
					{
						system("clear");	
						system("less manual.txt");
						exit(EXIT_SUCCESS);
					}
					else
					{
						fprintf(stderr, "ARGUMENT_ERROR - Unrecognised switch \"%s\"...\n", argv[ii]);
						exit(EXIT_FAILURE);
					}
			}
			
		}
		else
		{
			if (red == NULL)
				red = argv[ii];
			else if (blue == NULL)
				blue = argv[ii];
			else
			{
				fprintf(stderr, "ARGUMENT_ERROR - Unexpected argument \"%s\"...\n", argv[ii]);
				exit(EXIT_FAILURE);
			}
		}
	}



	if (inputFile == NULL)
	{
		if (red == NULL || blue == NULL) //Not enough arguments
		{
			fprintf(stderr, "ARGUMENT_ERROR - Did not recieve enough players (did you mean to use the -f switch?)\n");	
			exit(EXIT_FAILURE);	
		}
		Game::theGame = new Game(red,blue, graphics, timeout, allowIllegal,log, reveal,maxTurns, printBoard);
	}
	else
	{
		Game::theGame = new Game(inputFile, graphics, timeout, allowIllegal,log, reveal,maxTurns, printBoard);
	}

	if (Game::theGame == NULL)
	{
		fprintf(stderr,"INTERNAL_ERROR - Error creating Game!\n");
		exit(EXIT_FAILURE);
	}
	atexit(DestroyGame);
	
	return Game::theGame->Setup(red, blue);
	

}

void PrintResults(const MovementResult & result, string & buffer)
{
	stringstream s("");
	switch (Game::theGame->Turn())
	{
		case Piece::RED:
			s << Game::theGame->red->name << " RED ";
			break;
		case Piece::BLUE:
			s << Game::theGame->blue->name << " BLUE ";
			break;
		case Piece::BOTH:
			s << "neither BOTH ";
			break;
		case Piece::NONE:
			s << "neither NONE ";
			break;
	}

	if (!Board::LegalResult(result) && result != MovementResult::BAD_SETUP)
		s << "ILLEGAL ";
	else if (!Board::HaltResult(result))
		s << "INTERNAL_ERROR ";
	else
	{
		switch (result.type)
		{
			case MovementResult::VICTORY:
				s <<  "VICTORY ";
				break;
			case MovementResult::SURRENDER:
				s << "SURRENDER ";
				break;
			case MovementResult::DRAW:
				s << "DRAW ";
				break;
			case MovementResult::DRAW_DEFAULT:
				s << "DRAW_DEFAULT ";
				break;
			case MovementResult::BAD_SETUP:
				s << "BOTH_ILLEGAL ";
				break;	
			default:
				s << "INTERNAL_ERROR ";
				break;	
		}
	}
	
	s << Game::theGame->TurnCount() << " " << Game::theGame->theBoard.TotalPieceValue(Piece::RED) << " " << Game::theGame->theBoard.TotalPieceValue(Piece::BLUE);

	buffer = s.str();
	

}

void DestroyGame()
{
	delete Game::theGame;
	Game::theGame = NULL;
}
