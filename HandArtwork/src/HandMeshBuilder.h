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
	
	bool	buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, ofVec3f &theLeapWristPoint, Handmark *hmarks);
	int		getMeshVertexIndexOfControlPoint (int which);

	bool	isHandMeshFaulty();
	bool	areContourAndHandmarksFaulty(int nContourPts);
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
	int		nTolerableTriangleIntersections;
	
	// The MeshBuilder's local copies
	Handmark myHandmarks[N_HANDMARKS];
	ofPolyline myContour;
	int thumbsidePalmVertexIndices[6];
    ofVec3f theLeapCentroid;
    ofVec3f theLeapWrist;
	
	// Utils
	bool doesContourNeedToBeReversed (ofPolyline &handContour, Handmark *hmarks);
	void createLocalCopyOfContourAndHandmarks (ofPolyline &handContour, Handmark *hmarks, bool bReverse);
	int  getIndexOfClosestPointOnContour(ofPolyline& aPolyline, float qx, float qy);
	
	int imgW;
	int imgH;
    
    
private:
    // Returns true iff the mesh contains two index defined traingles that intersect.
    bool check_triangle_intersections(ofMesh &mesh);
	int	 count_triangle_intersections (ofMesh &mesh);
	bool checkForTrianglesThatAreTooTeeny (ofMesh &mesh); 
    
    int Intersecting(ofVec3f &p0, ofVec3f &p1, ofVec3f &t0, ofVec3f &t1, ofVec3f &t2);
    
    
    bool point_triangle_intersection(ofVec3f &t1, ofVec3f &t2, ofVec3f &t3, ofVec3f &point);
    
    void BarycentricCoords(const ofVec3f & vTriVtx1,
                           const ofVec3f & vTriVtx2,
                           const ofVec3f & vTriVtx3,
                           const ofVec3f & vVertex,
                           float & fBary1, float & fBary2, float & fBary3 );
    
    float Side(ofVec3f &p, ofVec3f &q, ofVec3f &a, ofVec3f b);
	
};
