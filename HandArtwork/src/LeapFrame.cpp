//
//  LeapFrame.cpp
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#include "LeapFrame.h"


void LeapFrame::clear(){
	

	frameCount			= 0;
	recordingTime		= 0;
	nHands				= 0;
	
	for (int i=0; i<hands.size(); i++){
		hands[i].clear();
	}
	hands.clear();
}

