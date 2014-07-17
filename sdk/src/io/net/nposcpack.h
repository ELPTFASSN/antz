/* -----------------------------------------------------------------------------
*
*  nposcpack.h
*
*  ANTz - realtime 3D data visualization tools for the real-world, based on NPE.
*
*  ANTz is hosted at http://openantz.com and NPE at http://neuralphysics.org
*
*  Written in 2010-2014 by Shane Saxon - makecontact@saxondigital.net
*
*  Please see main.c for a complete list of additional code contributors.
*
*  To the extent possible under law, the author(s) have dedicated all copyright 
*  and related and neighboring rights to this software to the public domain
*  worldwide. This software is distributed without any warranty.
*
*  Released under the CC0 license, which is GPL compatible.
*
*  You should have received a copy of the CC0 Public Domain Dedication along
*  with this software (license file named COPYING.txt). If not, see
*  http://creativecommons.org/publicdomain/zero/1.0/
*
* --------------------------------------------------------------------------- */
/* 
*  This file provides a C-Wrapper for the C++ based oscpack library
*
* --------------------------------------------------------------------------- */

#ifndef NPOSCPACK_H_
#define NPOSCPACK_H_


struct NPoscPackListener {
	int id;

	int txPort;
	int rxPort;
	//int oscPackIdx;  // index into array that holds the oscpack instances
};
typedef struct NPoscPackListener NPoscPackListener;
typedef struct NPoscPackListener* pNPoscPackListener;

// listener functions
void npStartListeners( pNPoscPackListener oscListener, int argc, char** argv, void* dataRef );
void npInitOscPackListener( pNPoscPackListener oscListener, int outgoing, int incoming, void* dataRef );
void npStartOscPackListener( pNPoscPackListener oscListener );

// read a line that has been received by listener
char* npOscReadLine( char* bufferStart, int maxLength, int udpFd );

struct NPoscPackSender {
	int oscPackIdx; // index into array that holds the oscpack instances
};

typedef struct NPoscPackSender NPoscPackSender;
typedef struct NPoscPackSender* pNPoscPackSender;

// Tx sender functions
void npInitOscPackSender( pNPoscPackSender oscSender, char* ip, int port );
void npCreateOscPackBundle( pNPoscPackSender oscSender );
void npBeginOscPackMessage( pNPoscPackSender oscSender, char* message );
void npOscAddString( pNPoscPackSender oscSender, char* s );
void npOscAddInt( pNPoscPackSender oscSender, int i );
void npOscAddFloat( pNPoscPackSender oscSender, float f );
void npOscAddBool( pNPoscPackSender oscSender, bool b );
void npEndOscPackMessage( pNPoscPackSender oscSender );
void npSendOscPackBundle( pNPoscPackSender oscSender );

/*	//zz-JJ may want something like this... not sure yet...
// Rx receive functions
void npOscGetNTP( long int timeStampNTP );
void npOscGetString( pNPoscPackSender oscSender, char* s );
void npOscGetInt( pNPoscPackSender oscSender, int i );
void npOscGetFloat( pNPoscPackSender oscSender, float f );
void npOscGetBool( pNPoscPackSender oscSender, bool b );
*/

//send OSC message  //zz-jj moved from nposc.c
void npOscTx( int oscID, char* addr, char* tag, void** arguments, void* dataRef);
//
void npOscListenerThread (void* oscListenerX);	//JJ-zz called from nposc

#define OSC_csvCh "/csvCh"
#define OSC_csvChMap "/csvChMap"
#define OSC_csvNode "/csvNode"
#define OSC_csvTag "/csvTag"

#define READLINE "/readline"
#define DUMP "/dump"


#endif

