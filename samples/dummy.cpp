#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
using namespace std;

/**
 * A suitably terrible program which combines C style IO with C++ style IO
 * Enjoy!
 * Mwuhahaha
 */

int main(int argc, char ** argv)
{
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);
	

	//Read in the colour, and choose a layout
	
	int width = 14; int height = 14;
	
	string colour; string opponent;
	cin >> colour; cin >> opponent; cin >> width; cin >> height;
	fgetc(stdin);

	//fprintf(stderr, "Colour is \"%s\", width and height are (%d, %d), opponent is \"%s\"\n", colour.c_str(), width, height, opponent.c_str());

	assert(width == 14 && height == 14); //Can't deal with other sized boards
	if (colour == "RED")
	{
		fprintf(stdout, "FB..........B.\n");
		fprintf(stdout, "BBCM....cccc.C\n");
		fprintf(stdout, "LSGmnsBmSsnsSm\n");
		fprintf(stdout, "sLSBLnLssssnyn\n");
	}
	else if (colour == "BLUE")
	{
		fprintf(stdout, "sLSBLnLssssnyn\n");
		fprintf(stdout, "LSGmnsBmSsnsSm\n");
		fprintf(stdout, "BBCM....cccc.C\n");
		fprintf(stdout, "FB..........B.\n");
	}
	else
	{
		return 1;
	}



	char board[width][height];

	vector<pair<int, int> > choices;

	int myPid = (int)(getpid());

	while (true)
	{
		//fprintf(stderr, "%s [%d] looping\n", argv[0], myPid);
		choices.clear();

	//	fprintf(stderr, "%s Waiting for status line...\n", colour.c_str());
		while (fgetc(stdin) != '\n')
		{
			//fprintf(stderr,".");
		}
			//fprintf(stderr, "%s Got status, waiting for board line...\n", colour.c_str());

		//Read in board
		for (int y=0; y < height; ++y)
		{
			for (int x=0; x < width; ++x)
			{
				board[x][y] = fgetc(stdin);
				if (board[x][y] == EOF)
					exit(EXIT_SUCCESS);		

				if (board[x][y] != '.' && board[x][y] != '*' && board[x][y] != '#' && board[x][y] != '+')
				{	
					choices.push_back(pair<int, int>(x, y));
				}
			}
			assert(fgetc(stdin) == '\n');
		}

		

		int dir = 0; int startDir = 0; int choice = rand() % choices.size(); int startChoice = choice;
		int x1 = 0; int y1 = 0;
		do
		{
			
		
			pair<int,int> pear = choices[choice];
			x1 = pear.first;
			y1 = pear.second;
			//fprintf(stderr,"Trying unit at %d %d...\n", x1, y1);

			if (board[x1][y1] == 'B' || board[x1][y1] == 'F')
			{
				choice = (choice+1) % choices.size();
				continue;
			}
			
			int x2 = x1;
			int y2 = y1;
			dir = rand() % 4; startDir = dir; int lastDir = dir;

			bool okay = false;
			while (!okay)
			{
				//fprintf(stderr,"	Trying direction %d...\n", dir);				
				x2 = x1; y2 = y1;
				switch (dir)
				{
				case 0:
					--y2;
					break;
				case 1:
					++y2;
					break;
				case 2:
					--x2;
					break;
				case 3:
					++x2;
					break;
				}

				okay = !(x2 < 0 || y2 < 0 || x2 >= width || y2 >= height || (board[x2][y2] != '.' && board[x2][y2] != '*' && board[x2][y2] != '#'));
				if (!okay)
				{
					dir = (dir+1) % 4;
					if (dir == startDir)
						break;
				}
				
			}
			


			choice = (choice+1) % choices.size();
			if (dir != startDir)
				break;
		}
		while (choice != startChoice);
		

		string direction="";
		switch (dir)
		{
			case 0:
				direction = "UP";
				break;
			case 1:
				direction = "DOWN";
				break;
			case 2:
				direction = "LEFT";
				break;
			case 3:
				direction = "RIGHT";
				break;
		}
		printf("%d %d %s\n", x1, y1, direction.c_str());
		//fprintf(stderr,"%s Made move, waiting for confirmation line\n", colour.c_str());
		while (fgetc(stdin) != '\n'); //Read in result line
		//fprintf(stderr, "%s Done turn\n", colour.c_str());
		//fprintf(stderr,"%s - %d %d %s\n",colour.c_str(),  x1, y1, direction.c_str() );
		//fprintf(stderr, "%s [%d] computed move\n", argv[0], myPid);
	}
}
