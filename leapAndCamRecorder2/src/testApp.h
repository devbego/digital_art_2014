#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"
#include "ofxCv.h"
#include "ofxLibdc.h"
#include "LeapFrame.h"
#include "BufferedVideo.h"
#include "LeapRecorder.h"
#include "LeapVisualizer.h"
#include "FingerTipVideoRecorder.h"

/* 
 Made some sanity changes in ofxXmlSettings:
	const float floatPrecision = 4; // changed by GL
	fprintf( cfile, "\t" ); // instead of 4 spaces, GOLAN fprintf( cfile, "    " );
 */


#define _USE_LIBDC_GRABBER
// uncomment this to use a libdc firewire camera standard of an OF video grabber


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
	
    //------------------------------
    // Gui
    void drawText();
    
    //------------------------------
	// For recording the CAMERA.
	ofxLibdc::PointGrey cameraLibdc;
	ofVideoGrabber 		cameraVidGrabber;
    float               cameraWidth,cameraHeight;
	
	ofImage currentFrameImg; // where we store the current frame we grabbed from the Camera
	int currentFrameNumber;
	vector<ofPixels> imageSequence;
	
    
    //------------------------------
    // Leap
	ofxLeapMotion leap;
    LeapVisualizer leapVisualizer;
    LeapRecorder leapRecorder;
    
	ofEasyCam cam;
    ofFbo fbo;

    bool bPlaying;
    bool bRecording;
	int  playingFrame;
    string folderName;
	
    //------------------------------
    // Video buffer playback
    BufferedVideo video;
    bool playing,active;
    bool bCompletedOneVideoLoop;
	bool bEndRecording;
    
	//------------------------------
    // Calibration recording
    FingerTipVideoRecorder indexRecorder;
    ofPoint lastIndexVideoPos;
    ofPoint lastIndexLeapPos;
    
    
    void finishRecording();
    void loadAndPlayRecording(string folderName);
    
};
