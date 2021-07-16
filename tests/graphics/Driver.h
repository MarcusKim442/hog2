/*
 *  Driver.h
 *  hog2
 *
 *  Created by Nathan Sturtevant on 5/31/05.
 *  Modified by Nathan Sturtevant on 07/15/21.
 *
 * This file is part of HOG2. See https://github.com/nathansttt/hog2 for licensing information.
 *
 */

void MyWindowHandler(unsigned long windowID, tWindowEventType eType);
void MyFrameHandler(unsigned long windowID, unsigned int viewport, void *data);
void MyDisplayHandler(unsigned long windowID, tKeyboardModifier, char key);
int MyCLHandler(char *argument[], int maxNumArgs);
int MyCLHandler2(char *argument[], int maxNumArgs);
int MyCLHandler3(char *argument[], int maxNumArgs);
bool MyClickHandler(unsigned long windowID, int x, int y, point3d loc, tButtonType, tMouseEventType);
void InstallHandlers();
