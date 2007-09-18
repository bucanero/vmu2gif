# dci4vmi util
# (c)2005 Damian Parrino
# $Id: Makefile,v 1.1.1.1 2001/09/26 07:05:00 bardtx Exp $

all:
	g++ -c dcvmu.cpp
	g++ -c gifsave.cpp
	g++ -c vmu2gif.cpp
	g++ -o vmu2gif dcvmu.o gifsave.o vmu2gif.o
