//
//  LeapBone.h
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#ifndef __ofxLeapMotion__LeapBone__
#define __ofxLeapMotion__LeapBone__

#include <iostream>
#include "ofMain.h"
#include "ofxLeapMotion.h"

class LeapBone {
public:
	
	Bone::Type			boneType;			// Bone::Type (metacarpal etc.)
	ofVec3f				boneStartPt;		// Bone start point (Note: Only reported for first bone)
	ofVec3f				boneEndPt;			// Bone end point
	
	void				clear();
};



#endif /* defined(__ofxLeapMotion__LeapBone__) */
