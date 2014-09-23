//
//  HandMeshBuilder.h
//  leapAndCamHandSegmenter
//
//  Created by GL on 9/22/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "HandContourAnalyzer.h"


enum HandType {
	HAND_ERROR = -1,
	HAND_NONE  =  0,
	HAND_RIGHT =  1,
	HAND_LEFT  =  2
};


class HandMeshBuilder {
public:
	
	void			initialize();
	void			setWorkAtHalfScale(bool bwork);
	void			buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks);
	void			loadDefaultMesh();
	void			drawMesh();
	void			drawMeshWireframe();
	void			informThereIsNoHandPresent();
	ofMesh			&getMesh();
	
	ofMesh			handMesh;
	vector<int>		joints;
	Handmark		Handmarks[N_HANDMARKS];
	int				fingerTipIndices[5];
	
	bool			bCalculatedMesh;
	bool			bWorkAtHalfScale;
	
	void			drawJoints();
	
	
	HandType		getHandType();
	HandType		currentHandType;
	int				currentHandExistsFrameCount;
	
};
