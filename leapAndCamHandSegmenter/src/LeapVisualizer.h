//
//  LeapVisualizer.h
//  leapAndCamRecorder2
//
//  Created by chris on 28/07/14.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"

// For screenspace projections
#include "ofxRay.h"

// For Voronoi rendering
#include "nbody.h"

enum {
	ID_NONE  = 0,
	ID_WRIST = 1,
	ID_PALM  = 2,
	ID_THUMB = 3,
	ID_INDEX = 4,
	ID_MIDDLE = 5,
	ID_RING  = 6,
	ID_PINKY = 7
}  HAND_PART_ID;

class LeapVisualizer{
    
    public:
    
    void setup();
    void loadXmlFile(string fileName);
    
    ofPoint getIndexFingertip(ofxLeapMotion & leap);
    ofPoint getIndexFingertipFromXML(int whichFrame);

    void drawOrientedCylinder (ofPoint pt0, ofPoint pt1, float radius);
	void setColorByFinger (Finger::Type fingerType, Bone::Type boneType);
	void drawGrid();
    
    // xml drawing
    void drawFrameFromXML(int whichFrame);
    void drawHandFromXML(int whichHand);
    void drawFingersFromXML();
    void drawFingerFromXML();
    void drawPalmFromXML();
    void drawArmFromXML();
  
    // live drawing
    void drawFrame(ofxLeapMotion & leap);
    void drawHand(Hand & hand,ofxLeapMotion & leap);
    void drawFingers(Hand & hand,ofxLeapMotion & leap);
    void drawFinger(const Finger & finger,ofxLeapMotion & leap);
    void drawBone(const Finger & finger, Bone & bone,ofxLeapMotion & leap);
    void drawPalm(Hand & hand,ofxLeapMotion & leap);
    void drawArm(Hand & hand,ofxLeapMotion & leap);
    
    ofxXmlSettings XML;
	bool bDrawSimple;
    bool bDrawGrid;
	
	bool bDrawDiagnosticColors;
	ofVec3f getColorDiagnostically (Finger::Type fingerType, Bone::Type boneType, ofPoint bonePt0, ofPoint bonePt1);
	void setProjector(ofxRay::Projector P);
	ofxRay::Projector screenProjector;
	bool bProjectorSet;
	float diagnosticFingerScaling;
	
	
	
	//-------------------------
	// For voronoi rendering.
	void				enableVoronoiRendering (int imgW, int imgH, bool bHalved);
	void				initPointsToVoronoi();
	void				feedBogusPointsToVoronoi();
	void				feedXMLFingerPointsToVoronoi (int whichFinger);
	void				updateVoronoi();
	void				drawVoronoi();
	void				drawVoronoiFrameFromXML (int whichFrame);
	void				drawVoronoiFrame (ofxLeapMotion & leap);
	ofFbo				voronoiFbo;
	
	bool				bEnableVoronoiRendering;
	bool				bActuallyDisplayVoronoiFbo;
	bool				bUseVoronoiFbo;
	bool				bWorkAtHalfScale;
	nbody				*NB;

	int 				nbody_w;
	int 				nbody_h;
    

};









