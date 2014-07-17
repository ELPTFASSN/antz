/* -----------------------------------------------------------------------------
*
*  main.c
*
*  ANTz - realtime 3D data visualization tools for the real-world, based on NPE.
*
*  Hosted at http://openantz.com and NPE at http://neuralphysics.org
*
*  Written in 2010-2014 by Shane Saxon - saxon@openantz.com
*
*  Additional code contributors are listed chronologically: Lucas Erickson, 
*  Mark Buchanan and Johannes Johannsen
*
*  Special thanks to Dave Warner, project synergist and conceptual architect.
*  Additional gratitude to Jeff Sale for creating great data visualizations.
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

#include "npdata.h"					/* scene graph and other data structures  */
#include "npio.h"					/* file IO, MySQL, OSC-UDP, OpenGL        */
#include "npctrl.h"					/* command logic with physics             */

/* ---------------------------------------------------------------------------
*  MVC architecture variant where View becomes a subset of the IO group
*  Model named Data (map) is a global scene graph
*  View named IO, includes keyboard, mice, haptic devices, audio, video...
*  Control named Ctrl, a pseudo state-machine based on physics, excludes IO
*
*  Ctrl modifies the scene state and the Neural Physics Engine updates it.
*  Platform specific functions are separately placed in the 'os' subfolders.
*
*  Currently only a single (global) dataRef instance is used, but expect that
*  multiple instances will be created to support several users and GL contexts.
*
*  CPU load is currently 15% NPE, 5% qsort, 80% GL commands
*  main() can be compiled as either C or C++
* --------------------------------------------------------------------------- */
int main (int argc, char **argv)
{
	int err = 0;

	void* data = NULL;	

	data = npInitData (argc, argv);			// Model
	npInitIO (data);						// View
	npInitCtrl (data);						// Control

	err = npAppLoop (data);

	npCloseCtrl (data);
	npCloseIO (data);
	npCloseData (data);

	return err;
}

