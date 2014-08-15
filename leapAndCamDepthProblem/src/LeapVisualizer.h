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
    void drawFrameFromXML(int whichFrame, ofxXmlSettings & myXML);

    void drawHandFromXML(int whichHand, ofxXmlSettings & XML);
    void drawFingersFromXML(ofxXmlSettings & XML);
    void drawFingerFromXML(ofxXmlSettings & XML);
    void drawPalmFromXML(ofxXmlSettings & XML);
    void drawArmFromXML(ofxXmlSettings & XML);
  
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
    

};