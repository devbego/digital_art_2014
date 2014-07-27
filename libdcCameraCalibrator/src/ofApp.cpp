#include "ofApp.h"

using namespace ofxCv;
using namespace cv;

const int countAfterWhichToStartCullingOutliers = 10; // start cleaning outliers after this many samples

//--------------------------------------------------------------
void ofApp::setup(){
	
	ofSetVerticalSync(true);
    woodblock.loadSound("woodblock.wav");
    woodblock.setVolume(0.75f);
	
    cameraW = 640;
    cameraH = 480;
    int patternXCount = 7;
    int patternYCount = 10;
    
	
    bActivelyTakingSnapshots = false;
    minTimeBetweenSnapshots = 1;
    maxSamples = 30;
    lastTime = 0;
    
    
    //---------------------------------------------
    // Load the settings, which describe both the camera and the calibration pattern.
	FileStorage settings (ofToDataPath("settingsForCameraCalibrator.yml"), FileStorage::READ);
	if(settings.isOpened()) {
        
        cameraW = settings["cameraW"];
        cameraH = settings["cameraH"];
        minTimeBetweenSnapshots = (float) settings["minTimeBetweenSnapshots"];
        motionThreshold = (float) settings["motionThreshold"];
        maxSamples = settings["maxSamples"];
		
        patternXCount = settings["patternXCount"];
        patternYCount = settings["patternYCount"];
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
		camera.setImageType(OF_IMAGE_GRAYSCALE);
        
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
        
        currFrame.allocate(cw,ch, camera.getImageType());
        imitate(undistortedFrame, currFrame);
        imitate(prevFrame,   currFrame);
        imitate(diffFrame,   currFrame);
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
        
        //----------------------
        // Compute frame difference, an estimate of camera stability:
        Mat currMat = toCv(currFrame);
		Mat prevMat = toCv(prevFrame);
		Mat diffMat = toCv(diffFrame);
		absdiff (prevMat, currMat, diffMat);
		currMat.copyTo (prevMat);
		diffMean = mean(Mat(mean(diffMat)))[0];
        
        //----------------------
        // Update the calibration
        float currTime = ofGetElapsedTimef();
        if (bActivelyTakingSnapshots){
			printf ("Elapsed: %f after %f \n", (currTime - lastTime),minTimeBetweenSnapshots);
			
            if ((currTime - lastTime > minTimeBetweenSnapshots) &&
                (diffMean < motionThreshold)) {
                // woodblock.play();
                
                if (myCalibration.add(currMat)) {
					woodblock.play();
                    myCalibration.calibrate();
                    
                    // if we're already at our maximum number of samples,
                    // peck off the sample with the worst reprojection error
                    if (myCalibration.size() > maxSamples){
                        float maxProjError = 0;
                        for (int i=0; i<myCalibration.size(); i++) {
                            if ( myCalibration.getReprojectionError(i) > maxProjError){
                                maxProjError = myCalibration.getReprojectionError(i);
                            }
                        }
                        myCalibration.clean( maxProjError - 0.00001 );
                    } else if (myCalibration.size() > countAfterWhichToStartCullingOutliers) {
                        myCalibration.clean();
                    }
                    
                    myCalibration.save("calibration.yml");
                    lastTime = currTime;
                }
            }
        }
		
		if (myCalibration.size() > 0) {
			myCalibration.undistort(toCv(currFrame), toCv(undistortedFrame));
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
            ofPushMatrix();
            
            //------------------
            // The camera doesn't draw itself; currFrame does.
            int cw = (int) camera.getWidth();
            int ch = (int) camera.getHeight();
            float drawW = ofGetWidth()/2.0;
            float drawH = drawW * (float)ch/(float)cw;
            ofFill();
            ofSetColor(255,255,255);
            currFrame.draw(0,0, drawW,drawH);
            undistortedFrame.draw(drawW,0, drawW,drawH);
            ofTranslate(0, drawH);
			
            
            //------------------
            // Collect the camera's capture dimensions (which may not be the same as the display dimensions)
            // and also collect other properties
            dc1394camera_t* my1394cam = camera.getLibdcCamera();
            char *camVendor = my1394cam->vendor;
            char *camModel  = my1394cam->model;
            
            //------------------
            // Display the diagnostic information about the camera
            float textX = 10;
            if (diffMean > motionThreshold){
                ofSetColor(255,102,0);
            } else {
                ofSetColor(0,255,0);
            }
            string displayString = "";
            displayString += "See 'bin/data/settingsForCameraCalibrator.yml' for params.\n";
            displayString += "Camera: " + ofToString(camVendor) + " " + ofToString(camModel) + "\n";
            displayString += "Libdc1394 (Firewire) capture at " + ofToString(cw) +  "x" + ofToString(ch) + "\n";
            displayString += "Press 'x' to clear calibration.\n";
            if (!bActivelyTakingSnapshots){
                displayString += "Press 'space' to toggle automatic snapshot sequence.\n";
            } else {
                displayString += "Taking snapshots every " + ofToString((int)minTimeBetweenSnapshots) + " second";
                displayString += (((int)minTimeBetweenSnapshots) > 1) ? "s " : " ";
                displayString += "(if Movement < " + ofToString(motionThreshold) + ")";
                displayString += "\n";
            }
            displayString += "Movement (from frame differencing): " + ofToString(diffMean) + "\n";
            ofDrawBitmapString (displayString, textX, 20);
			
            //------------------
            // Display the calibration constituents
            ofSetColor(255,255,0);
            for (int i = 0; i < myCalibration.size(); i++) {
                ofDrawBitmapString (ofToString(i) + ": " +
                                    ofToString(myCalibration.getReprojectionError(i)),
                                    textX + (i/10)*128, 120 + 12 * (i%10));
            }
            
            //------------------
            // Draw text under undistorted image.
            ofTranslate(drawW, 0);
            string displayString2 = "";
            displayString2 += "Undistorted (corrected) image";
            if (bSavedCalibrationExists){
                ofSetColor(255,255,0);
                displayString2 += ", from 'calibration.yml' file.\n";
                displayString2 += "Pressing 'space' will start a new calibration.\n";
                displayString2 += "(Previous work will be saved as 'calibration-old.yml')\n";
            } else {
                ofSetColor(0,255,0);
                displayString2 += ".\n";
            }
            
            displayString2 += "\n";
            displayString2 += "Calibration from " + ofToString(myCalibration.size()) + " samples:\n";
            displayString2 += "Reprojection error: " + ofToString(myCalibration.getReprojectionError()) + "\n";
            displayString2 += "FOV: " + ofToString(toOf(myCalibration.getDistortedIntrinsics().getFov())) + "\n";
            
            Mat distCoeffs = myCalibration.getDistCoeffs();
            int nCoeffs = distCoeffs.total();
            for (int i=0; i<nCoeffs; i++){
                double ithCoeff = distCoeffs.at<double>(i,0);
                displayString2 += "Distortion Coeff. " + ofToString(i) + ": " + ofToString(ithCoeff) + "\n";
            }
			
            ofDrawBitmapString (displayString2, textX, 20);
            ofPopMatrix();
            
			
			
			//------------------
			// draw a clock indicator if bActivelyTakingSnapshots
			if (bActivelyTakingSnapshots){
				float currTime = ofGetElapsedTimef();
				float elapsedFrac = (currTime - lastTime)/minTimeBetweenSnapshots;
				float angle = elapsedFrac * TWO_PI - HALF_PI;
				float alp = 127 + 127 * powf(1.0-elapsedFrac, 0.5);
				
				float rad = 20;
				float ex = drawW - rad*1.5;
				float ey =         rad*1.5;
				
				ofNoFill();
				ofEnableAlphaBlending();
				ofSetColor(0,255,0, alp);
				ofEllipse(ex,ey, rad*2,rad*2);
				float cx = ex + rad*cosf(angle);
				float cy = ey + rad*sinf(angle);
				ofLine(ex,ey, cx,cy);
				ofDisableAlphaBlending();
			}
            
		}
        
    } else {
        ofBackground(0,0,0);
        ofSetColor(255,0,0);
        ofDrawBitmapString("NO CAMERAS CONNECTED!", 10, 20);
    }
	


}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	if (key == ' ') {
        if (bSavedCalibrationExists){
            myCalibration.save("calibration-old.yml");
            myCalibration.clean(0.0);
            bSavedCalibrationExists = false;
        }
		bActivelyTakingSnapshots = !bActivelyTakingSnapshots;
	} else if (key == 'x') {
        myCalibration.clean(0.0);
    } else if (key == 'f') {
		ofToggleFullscreen();
	}

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
