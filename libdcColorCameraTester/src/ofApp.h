#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxLibdc.h"

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
	
	
    // ofxLibdc is a namespace, and Camera is the object
	ofxLibdc::PointGrey camera;
	
	// ofImage is where we store the current frame we grabbed from the Camera
	ofImage currFrame;
    
    bool bCameraConnected;
    int  cameraW;
    int  cameraH;
    
    bool bSavedCalibrationExists;
    ofxCv::Calibration myCalibration;
    ofImage  undistortedFrame;
	ofPixels prevFrame;
	ofPixels diffFrame;
	float    diffMean;
	
	float    lastTime;
	bool     bActivelyTakingSnapshots;
    bool     bAutomaticSnapshots;
    float    minTimeBetweenSnapshots;
    float    motionThreshold;
    int      maxSamples;

		
};
