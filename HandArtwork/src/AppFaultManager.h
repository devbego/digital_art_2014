//
//  appFaultManager.h
//  HandArtwork
//
//  Created by chris on 1/10/14.
//
//

#pragma once
#include "ofMain.h"

enum ApplicationFault {
	FAULT_NOTHING_WRONG				= 0,	/* Everything appears to be working normally */
	FAULT_NO_USER_PRESENT_BRIEF		= 1,	/* Hand may have been momentarily withdrawn, not present for ~1 second */
	FAULT_NO_USER_PRESENT_LONG		= 2,	/* Nobody is present in the camera nor leap, for more than ~10 seconds */
	FAULT_LEAP_DROPPED_FRAME		= 4,	/* There was a hand in the camera (as recently as a second ago), but there's a leap dropout */
	FAULT_NO_LEAP_HAND_TOO_SMALL	= 8,	/* There's a handlike object in the camera, but it may be too small for the leap to work */
	FAULT_NO_LEAP_OBJECT_PRESENT	= 16,	/* Some bastard put something in the box */
	FAULT_TOO_MANY_HANDS			= 32,	/* There's more than one hand in view */
	FAULT_HAND_TOO_FAST				= 64,	/* The hand is moving too quickly */
	FAULT_HAND_TOO_HIGH				= 128,	/* The hand is physically too close to the cameras */
	FAULT_HAND_TOO_CURLED			= 256,	/* The hand is a fist or curled up, or has a curled finger */
	FAULT_HAND_TOO_VERTICAL			= 512,	/* The hand is turned away from the camera */
    FAULT_HAND_NOT_DEEP_ENOUGH      = 1024,
    FAULT_SAME_SCENE_TOO_LONG       = 2048  /* The same scene has been playing more than ~1 minute */
};


class AppFaultManager{
    
public:
    
    void setup();
    
    // updates total time this fault continuously present
    void updateHasFault( ApplicationFault fault, float dt);
    
    // resets time to 0
    void updateResetFault( ApplicationFault fault);

    // draw fault screens (for some faults, give user feedback visually)
    void drawFaultHelpScreen();
    
    // draws faults as bitmap text with color indicating "strength" of detection of the fault
    void drawDebug(int x, int y);
    
    // get the fault that has been detected the longest amount of time
    ApplicationFault getLongestFault();
    
    // gets the faults with highest priority that has been triggered
    ApplicationFault getMostImportantActiveFault();
    
    // get the current duration of a given fault
    float getDurationOfFault (ApplicationFault fault);
    
    // get all detected faults
    vector<ApplicationFault> getAllFaults();
    
    bool getHasFault(ApplicationFault fault);
	
	bool doCurrentFaultsIndicateLikelihoodOfBadMeshes(); 
    
    map<ApplicationFault, float> timeHasFault; // total seconds fault recorded continuously, if fault not found reset to 0
    map<ApplicationFault, float> timeLimit; // total seconds needed to be considered active fault
    
    map<ApplicationFault, int> priorityOrder; // order by most important (0 most important)

    
    ofTrueTypeFont font;
    float fontAlpha;
    ApplicationFault shownFault;
    string myFaultString, myFaultStringNL;
    bool bShowingFault;
    float timeStartShowFault;
    int showFaultMode;
    int minTimeOn;
    
    
};