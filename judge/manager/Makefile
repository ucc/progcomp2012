#Makefile for Stratego

#Use this to build with graphics
CPP = g++ -Wall -pedantic -lSDL -lGL -lpthread -g
#Use this to build without graphics
#CPP = g++ -Wall -pedantic -lpthread -g
OBJ = main.o controller.o ai_controller.o human_controller.o program.o thread_util.o stratego.o graphics.o game.o

BIN = stratego



$(BIN) : $(OBJ) 
	$(CPP) -o $(BIN) $(OBJ)




%.o : %.cpp %.h
	$(CPP) -c $<

clean :
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)

clean_full: #cleans up all backup files
	$(RM) $(BIN) $(OBJ) $(LINKOBJ)
	$(RM) *.*~
	$(RM) *~


