//
//  LeapFinger.h
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#ifndef __ofxLeapMotion__LeapFinger__
#define __ofxLeapMotion__LeapFinger__

#include <iostream>
#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "LeapBone.h"


class LeapFinger {
public:
	
	Finger::Type		fingerType;
	float				fingerWidth;
	ofVec3f				fingerTipPt;
	
	void				clear();
	vector<LeapBone>	bones;
};


#endif /* defined(__ofxLeapMotion__LeapFinger__) */
