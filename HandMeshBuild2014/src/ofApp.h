#pragma once

#include "ofMain.h"
#include "ofxOpenCv.h"


#define N_HANDMARKS 16
#define DESIRED_N_CONTOUR_POINTS	800 


enum HandmarkType {
	HANDMARK_INVALID			= -1,
	HANDMARK_PINKY_TIP			= 0,
	HANDMARK_PR_CROTCH			= 1,
	HANDMARK_RING_TIP			= 2,
	HANDMARK_RM_CROTCH			= 3,
	HANDMARK_MIDDLE_TIP			= 4,
	HANDMARK_MI_CROTCH			= 5,
	HANDMARK_POINTER_TIP		= 6,
	HANDMARK_POINTER_SIDE		= 7,
	HANDMARK_IT_CROTCH			= 8,
	HANDMARK_THUMB_TIP			= 9,
	HANDMARK_THUMB_KNUCKLE		= 10,
	HANDMARK_THUMB_BASE			= 11,
	HANDMARK_THUMBSIDE_WRIST	= 12,
	HANDMARK_PINKYSIDE_WRIST	= 13,
	HANDMARK_PALM_BASE			= 14,
	HANDMARK_PINKY_SIDE			= 15
};

struct Handmark { // analogous to landmark
	ofVec3f			point;			// the final actual location
	int				index;			// the final index in the theHandContourFinal
	HandmarkType	type;			// the name (or type) of the point
	bool			valid;
};



class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
	
	// Vision, to produce a contour.
	bool bLoadL;
	ofImage handImage;
	ofxCvContourFinder contourFinder;
	ofxCvColorImage	handImageColor; 
	ofxCvGrayscaleImage handImageGray;
	ofxCvGrayscaleImage handImageThresholded;
	ofPolyline	handContourRaw;
	ofPolyline	handContourFinal;
	int getIndexOfClosestPointOnContour(ofPolyline& aPolyline, float qx, float qy);
	
	// Inputs to buildMesh
	Handmark HandmarksInput[N_HANDMARKS];
	ofVec3f handCentroid;
	
	// Mesh building
	ofMesh	handMesh;
	int		getMeshVertexIndexOfControlPoint (int which);
	void	buildMesh (ofPolyline &handContour, ofVec3f &handCentroid, Handmark *hmarks);
	void	checkForDuplicatedVertices();
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

		
};
