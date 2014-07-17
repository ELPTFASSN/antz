/* -----------------------------------------------------------------------------
*
*  npglut.c
*
*  ANTz - realtime 3D data visualization tools for the real-world, based on NPE.
*
*  ANTz is hosted at http://openantz.com and NPE at http://neuralphysics.org
*
*  Written in 2010-2014 by Shane Saxon - saxon@openantz.com
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

#include "npglut.h"
#include "../../libs/soil/src/SOIL.h"						//zz debug screenGrab

#include "../npio.h"
#include "../npctrl.h"				//remove once npPostMsg is global, debug zz

#include "npgl.h"
#include "npkey.h"
#include "npmouse.h"

#include "../os/npos.h"

#include "../data/npmapfile.h"		//debug, zz


void npGlutInitGL();
void npGlutDrawGLScene(void);
void npGlutDrawGLSceneIdle(void);
// void npGlutResizeGLScene(int Width, int Height);

void npGlutKeyDown (unsigned char key, int x, int y);
void npGlutKeyUp (unsigned char key, int x, int y);
void npGlutKeyDownSpecial (int key, int x, int y);
void npGlutKeyUpSpecial (int key, int x, int y);

//the intent of this file is wrap glut functions and glut specific code
//its 'npgl.h' that handles the primary OpenGL routines
//app is designed to be wrapped by GLUT or other OS methods, wgl, Cocoa, etc...]

//update to allow entering and leaving game mode without re-loading textures
// wglShareLists() function in MSW and glXCreateContext() under X Windows. zz
//------------------------------------------------------------------------------
void npInitGlut (int argc, char **argv, void* dataRef)
{
	GLboolean stereoSupport = false;
	int depth = 0;
	int result = 0;
	int gMainWindow = 0;
	char msg[256];

	pData data = (pData) dataRef;
	pNPgl gl = &data->io.gl;

	glutInit (&argc, argv);

//zz debug, move OS specific code to npos.h ?	
	//stereo3D not yet supported on OSX, zz debug
#ifndef NP_OSX_
	sprintf (msg, "freeglut ver: %d", glutGet(GLUT_VERSION));
	npPostMsg (msg, kNPmsgCtrl, data);

	glGetBooleanv (GL_STEREO, &stereoSupport);

#else
	npPostMsg ("Apple GLUT", kNPmsgCtrl, data);

	stereoSupport = false;			
#endif
	
	gl->stereo3D = stereoSupport;

	if (gl->stereo3D)
		sprintf (msg, "OpenGL Stereo 3D: YES");
	else
		sprintf (msg, "OpenGL Stereo 3D: NO");
	npPostMsg (msg, kNPmsgCtrl, data);

	//OpenGL stereo 3D currently ONLY supported by Quadro and AMD Fire Pro series
	//at this time the bios is different consumer cards use same chip die...?

	if (gl->stereo3D)
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO);
	else
		glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);			//add SRGBA support, zz

	glutInitWindowPosition (gl->position.x, gl->position.y);
	glutInitWindowSize (gl->windowSize.x, gl->windowSize.y);
	gl->windowID = glutCreateWindow (gl->name);
	
	glutSetWindow (gl->windowID);			//S3D
	glutHideWindow();						//S3D

/*	//stereo 3D and GameMode
	//if (gl->gameMode)
	printf("Attempting Game Mode, please ignore GLUT warnings\n");
	glutGameModeString("1920x1080:32@121");
	if ( glutEnterGameMode() == 0 )
	{
		glutGameModeString("1920x1080:32@120");
		if ( glutEnterGameMode() == 0 )
		{
			glutGameModeString("1920x1080:32@119");
			if ( glutEnterGameMode() == 0 )
			{
				glutGameModeString("1920x1080:32@60"); //does not specify refresh
				result = ( glutEnterGameMode() );
	}}}	//add more res... 1680x1050 1366x768 1280x720 1280x1024, 1024x768, also 100Hz and 110Hz
*/
//	wglSwapIntervalEXT(1);		//to remove frame tearing with VSYNC debug, zz
//	nposSwapInterval(1);		//move OS methods to npos.h, debug zz
//	glfwSwapInterval(1);		//may use the glfw libarry, not implememted, zz

	//register keyboard events with GLUT
	glutKeyboardFunc (npGlutKeyDown);
	glutKeyboardUpFunc (npGlutKeyUp);
	glutSpecialFunc (npGlutKeyDownSpecial);
	glutSpecialUpFunc (npGlutKeyUpSpecial);
	
	//register mouse events with GLUT
	glutMouseFunc (npMouseEvent);
	glutMotionFunc (npMouseMotion);
	glutMouseWheelFunc (npMouseWheel);

	glutDisplayFunc (npGlutDrawGLScene);
	glutIdleFunc (npGlutDrawGLSceneIdle);

	glutReshapeFunc (npGLResizeScene);
	
	//---
	npInitGL (dataRef);

	glutShowWindow ();

	if (gl->fullscreen)
	{
		npPostMsg ("FullScreen Window", kNPmsgCtrl, dataRef);
		glutFullScreen ();
	}		


//		gl->stereo3D = false;		
/*
	if (result == 0)
	{
		npPostMsg ("FullScreen Window", kNPmsgCtrl, dataRef);
		glutShowWindow ();
		glutFullScreen ();
//		gl->stereo3D = false;								//stereo 3D, debug zz
	}
	else
	{	//GameMode may be different then what we requested, so get the modes
		glutSetWindowTitle("ANTz - GameMode");

		gl->width = glutGameModeGet( GLUT_GAME_MODE_WIDTH );
		gl->height = glutGameModeGet( GLUT_GAME_MODE_HEIGHT );
		gl->pixelDepth = glutGameModeGet( GLUT_GAME_MODE_PIXEL_DEPTH );
		gl->refreshRate = (float)glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE );
		printf("FullScreen Game Mode: %dx%d:%d@%d\n",gl->width,gl->height,
						gl->pixelDepth, (int)gl->refreshRate);
		//stereo 3D, turn off stereo if reresh rate is too low, update this, zz
		if (gl->refreshRate >= 99)
			printf("Stereo 3D enabled\n");
		else
		{
			if (gl->stereo3D)
				printf("Stereo 3D disabled, low refresh rate: %dHz\n", 
					gl->refreshRate);
			gl->stereo3D = false;
		}
	}
*/
	npPostMsg ("www.openANTz.com", kNPmsgCtrl, dataRef);
	npPostMsg ("System Ready...", kNPmsgCtrl, dataRef);
	data->ctrl.startup = false;
	//glGetIntegerv (GL_TEXTURE_STACK_DEPTH, &depth); // GL_MODELVIEW_STACK_DEPTH
}


//------------------------------------------------------------------------------
void npCloseGlut (void* dataRef)
{
	npCloseGL (dataRef);

	return;
}


//------------------------------------------------------------------------------
void npAppLoopGlut (void* dataRef)
{
	npInitGlut (0, 0, dataRef);
	glutMainLoop();
}


void npScreenGrab( char* filePath, int type, int x, int y, int w, int h, void* dataRef);
//zz screenGrab
//grabs back buffer from current OpenGL context
//this function needs to be called just before glSwapBuffers()
//------------------------------------------------------------------------------
void npScreenGrab( char* filePath, int type, int x, int y, int w, int h,  void* dataRef )
{
	int i = 0;
	int err = 0;
	int temp = 0; //store console level
	unsigned char* pixelBuf = NULL;
//	char filePath[kNPmaxPath];
	static char timeStamp[64];

	static unsigned char tempLine[65535];	//max fixed array size is 65535

	w = glutGet( GLUT_WINDOW_WIDTH );
	h = glutGet( GLUT_WINDOW_HEIGHT );

	pixelBuf = (unsigned char*)npMalloc( 0, w*h*3, dataRef );
	if (!pixelBuf) return;
   
    glPixelStorei( GL_PACK_ALIGNMENT, 1 );
    glReadPixels( 0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixelBuf );
	
	//RGBA is good for DB thumbnails... see-through dataset snapshot
	//backbuffer needs to be black with alpha = 0
	//glReadPixels( 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, &buf[0] );	 

	for (i=0; i < (h / 2); i++);
	{	
		memcpy( tempLine, &pixelBuf[i * w * 3], w );
		memcpy( &pixelBuf[i * w * 3], &pixelBuf[(h-i) * w * 3], w );
		memcpy( &pixelBuf[(h-i) * w * 3], tempLine, w );
	}

	nposTimeStampCSV( timeStamp );
	sprintf( filePath, "usr/images/%s.bmp", timeStamp );
    SOIL_save_image
		(
        filePath,
        SOIL_SAVE_TYPE_BMP, //SOIL_SAVE_TYPE_TGA, //SOIL_SAVE_TYPE_DDS
        w, h, 3,
        pixelBuf
        );
}


//------------------------------------------------------------------------------
void npGlutDrawGLScene(void) 
{
		//screenGrab
		int temp = 0; //store console level
		int i = 0;
        int w = 0, tempW = 0;
        int h = 0, tempH = 0;

		static char filePath[kNPmaxPath];
		static char timeStamp[64];
		char msg[kNPmaxPath];
		int err = 0;

	double deltaTime = 0.0;
	double currentTime = 0.0;
	double targePeriod = 0.0;

	pData data = (pData) npGetDataRef();
		
	//update data, positions, physics, user input
	npUpdateCtrl (data);


	//screenGrab
    if( data->io.gl.screenGrab ) //screenGrab F12
    {
		w = glutGet( GLUT_WINDOW_WIDTH );
		h = glutGet( GLUT_WINDOW_HEIGHT );

		w = tempW = data->io.gl.width;
		h = tempH = data->io.gl.height;

		//construct the filename path
		nposTimeStampCSV( timeStamp );
		strcpy( filePath, data->io.file.mapPath );
		strcat( filePath, timeStamp );
		
		//resize to thumbnail and turn off HUD
		if( data->io.gl.screenGrab == 2 )
		{
			filePath[0] = '\0';
			strncat( filePath, data->io.gl.datasetName, kNPmaxPath - 4  );
			strcat( filePath, ".dds" );

			w = 480; // 320 // 160 // data->map.thumbSize.width
			h = 270; // 180 // 90  // data->map.thumbSize.height
			temp = data->io.gl.hud.console.level;
			data->io.gl.hud.console.level = 0;
			npGLResizeScene( w, h );
			npGLDrawScene( data );
			
			err = SOIL_save_screenshot		//grab backbuffer and write to file
				(
				filePath,
				//SOIL_SAVE_TYPE_BMP,
				//SOIL_SAVE_TYPE_TGA,
				SOIL_SAVE_TYPE_DDS, 
				0, 0, w, h
				);

			//restore screen size and re-render
			data->io.gl.hud.console.level = temp;
			npGLResizeScene(tempW, tempH);
			npGLDrawScene (data);
			sprintf(msg, "Saved Thumbnail: %s", filePath);
		}
		else
		{
			strcat( filePath, ".bmp" );
		//	strcat( filePath, ".tga" );
		
			npGLDrawScene (data);

			err = SOIL_save_screenshot		//grab backbuffer and write to file
				(
				filePath,
				SOIL_SAVE_TYPE_BMP,
				//SOIL_SAVE_TYPE_TGA,
				//SOIL_SAVE_TYPE_DDS, 
				0, 0, w, h
				);
			sprintf(msg, "Saved Screenshot: %s", filePath);
		}
		npPostMsg( msg, kNPmsgFile, data );

		data->io.gl.screenGrab = false;
    }
	else  //init time on first run
		npGLDrawScene (data);

//	glFlush();		//zzhp							//vsync, not yet functional, debug zz
//	glFinish();		//zzhp

	//calculate the delta time using previously stored time
	currentTime = nposGetTime();
	deltaTime = currentTime - data->io.time;	//add clock roll-over handling, debug zz

	//if extra time left over then sleep for remainder of cycle
	if (deltaTime < data->io.cyclePeriod)
		nposSleep (data->io.cyclePeriod - deltaTime);

	glutPostRedisplay();		//supposed to help with window freezing,	debug zz
								//issue with mouse hitting edge of screen
	glutSwapBuffers();

//	glFinish();					//vsync

	//update the locally stored time, used when calculating the delta time
	nposUpdateTime (data);

	if( !data->io.gl.fullscreen )
	{
		data->io.gl.position.x = glutGet(GLUT_WINDOW_X);		//S3D
		data->io.gl.position.y = glutGet(GLUT_WINDOW_Y);
	}
}

//------------------------------------------------------------------------------

void npGlutDrawGLSceneIdle(void)
{
//	npGlutDrawGLScene();

	return;
}

//fullscreen GameMode creates a new GL context
//currently requires re-registering event callbacks and re-loading texture maps
//------------------------------------------------------------------------------
void npglFullscreen (void* dataRef)
{
	int deltaX = 0, deltaY = 0;
	int result = 0;		//fullscreen window used only if gamemode fails

	pData data = (pData) dataRef;
	pNPgl gl = &data->io.gl;

	if (gl->fullscreen)
	{
		if (glutGameModeGet(GLUT_GAME_MODE_ACTIVE) != 0)		//stereo 3D
		{
			glutLeaveGameMode();
			glutSetWindow (gl->windowID);
            glutShowWindow ();

			glutKeyboardFunc (npGlutKeyDown);
			glutKeyboardUpFunc (npGlutKeyUp);
			glutSpecialFunc (npGlutKeyDownSpecial);
			glutSpecialUpFunc (npGlutKeyUpSpecial);


			//register mouse events with GLUT
			glutMouseFunc (npMouseEvent);
			glutMotionFunc (npMouseMotion);

			glutDisplayFunc (npGlutDrawGLScene);
			glutIdleFunc (npGlutDrawGLSceneIdle);

			glutReshapeFunc (npGLResizeScene);
			//---
			npInitGL (data);
		}
	
		//exit fullscreen and restore previous window position
		gl->fullscreen = false;

		glutReshapeWindow (gl->windowSize.x, gl->windowSize.y);
		glutPositionWindow (gl->position.x, gl->position.y);

		//correct for window border offset, glut workaround
		deltaX = gl->position.x - glutGet((GLenum)GLUT_WINDOW_X);
		deltaY = gl->position.y - glutGet((GLenum)GLUT_WINDOW_Y);
		if (deltaX != 0 || deltaY != 0)
			glutPositionWindow (gl->position.x + deltaX,
								gl->position.y + deltaY);
		
		//zz debug err checking to make sure window is proper size!
/*
		if (x != gl->windowSize.x);
		{
			gl->windowSize.x = x;
			gl->windowSize.y = y;
			glutReshapeWindow (x, y);
			glutPositionWindow (100, 100);

			npPostMsg("err 2978 - window width mismatch", kNPmsgGL, data);
		}
*/

	
		npPostMsg("Exit FullScreen", kNPmsgGL, data);
	}
	else
	{
/*		glutSetWindow (	gl->windowID);
		glutHideWindow ();

		//Game Mode with stereo 3D
		if (gl->stereo3D)
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STEREO); // stereo display mode for glut
		else
			glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

		//stereo 3D and GameMode
		printf("Attempting Game Mode, please ignore GLUT warnings\n");
		glutGameModeString("1920x1080:32@121");
		if ( glutEnterGameMode() == 0 )
		{
			glutGameModeString("1920x1080:32@120");
			if ( glutEnterGameMode() == 0 )
			{
				glutGameModeString("1920x1080:32@119");
				if ( glutEnterGameMode() == 0 )
				{
					glutGameModeString("1920x1080:32@60"); //does not specify refresh
					result = ( glutEnterGameMode() );
		}}}

		//register keyboard events with the new GameMode GL context
		glutKeyboardFunc (npGlutKeyDown);
		glutKeyboardUpFunc (npGlutKeyUp);
		glutSpecialFunc (npGlutKeyDownSpecial);
		glutSpecialUpFunc (npGlutKeyUpSpecial);
		
		//register mouse events with GLUT
		glutMouseFunc (npMouseEvent);
		glutMotionFunc (npMouseMotion);

		glutDisplayFunc (npGlutDrawGLScene);
		glutIdleFunc (npGlutDrawGLSceneIdle);
		glutReshapeFunc (npGLResizeScene);
		
		//---
		npInitGL (data);
*/
		gl->position.x = glutGet((GLenum)GLUT_WINDOW_X);
		gl->position.y = glutGet((GLenum)GLUT_WINDOW_Y);

		gl->windowSize.x = gl->width;
		gl->windowSize.y = gl->height;

		if (result == 0)	//fullscreen window used only if gamemode fails
		{
			printf("FullScreen Window\n");
			glutShowWindow ();
			glutFullScreen ();
			//gl->stereo3D = false;							// 3D, debug zz
		}
		else
		{	//GameMode may be different then what we requested, so get the modes
			glutSetWindowTitle("ANTz - GameMode");

			gl->width = glutGameModeGet( GLUT_GAME_MODE_WIDTH );
			gl->height = glutGameModeGet( GLUT_GAME_MODE_HEIGHT );
			gl->pixelDepth = glutGameModeGet( GLUT_GAME_MODE_PIXEL_DEPTH );
			gl->refreshRate = (float)glutGameModeGet( GLUT_GAME_MODE_REFRESH_RATE );
			printf("FullScreen Game Mode: %dx%d:%d@%d\n", gl->width, gl->height,
							gl->pixelDepth, (int)gl->refreshRate);
			//stereo 3D, turn off stereo if reresh rate is too low, update this, zz
			if (gl->refreshRate >= 99)
			{
				printf("Stereo 3D enabled\n");
				gl->stereo3D = true;
			}
			else
			{
				printf("Stereo 3D disabled, low refresh rate\n");
				gl->stereo3D = false;
			}
		}

		gl->fullscreen = true;
	}
}


//------------------------------------------------------------------------------
void npGlutKeyDown (unsigned char key, int x, int y) 
{
	// printf("1 key: %d \n", key);
	npKeyGlut (key, x, y, kGlutKeyDown, 0);		// glutGetModifiers()); //zz debug, not a problem here, just no longer needed
}

//------------------------------------------------------------------------------
void npGlutKeyUp (unsigned char key, int x, int y) 
{
	// printf("2 key: %d \n", key);
	npKeyGlut (key, x, y, kGlutKeyUp, 0);		// glutGetModifiers()); //zz debug
}

//------------------------------------------------------------------------------
void npGlutKeyDownSpecial (int key, int x, int y) 
{
	// printf("3 key: %d \n", key);
	npKeyGlut (key, x, y, kGlutKeyDownSpecial, 0);	// glutGetModifiers());	
	
	//zz debug, if glutGetModifiers() called from a ...Special callback 
	//then err - "freeglut glutGetModifiers() called outside an input callback"
}

//------------------------------------------------------------------------------
void npGlutKeyUpSpecial (int key, int x, int y) 
{
	// printf("4 key: %d \n", key);
	npKeyGlut (key, x, y, kGlutKeyUpSpecial, 0);	// glutGetModifiers());	
}


//------------------------------------------------------------------------------
void npGLSolidSphere (GLdouble radius, GLint slices, GLint stacks)
{
	glutSolidSphere (radius, slices, stacks);
}

//------------------------------------------------------------------------------
void npGLWireSphere (GLdouble radius, GLint slices, GLint stacks)
{
	glutWireSphere (radius, slices, stacks);
}

//------------------------------------------------------------------------------
void npGLSolidCone (GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	glutSolidCone (base, height, slices, stacks);
}

//------------------------------------------------------------------------------
void npGLWireCone (GLdouble base, GLdouble height, GLint slices, GLint stacks)
{
	glutWireCone (base, height, slices, stacks);
}

//------------------------------------------------------------------------------
void npGLSolidTorus (GLdouble innerRadius, GLdouble outerRadius, 
					GLint slices, GLint stacks)
{
	glutSolidTorus (innerRadius, outerRadius, slices, stacks);
}

//------------------------------------------------------------------------------
void npGLWireTorus (GLdouble innerRadius, GLdouble outerRadius, 
					GLint slices, GLint stacks)
{
	glutWireTorus (innerRadius, outerRadius, slices, stacks);
}

//------------------------------------------------------------------------------
void npGlutPrimitive (int primitive)
{
	switch (primitive)
	{
	case kNPgeoCubeWire :
		glScalef (0.6f, 0.6f, 0.6f); 
		glutWireCube(2.0f);
		glScalef (1.666667f, 1.666667f, 1.666667f);
		break;
	case kNPgeoCube :
		glScalef (0.6f, 0.6f, 0.6f);
		glutSolidCube(2.0f);
		glScalef (1.666667f, 1.666667f, 1.666667f);
		break;
	case kNPgeoSphereWire : glutWireSphere( 1.0f, 24, 12); break;//15, 15 ); break;
	case kNPgeoSphere : glutSolidSphere( 1.0f, 24, 12 ); break;

	case kNPgeoConeWire : glutWireCone( 1.0f, 2.0f, 24, 1 ); break;
	case kNPgeoCone : glutSolidCone( 1.0f, 2.0f, 24, 1 ); break;

	case kNPgeoTorusWire : glutWireTorus(kNPtorusRadius * 0.1f, kNPtorusRadius, 7, 16); break;
	case kNPgeoTorus : glutSolidTorus(kNPtorusRadius * 0.1f, kNPtorusRadius, 7, 16); break;

	case kNPgeoDodecahedronWire :
		glScalef (0.6f, 0.6f, 0.6f);
		glutWireDodecahedron();
		glScalef (1.666667f, 1.666667f, 1.666667f);
		break;
	case kNPgeoDodecahedron :
		glScalef (0.6f, 0.6f, 0.6f);
		glutSolidDodecahedron();
		glScalef (1.666667f, 1.666667f, 1.666667f);
		break;
	case kNPgeoOctahedronWire : glutWireOctahedron(); break;
	case kNPgeoOctahedron : glutSolidOctahedron(); break;
	case kNPgeoTetrahedronWire : glutWireTetrahedron(); break;
	case kNPgeoTetrahedron : glutSolidTetrahedron(); break;
	case kNPgeoIcosahedronWire : glutWireIcosahedron(); break;
	case kNPgeoIcosahedron : glutSolidIcosahedron(); break;

//	case kNPglutWireTeapot : glutWireTeapot( 2.0f ); break;
//	case kNPglutSolidTeapot : glutSolidTeapot( 2.0f ); break;

	default : glutWireTetrahedron(); break;
	}
}

