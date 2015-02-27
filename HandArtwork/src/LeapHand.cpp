//
//  LeapHand.cpp
//  ofxLeapMotion
//
//  Created by GL on 6/29/14.
//
//

#include "LeapHand.h"

void LeapHand::clear(){
	
	if (fingers.size() > 0){
		for (int i=0; i<fingers.size(); i++){
			fingers[i].clear();
		}
	}
	fingers.clear();
	
	
	index			= 0;
	id				= 0;
	handType		= 0;
	handConfidence	= 0;
	armWidth		= 0;
	
	palmPt.zero();
	wristPt.zero();
	elbowPt.zero();
	palmNormPt.zero();
	
	handBasisX.zero();
	handBasisY.zero();
	handBasisZ.zero();
	
	armBasisX.zero();
	armBasisY.zero();
	armBasisZ.zero();
}