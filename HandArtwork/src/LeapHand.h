//
//  LeapHand.h
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#ifndef __ofxLeapMotion__LeapHand__
#define __ofxLeapMotion__LeapHand__

#include <iostream>
#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "LeapFinger.h"

class LeapHand {
public:
	
	int					index;				// Index in the hand array
	int					id;					// Temporally-coherent ID number
	int					handType;			// LEFT or RIGHT
	float				handConfidence;		// Confidence, 0...1
	float				armWidth;			// Arm width (millimeters)
	
	void				clear();
	
	vector<LeapFinger>	fingers;
	
	ofVec3f				palmPt;				// Palm point
	ofVec3f				wristPt;			// Wrist point
	ofVec3f				elbowPt;			// Elbow point
	ofVec3f				palmNormPt;			// Palm Normal
	
	ofVec3f				handBasisX;			// Hand basis matrix, X component
	ofVec3f				handBasisY;			// Hand basis matrix, Y component
	ofVec3f				handBasisZ;			// Hand basis matrix, Z component
	
	ofVec3f				armBasisX;			// Arm basis matrix, X component
	ofVec3f				armBasisY;			// Arm basis matrix, Y component
	ofVec3f				armBasisZ;			// Arm basis matrix, Z component
};


#endif /* defined(__ofxLeapMotion__LeapHand__) */
