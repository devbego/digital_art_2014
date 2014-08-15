#pragma once

#include "ofMain.h"
#include "ofxLeapMotion.h"
#include "ofxXmlSettings.h"
#include "ofxCv.h"
#include "ofxCvMin.h"
#include "LeapFrame.h"
#include "LeapRecorder.h"
#include "LeapVisualizer.h"
#include "LeapToCameraCalibrator.h"


#define _USE_CORRECTED_CAMERA


class testApp : public ofBaseApp{

  public:
    void setup();
    void update();
    void draw();
	
	ofImage handImage; 
	
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
    
    int cameraWidth,cameraHeight;
	float drawW, drawH;
	
	ofImage currentFrameImg; // where we store the current frame we grabbed from the Camera
    ofImage processFrameImg; // where we store the processed frame for calibration/display

    
    //------------------------------
    // Leap
    LeapVisualizer			leapVisualizer;
    LeapRecorder			leapRecorder;
    LeapToCameraCalibrator	leapCameraCalibrator;
    
	ofEasyCam cam;
    ofFbo fbo;

    bool bUseVirtualProjector;
    bool bUseFbo;
    bool bShowCalibPoints;
    bool bUseCorrectedCamera;
	bool bEnableDepthTest; 
    
    
    //------------------------------
    // get camera calibration pre-calculated
    ofxCv::Calibration myCalibration;

    void drawLeapWorld();
    
};
