//
//  LeapFrame.h
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#ifndef __ofxLeapMotion__LeapFrame__
#define __ofxLeapMotion__LeapFrame__

#include <iostream>
#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "LeapHand.h"
#include "LeapFinger.h"
#include "LeapBone.h"

class LeapFrame	{
	public:
	
	int					frameCount;			// Count of this Frame
	int					recordingTime;		// Recording time of this Frame (milliseconds)
	int					nHands;				// Number of Hands in Frame
 
	vector<LeapHand>	hands;
	void				clear(); 
};


#endif /* defined(__ofxLeapMotion__LeapFrame__) */
