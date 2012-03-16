#Makefile for basic_java
# Sample C++ Stratego AI
# UCC Programming Competition 2012 

BasicAI : basic.java Reader.java piece.java
	gcj -o BasicAI --main=BasicAI basic.java Reader.java piece.java

BasicAI.class : basic.java Reader.java piece.java
	javac Reader.java
	javac piece.java
	javac basic.java

clean : 
	rm -f BasicAI
	rm -f *.class


