#include "ofApp.h"

using namespace ofxCv;
using namespace cv;


//--------------------------------------------------------------
void ofApp::setup(){
	
	cameraW = 640;
    cameraH = 480;
	
	
	//---------------------------------------------
    // Load the settings, which describe both the camera and the calibration pattern.
	FileStorage settings (ofToDataPath("settingsForCameraCalibrator.yml"), FileStorage::READ);
	if(settings.isOpened()) {
        
        cameraW = settings["cameraW"];
        cameraH = settings["cameraH"];
		
        int patternXCount = settings["patternXCount"];
        int patternYCount = settings["patternYCount"];
        myCalibration.setPatternSize(patternXCount, patternYCount);
        float squareSize = settings["squareSize"];
        myCalibration.setSquareSize(squareSize);
		
        CalibrationPattern patternType = CHESSBOARD;
        switch(settings["patternType"]) {
            default:
            case 0: patternType = CHESSBOARD; break;
            case 1: patternType = CIRCLES_GRID; break;
            case 2: patternType = ASYMMETRIC_CIRCLES_GRID; break;
        }
        myCalibration.setPatternType(patternType);
		
	}
    
    //---------------------------------------------
    bSavedCalibrationExists = false;
    FileStorage prevCalibrationFile (ofToDataPath("calibration.yml"), FileStorage::READ);
    if (prevCalibrationFile.isOpened()){
        prevCalibrationFile.release();
        myCalibration.load("calibration.yml");
        myCalibration.calibrate();
        if (myCalibration.isReady()){
            bSavedCalibrationExists = true;
        }
    }
	

    //---------------------------------------------
    // Configure the camera, per the loaded settings.
    bCameraConnected = false;
    int nCameras = camera.getCameraCount();
    printf ("Number of attached dc1394 cameras: %d\n", nCameras);
    if (nCameras >= 1){
        bCameraConnected = true;
        int cw = (int) camera.getWidth();
        int ch = (int) camera.getHeight();
        printf ("This camera defaults to dimensions of (%d %d).\n", cw, ch);
		
		ofSetVerticalSync(false);
		ofSetLogLevel(OF_LOG_VERBOSE);
		
		camera.setImageType(OF_IMAGE_COLOR); //OF_IMAGE_GRAYSCALE
		
	
		// camera.setFormat7(false);
		camera.setSize(cameraW, cameraH);
		camera.setImageType(OF_IMAGE_COLOR);
		camera.setBayerMode(DC1394_COLOR_FILTER_GRBG);
		
		camera.setBlocking(true);
		camera.setFrameRate(30);
		
		camera.setExposure(1.0);
		camera.setup();
		
		// After setup it's still possible to change a lot of parameters. If you want
		// to change a pre-setup parameter, the camera will auto-restart
		camera.setBrightness(0);
		camera.setGain(0);
		camera.setExposure(1.0);
		camera.setGammaAbs(1);
		camera.setShutterAbs(1.0 / 40.0); // DON'T DO 1/30 or you choke with 7.5 fps!!!
		
		
		
		
		
        /*
        if ((cw != cameraW) || (ch != cameraH)) {
            printf ("Requesting camera dimensions of (%d %d)...\n", cameraW, cameraH);
            camera.setSize (cameraW, cameraH);
            
            cw = (int) camera.getWidth();
            ch = (int) camera.getHeight();
            printf ("Camera now has dimensions of (%d %d).\n", cw, ch);
        }
		
        
        // If you want to set any non-default parameters like size, format7, blocking
        // capture, etc., you can do it here before setup. They'll be applied to the
        // camera during setup(). setup() will pick the first camera.
        camera.setup();
		 */
        
        currFrame.allocate(cameraW, cameraH, camera.getImageType());
        imitate(undistortedFrame, currFrame);
    }

}

//--------------------------------------------------------------
void ofApp::update(){

	// grabVideo() will place the most recent frame into currFrame. If it's a new
	// frame, grabFrame returns true. If there are multiple frames available, it
	// will drop old frames and only give you the newest. This guarantees the
	// lowest latency. If you prefer to not drop frames, set the second argument
	// (dropFrames) to false. By default, capture is non-blocking.
	if (camera.grabVideo(currFrame)) {
		currFrame.update();
        
		
		if (myCalibration.size() > 0) {
			int then = ofGetElapsedTimeMicros();
			myCalibration.undistort(toCv(currFrame), toCv(undistortedFrame), INTER_LINEAR); // INTER_NEAREST, INTER_CUBIC
			int now = ofGetElapsedTimeMicros();
			printf("Elapsed = %d\n", (now-then));
			undistortedFrame.update();
		}
	}

}

//--------------------------------------------------------------
void ofApp::draw(){
	if (bCameraConnected){
		
        
		
        // If the camera isn't ready, the curFrame will be empty.
        if (camera.isReady()) {
            ofBackground(51,51,51);
            
            //------------------
            // The camera doesn't draw itself; currFrame does.

            ofFill();
            ofSetColor(255,255,255);
			
            //currFrame.draw(0,0, cameraW,cameraH);
            undistortedFrame.draw(0,0, cameraW,cameraH);
			
			
			ofDrawBitmapString(ofToString(ofGetFrameRate()) + " fps ", 10, 20);
  
		} else {
			ofBackground(255,51,51);
		}
		 
        
    } else {
        ofBackground(0,0,0);
        ofSetColor(255,0,0);
        ofDrawBitmapString("NO CAMERAS CONNECTED!", 10, 20);
    }
	
	

}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
