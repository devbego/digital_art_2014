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
#include "ofxBlur.h"

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
    void drawFrameFromXML	(int whichFrame);
	void drawFrameFromXML	(int whichFrame, ofxXmlSettings & myXML);
	
	void drawHandFromXML	(int whichHand, ofxXmlSettings & XML);
    void drawFingersFromXML	(ofxXmlSettings & XML);
    void drawFingerFromXML	(ofxXmlSettings & XML);
    void drawPalmFromXML	(ofxXmlSettings & XML);
    void drawArmFromXML		(ofxXmlSettings & XML);
  
    // live drawing
    void drawFrame			(ofxLeapMotion & leap);
    void drawHand			(Hand & hand, ofxLeapMotion & leap);
    void drawFingers		(Hand & hand, ofxLeapMotion & leap);
    void drawFinger			(const Finger & finger, ofxLeapMotion & leap);
    void drawBone			(const Finger & finger, Bone & bone, ofxLeapMotion & leap);
    void drawPalm			(Hand & hand, ofxLeapMotion & leap);
    void drawArm			(Hand & hand, ofxLeapMotion & leap);
    
    ofxXmlSettings myXML;
	bool bDrawSimple;
    bool bDrawGrid;
	
	bool	bDrawDiagnosticColors;
	ofVec3f getColorDiagnostically (Finger::Type fingerType, Bone::Type boneType, ofPoint bonePt0, ofPoint bonePt1);
	float	getDiagnosticOrientationFromColor (float r255, float g255, float b255);
	int		getDiagnosticIdentityFromColor (float r255, float g255, float b255);
	
	void setProjector(const ofxRay::Projector &P);
	ofxRay::Projector screenProjector;
	bool bProjectorSet;
	float diagnosticFingerScaling;
	float armWidthScaling;
	
	//-------------------------
	// Centroid and orientation of the current hand, stashed temporarily in global variables.
	void captureHandPropertiesFromLeap ( ofxLeapMotion &leap, int whichHandId);
	void captureHandPropertiesFromXML  ( ofxXmlSettings &XML, int whichFrame, int whichHandId);
	void drawCapturedHandProperties(); 
	ofPoint handCentroid;
	ofPoint handNormal;
	
	ofPoint wristPosition;
	ofPoint wristOrientationX;
	
	ofVec3f handCentroidVec3f;
	ofVec3f handNormalVec3f;
	ofPoint handOrientationX;
	ofPoint handOrientationY;
	ofPoint handOrientationZ;
	int		nLeapHandsInScene;
	
	void	updateHandPointVectors();
	float	getMotionAmountFromHandPointVectors();
	float	getCurlFromHandPointVectors();
	ofVec2f getZExtentFromHandPointVectors();
	
	ofVec3f getProjectedHandPoint (int which);
	ofVec3f getProjectedKnuckle (int which);
	ofVec3f getProjectedHandCentroid ();
	ofVec3f getProjectedWristPosition ();
	ofVec3f getProjectedWristOrientation ();
	ofVec3f	getProjectedWristOrientation2 ();
	
	vector<ofPoint> prevHandPoints;
	vector<ofPoint> currHandPoints;
	vector<ofPoint> currKnuckles;
	float fingerThicknesses[5];
	
	
	//-------------------------
	// For voronoi rendering.
	void				enableVoronoiRendering (int imgW, int imgH, bool bHalved);
	void				initPointsToVoronoi();
	void				feedBogusPointsToVoronoi();
	void				feedXMLFingerPointsToVoronoi (int whichFinger, ofxXmlSettings & XML);
	void				updateVoronoiExpansion();
	void				drawVoronoi();
	void				drawVoronoiFrameFromXML (int whichFrame, ofxXmlSettings & XML);
	void				drawVoronoiFrame (ofxLeapMotion & leap);
	ofFbo				voronoiFbo;
	
	bool				bEnableVoronoiRendering;
	bool				bActuallyDisplayVoronoiFbo;
	bool				bUseVoronoiFbo;
	bool				bWorkAtHalfScale;
	nbody				*NB;

	int 				nbody_w;
	int 				nbody_h;
	

	bool				bDoVoronoiShaderBlur;
    ofxBlur				blurShader;

};









