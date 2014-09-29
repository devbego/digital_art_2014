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
#include "ofxButterfly.h"

enum HandType {
	HAND_ERROR = -1,
	HAND_NONE  =  0,
	HAND_RIGHT =  1,
	HAND_LEFT  =  2
};


class HandMeshBuilder {
public:
	
	void			initialize(int w, int h);
	void			setWorkAtHalfScale(bool bwork);
	
	void			loadDefaultMesh();
	void			drawMesh();
	void			drawMeshWireframe();
    void            drawRefinedMeshWireframe();
	void			informThereIsNoHandPresent();
	ofMesh			&getMesh();
	
	void			buildMesh2013 (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks);
	
	ofMesh			handMesh;
    ofMesh          refinedHandMesh;
    ofxButterfly    butterflyMeshSubdivider;
    
	vector<int>		joints;
	Handmark		Handmarks[N_HANDMARKS];
	int				fingerTipIndices[5];
	
	bool			bWorkAtHalfScale;
	
	void			drawJoints();
	
	
	HandType		getHandType();
	HandType		currentHandType;
	int				currentHandExistsFrameCount;
	
	//-----------------------------
	// 2014 Mesher
	
	void	buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks);
	int		getMeshVertexIndexOfControlPoint (int which);

	bool	isHandMeshProblematic();
	bool 	checkForDuplicatedVertices();
	
	void	addFingersToHandMesh();
	void	addWristToHandMesh();
	void	addPalmToHandMesh();
	void	addThumbWebbingToHandMesh();
	bool	bCalculatedMesh;
	bool	bRenderIntermediate;
	bool	bWindingCCW;
	int		nTrianglesAccum;
	int		verticesPerFinger;
	int		handMeshWristVertexIndex;
	
	// The MeshBuilder's local copies
	Handmark myHandmarks[N_HANDMARKS];
	ofPolyline myContour;
	int thumbsidePalmVertexIndices[6];
	
	// Utils
	bool doesContourNeedToBeReversed (ofPolyline &handContour, Handmark *hmarks);
	void createLocalCopyOfContourAndHandmarks (ofPolyline &handContour, Handmark *hmarks, bool bReverse);
	int  getIndexOfClosestPointOnContour(ofPolyline& aPolyline, float qx, float qy);
	
	int imgW;
	int imgH; 
	
};
