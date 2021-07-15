/*
 *  $Id: sample.cpp
 *  hog2
 *
 *  Created by Nathan Sturtevant on 5/31/05.
 *  Modified by Nathan Sturtevant on 02/29/20.
 *
 * This file is part of HOG2. See https://github.com/nathansttt/hog2 for licensing information.
 *
 */

#include "Common.h"
#include "Driver.h"
#include "GraphEnvironment.h"
#include <string>
#include "Racetrack.h"


bool recording = false;
bool running = false;
bool move = false;

Map *m = 0;
Racetrack *r = 0;
RacetrackState s;
RacetrackMove v;

// -------------- MAIN FUNCTION ----------- //
int main(int argc, char* argv[])
{
	InstallHandlers();
	RunHOGGUI(argc, argv, 1600, 800);
	return 0;
}


/**
 * Allows you to install any keyboard handlers needed for program interaction.
 */
void InstallHandlers()
{
	InstallKeyboardHandler(MyDisplayHandler, "Reset", "Reset to start state", kAnyModifier, 'r');
	// TODO: Add new handlers to enable use of WASD -- done
	InstallKeyboardHandler(MyDisplayHandler, "Up", "Accelerate upwards", kAnyModifier, kUpArrow);
	InstallKeyboardHandler(MyDisplayHandler, "Down", "Accelerate downwards", kAnyModifier, kDownArrow);
	InstallKeyboardHandler(MyDisplayHandler, "Left", "Accelerate left", kAnyModifier, kLeftArrow);
	InstallKeyboardHandler(MyDisplayHandler, "Right", "Accelerate right", kAnyModifier, kRightArrow);
	// --- WASD handlers --- //
	InstallKeyboardHandler(MyDisplayHandler, "W", "Accelerate upwards", kAnyModifier, 'w');
	InstallKeyboardHandler(MyDisplayHandler, "A", "Accelerate Left", kAnyModifier, 'a');
	InstallKeyboardHandler(MyDisplayHandler, "S", "Accelerate downwards", kAnyModifier, 's');
	InstallKeyboardHandler(MyDisplayHandler, "D", "Accelerate Right", kAnyModifier, 'd');

	InstallKeyboardHandler(MyDisplayHandler, "M", "Move automatically", kAnyModifier, 'm');

	InstallWindowHandler(MyWindowHandler);

	InstallMouseClickHandler(MyClickHandler, static_cast<tMouseEventType>(kMouseMove|kMouseDown));
	srandom(time(0));
}

void MyWindowHandler(unsigned long windowID, tWindowEventType eType)
{
	if (eType == kWindowDestroyed)
	{
		printf("Window %ld destroyed\n", windowID);
		RemoveFrameHandler(MyFrameHandler, windowID, 0);
	}
	else if (eType == kWindowCreated)
	{
		printf("Window %ld created\n", windowID);
		InstallFrameHandler(MyFrameHandler, windowID, 0);
		ReinitViewports(windowID, {-1, -1, 1, 1}, kScaleToSquare);
		
		m = new Map(11, 11); // makes a map with the dimensions in the parentheses
		
		for (int x = 0; x <10; x++)
		{
			m->SetTerrainType(x, 0, kStartTerrain); // Start terrain placed down
		}
		for (int x = 0; x < 5; x++)
		{
			m->SetTerrainType(x, 5, kTrees); // Tree terrain placed down
		}

		for (int x = 0; x < 7; x++)
		{
			m->SetTerrainType(x, m->GetMapHeight()-1, kEndTerrain); // End terrain
		}
		r = new Racetrack(m);
		r->Reset(s);
	}
}


void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *)
{
	Graphics::Display &display = getCurrentContext()->display;
	display.FillRect({-1, -1, 1, 1}, Colors::black);
	
	// Draw map
	r->Draw(display);
	// Draw "racecar"
	r->Draw(display, s); //Draws the state of the racetrack
	r->GoalTest(s, s);
	if (move == true) // if m was pressed, makes the agent move automatically
	{
		r->ApplyAction(s, v);
	}
	Boundaries(); // doesn't let the agent out of the map
	return;
}

int MyCLHandler(char *argument[], int maxNumArgs)
{
	if (maxNumArgs <= 1)
		return 0;
	strncpy(gDefaultMap, argument[1], 1024);
	return 2;
}



uint64_t random64()
{
	uint64_t r1 = random();
	uint64_t r2 = random();
	return (r1<<32)|r2;
}
// ------------ Boundaries ----------- //
void Boundaries()
{
	if (s.loc.x > 60000)
	{
		std::cout << "Too far left!! D: \n";
		// makes the agent stop when it hits the wall
		s.loc.x = 0;
		s.xVelocity = 0;
		v.xDelta = 0;
	}
	else if (s.loc.x >= m->GetMapWidth() - 1)
	{
		std::cout << "Too far right! \n";
		s.loc.x = m->GetMapWidth()-1;
		s.xVelocity = 0;
		v.xDelta = 0;
	}
	if (s.loc.y > 60000)
	{
		std::cout << "Too high up!! \n";
		// makes the agent stop
		s.loc.y = 0;
		s.yVelocity = 0;
		v.yDelta = 0;
	}
	
	else if (s.loc.y >= m->GetMapHeight() - 1)
	{
		std::cout << "Too far down! \n";
		s.loc.y = m->GetMapHeight()-1;
		s.yVelocity = 0;
		v.yDelta = 0;
	}
	// std::cout << s.loc.x << ", " << s.loc.y << std::endl;
	// std::cout << m->GetMapWidth() << ", " << m->GetMapHeight() << std::endl;
	
}


void MyDisplayHandler(unsigned long windowID, tKeyboardModifier mod, char key) // handles keypresses that change display
{
	switch (key)
	{
		case 'r':
			// TODO: Reset to start state
			r->Reset(s);
			break;
			// TODO: Make appropriate movements - done
			// TODO: Add support for WASD here - done
		
		
		case kUpArrow: // y velocity goes up
			std::cout << "Up arrow!" << std::endl;
			v.xDelta = 0;
			v.yDelta = -1;
			r->ApplyAction(s, v);
			break;
		case kDownArrow:
			std::cout << "Down arrow!" << std::endl;
			v.xDelta = 0;
			v.yDelta = 1;
			r->ApplyAction(s, v);
			break;
		case kLeftArrow:
			v.xDelta = -1;
			v.yDelta = 0;
			r->ApplyAction(s, v);
			break;
		case kRightArrow:
			v.xDelta = 1;
			v.yDelta = 0;
			r->ApplyAction(s, v);
			break;
		
		// --- Support for WASD --- //
		case 'w':
			std::cout << "The W key!" << std::endl;
			v.xDelta = 0;
			v.yDelta = -1;
			r->ApplyAction(s, v);
			break;
		case 'a':
			std::cout << "The A key!" << std::endl;
			v.xDelta = -1;
			v.yDelta = 0;
			r->ApplyAction(s, v);
			break;
		case 's':
			std::cout << "The S key!" << std::endl;
			v.xDelta = 0;
			v.yDelta = 1;
			r->ApplyAction(s, v);
			break;
		case 'd':
			std::cout << "The D key!" << std::endl;
			v.xDelta = 1;
			v.yDelta = 0;
			r->ApplyAction(s, v);
			break;

		case 'm':
			std::cout << "The M Key! \n";
			std::cout << move << " \n";
			if (move == true)
			{
				move = false;
			}
			else
			{
				move = true;
			}
			
			break;

		default:
			break;
		
	}
	
}


/*
 * Code runs when user clicks or moves mouse in window
 *
 * Application does not currently need mouse support
 */
bool MyClickHandler(unsigned long , int windowX, int windowY, point3d loc, tButtonType button, tMouseEventType mType)
{
	switch (mType)
	{
		case kMouseMove:
		{
		}
			break;
		case kMouseDown:
		{
		}
			break;
		default:
			break;
	}
	return true;
}

