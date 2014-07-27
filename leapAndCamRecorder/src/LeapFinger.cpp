//
//  LeapFinger.cpp
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#include "LeapFinger.h"

void LeapFinger::clear(){
	
	fingerType = (Finger::Type)0;
	fingerWidth = 0;
	fingerTipPt.zero();
	
	if (bones.size() > 0){
		for (int i=0; i<bones.size(); i++){
			bones[i].clear();
		}
	}
	bones.clear();
	
}