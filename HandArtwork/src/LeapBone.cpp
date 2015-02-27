//
//  LeapBone.cpp
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#include "LeapBone.h"


void LeapBone::clear(){
	
	boneType = (Bone::Type)0;
	boneStartPt.zero();
	boneEndPt.zero();
	
}