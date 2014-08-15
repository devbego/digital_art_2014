#include "testApp.h"

/*
// OPENFRAMEWORKS + LEAP MOTION SDK 2.0 HAND SKELETON DEMO 
// By Golan Levin (@golan), http://github.com/golanlevin
// Uses ofxLeapMotion addon by Theo Watson, with assistance from Dan Wilcox
// Supported in part by the Frank-Ratchye STUDIO for Creative Inquiry at CMU
*/



/* Note on OS X, you must have this in the Run Script Build Phase of your project. 
where the first path ../../../addons/ofxLeapMotion/ is the path to the ofxLeapMotion addon. 

cp -f ../../../addons/ofxLeapMotion/libs/lib/osx/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/libLeap.dylib"; install_name_tool -change ./libLeap.dylib @executable_path/libLeap.dylib "$TARGET_BUILD_DIR/$PRODUCT_NAME.app/Contents/MacOS/$PRODUCT_NAME";

   If you don't have this you'll see an error in the console: dyld: Library not loaded: @loader_path/libLeap.dylib
*/



//--------------------------------------------------------------
void testApp::setup(){
	
	handImage.loadImage("0.jpg");
    
    // string basePath = ofToDataPath("", true);
    // ofSetDataPathRoot("../../../../../SharedData/");
    
    cameraWidth = 1024;
    cameraHeight = 768;
	drawW = 640;
    drawH = 480;
    
	currentFrameImg.clear();
	currentFrameImg.allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	processFrameImg.allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	
    
    //--------------- Setup camera calibration for undistortion
    #ifdef _USE_CORRECTED_CAMERA
    ofxCv::FileStorage settings (ofToDataPath("settingsForCameraCalibrator.yml"), ofxCv::FileStorage::READ);
	if (settings.isOpened()) {
        
        // needed if not calibrating??
        int patternXCount = settings["patternXCount"];
        int patternYCount = settings["patternYCount"];
        myCalibration.setPatternSize(patternXCount, patternYCount);
        float squareSize = settings["squareSize"];
        myCalibration.setSquareSize(squareSize);
		
        ofxCv::CalibrationPattern patternType = ofxCv::CHESSBOARD;
        switch(settings["patternType"]) {
            default:
            case 0: patternType = ofxCv::CHESSBOARD; break;
            case 1: patternType = ofxCv::CIRCLES_GRID; break;
            case 2: patternType = ofxCv::ASYMMETRIC_CIRCLES_GRID; break;
        }
        myCalibration.setPatternType(patternType);
	}
    
    ofxCv::FileStorage prevCalibrationFile (ofToDataPath("calibration.yml"), ofxCv::FileStorage::READ);
    if (prevCalibrationFile.isOpened()){
        prevCalibrationFile.release();
        myCalibration.load("calibration.yml");
        myCalibration.calibrate();
        if (myCalibration.isReady()){
            cout << "calibration ready" << endl;
        }
    }
    #endif
    
    
    //--------------- Setup leap
    leapVisualizer.setup();
    leapRecorder.setup();
	cam.setOrientation(ofPoint(-55, 0, 0));
    fbo.allocate(cameraWidth,cameraHeight,GL_RGBA);

    bUseVirtualProjector = true;
    bUseFbo              = true;
    bShowCalibPoints     = true;
    bUseCorrectedCamera  = true;
	bEnableDepthTest     = true;
	
	//--------------- Leap and Camera Calibration
	leapCameraCalibrator.setup(cameraWidth, cameraHeight);
    leapCameraCalibrator.loadFingerTipPoints("fingerCalibPts.xml");
	leapCameraCalibrator.correctCameraPNP(myCalibration);
	leapVisualizer.loadXmlFile("leap.xml");
    
    //--------------- App settings
    // ofSetFrameRate(60);
    ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_WARNING);
	ofSetCylinderResolution (16, 1, 16);
	
	
}


//--------------------------------------------------------------
void testApp::update(){
    
	/*
    currentFrameImg.setFromPixels(handImage.getPixels(), cameraWidth,cameraHeight, OF_IMAGE_COLOR);
    
	if (myCalibration.size() > 0) {
		myCalibration.undistort(ofxCv::toCv(currentFrameImg), ofxCv::toCv(processFrameImg));
		processFrameImg.update();
	} else {
		processFrameImg = currentFrameImg;
	}
	 */
}


//--------------------------------------------------------------
void testApp::draw(){

    ofBackground(0,0,0);
    
	
	drawLeapWorld();
    
    // if calibrated and want to see view from projector
    if (leapCameraCalibrator.calibrated && bShowCalibPoints && bUseVirtualProjector){
        ofPushMatrix();
        ofScale(drawW/(float)cameraWidth, drawH/(float)cameraHeight);
        leapCameraCalibrator.drawImagePoints();
        ofPopMatrix();
    }
	
	
	ofSetColor(255);
	handImage.draw(drawW, 0,drawW,drawH);
	
	drawText();
}


//--------------------------------------------------------------
void testApp::drawLeapWorld(){
    
    // draw video underneath calibration to see alignment
    glDisable(GL_DEPTH_TEST);
    if (leapCameraCalibrator.calibrated && bUseVirtualProjector){
        ofSetColor(255);
		handImage.draw(0, 0, drawW,drawH);
		// processFrameImg.draw(0,0,drawW,drawH);
    }
    
    // start fbo
    if (bUseFbo){
        fbo.begin();
        ofClear(0,0,0,0);
    }
	
	if (bEnableDepthTest){
		glEnable(GL_DEPTH_TEST);
	} else {
		glDisable(GL_DEPTH_TEST);
	}
	
    
    // start camera (either calibrated projection or easy cam)
    if (leapCameraCalibrator.calibrated && bUseVirtualProjector){
		// ofEnableAlphaBlending();
		// glEnable(GL_NORMALIZE); // needed??
		// glEnable(GL_DEPTH);
        // glEnable(GL_DEPTH_TEST); // why is this messing the render up in the projector cam??????
		
		
        leapCameraCalibrator.projector.beginAsCamera();
		//glDisable(GL_DEPTH);
		
		
    } else {
        //glEnable(GL_DEPTH_TEST);
        cam.begin();
    }
	
	// Draw the LEAP hand
	leapVisualizer.drawFrameFromXML(0);
    
    // draw grid
    ofSetColor(255);
    leapVisualizer.drawGrid();
    
    // draw world points
    if (leapCameraCalibrator.calibrated && bShowCalibPoints){
        leapCameraCalibrator.drawWorldPoints();
    }
    
    // end camera
	if (leapCameraCalibrator.calibrated && bUseVirtualProjector){
        leapCameraCalibrator.projector.endAsCamera();
    } else {
        
        // draw calibration projector for debugging
        if (leapCameraCalibrator.calibrated){
            leapCameraCalibrator.projector.draw();
        }
        cam.end();
        glDisable(GL_DEPTH_TEST);
    }
    
    // end fbo
    if (bUseFbo){
        fbo.end();
    }
    
    
    // if we are calibrated, draw fbo with transparency to see overlay better
    if (leapCameraCalibrator.calibrated) ofSetColor(255,255,255,200);
    else ofSetColor(255,255,255,255);
    
    
    // draw the fbo
    ofPushMatrix();
	if(bUseVirtualProjector){
		ofScale(1,-1,1);    // flip fbo
		fbo.draw(0,-drawH,drawW,drawH);
	}else{
		fbo.draw(0,0,drawW,drawH);
	}
    ofPopMatrix();
    

}

//--------------------------------------------------------------
void testApp::drawText(){
	
	float textY = 500;
	
	ofSetColor(ofColor::orange);
	ofDrawBitmapString("GL_DEPTH_TEST = " + ofToString(bEnableDepthTest),20, textY); textY+=15;
	if (bUseVirtualProjector){
		ofDrawBitmapString("Camera = ofxRay",20, textY); textY+=15;
	} else {
		ofDrawBitmapString("Camera = EasyCam",20, textY); textY+=15;
	}
	textY+=15;
	ofSetColor(ofColor::white);
	ofDrawBitmapString("Press 'v' to toggle ofxRay/easyCam virtual projector", 20, textY); textY+=15;
	ofDrawBitmapString("press 'd' to toggle glEnable(GL_DEPTH_TEST)", 20, textY); textY+=15;
	textY+=15;
	ofDrawBitmapString("Press 'c' to restore easyCam orientation", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'w' to toggle calibration-points draw", 20, textY); textY+=15;
    ofDrawBitmapString("Press 's' to toggle hand skeleton/cylinders", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'g' to toggle grid", 20, textY); textY+=15;
	

	
}


//--------------------------------------------------------------
void testApp::keyPressed(int key){

    switch(key){
			
		case 'v':
		case 'V':
            bUseVirtualProjector = !bUseVirtualProjector;
            break;
		case 'd':
		case 'D':
			bEnableDepthTest = !bEnableDepthTest;
			break;
			
        case 'c':
            cam.reset();
            break;
		case 'f':
            ofToggleFullscreen();
            break;
        case 'F':
            bUseFbo = !bUseFbo;
            break;
        case 'g':
            leapVisualizer.bDrawGrid = !leapVisualizer.bDrawGrid;
            break;
        case 's':
            leapVisualizer.bDrawSimple = !leapVisualizer.bDrawSimple;
            break;

			
        case 'w':
            bShowCalibPoints = !bShowCalibPoints;
            break;

    }
}







//--------------------------------------------------------------
void testApp::keyReleased(int key){
}
//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}
//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}
//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}
//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
}
//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
}
//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
}
//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
}
//--------------------------------------------------------------
void testApp::exit(){
}
