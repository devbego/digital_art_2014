#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"
#include "LeapFrame.h"


/* 
 Made some sanity changes in ofxXmlSettings:
	const float floatPrecision = 4; // changed by GL
	fprintf( cfile, "\t" ); // instead of 4 spaces, GOLAN fprintf( cfile, "    " );
 */


class testApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();
	
    void keyPressed  (int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y );
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    void exit();
		
	ofxLeapMotion leap;
	vector <ofxLeapMotionSimpleHand> simpleHands;
    
	vector <int> fingersFound; 
	ofEasyCam cam;
	
	bool bDrawGrid; 
	bool bDrawSimple; 
	void drawOrientedCylinder (ofPoint pt0, ofPoint pt1, float radius);
	void setColorByFinger (Finger::Type fingerType, Bone::Type boneType);
	
	void drawText();
	void drawGrid();
	void drawFrame ();
	void drawHand (Hand & hand);
	void drawFingers (Hand & hand);
	void drawFinger (const Finger & finger);
	void drawBone (const Finger & finger, Bone & bone);
	void drawPalm (Hand & hand);
	void drawArm (Hand & hand);
	
	void recordFrameXML ();
	void recordHandXML (Hand & hand, int handIndex);
	void recordFingerXML (const Finger & finger);
	void recordBoneXML (const Finger & finger, Bone & bone);
	void recordArmXML (Hand & hand);

	bool bPlaying;
	int  playingFrame;
	void drawFrameFromXML (int whichFrame);
	void drawHandFromXML (int whichHand);
	void drawFingersFromXML();
	void drawFingerFromXML();
	void drawPalmFromXML();
	void drawArmFromXML();
	
	bool bRecording;
	bool bRecordingThisFrame; 
	ofxXmlSettings XML;
	int lastTagNumber;
	int recordingFrameCount;
	int recordingStartTimeMillis;
	
	LeapFrame	aLeapFrame;
	void		fillLeapFrameFromCurrentData();

};
