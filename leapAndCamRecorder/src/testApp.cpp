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

    // ofSetFrameRate(60);
    ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_WARNING);
	ofSetCylinderResolution (16, 1, 16);
	
#ifdef _USE_LIBDC_GRABBER
	// For the ofxLibdc::PointGrey cameraLibdc;
	
	cameraLibdc.setSize(640, 480);
	cameraLibdc.setImageType(OF_IMAGE_COLOR);
	cameraLibdc.setBayerMode(DC1394_COLOR_FILTER_GRBG);
	cameraLibdc.setBlocking(true);
	
	cameraLibdc.setExposure(1.0);
	cameraLibdc.setup();
	
	// After setup it's still possible to change a lot of parameters. If you want
	// to change a pre-setup parameter, the camera will auto-restart
	cameraLibdc.setBrightness(0);
	cameraLibdc.setGain(0);
	cameraLibdc.setExposure(1.0);
	cameraLibdc.setGammaAbs(1);
	cameraLibdc.setShutterAbs(1. / 31.0);
	
#else
	cameraVidGrabber.setVerbose(true);
	cameraVidGrabber.initGrabber(640,480);
#endif
	
	
	bRecording = false;
	currentFrameNumber = 0;
	imageSequence.resize(300);
	currentFrameImg.allocate(640, 480, OF_IMAGE_COLOR);
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate(640,480, OF_IMAGE_COLOR);
	}
	
	
	/// SETUP LEAP
	leap.open();
	cam.setOrientation(ofPoint(-55, 0, 0));

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH);
	
	bDrawSimple = false;
	bDrawGrid   = true;
	
	bPlaying    = false;
	playingFrame = 0; 
	
	bRecording  = false;
	bRecordingThisFrame = false;
	
	lastTagNumber				= 0;
	recordingFrameCount			= 0;
	recordingStartTimeMillis	= 0;
	
	string versionDisplay = "Using openFrameworks version: " + ofToString( ofGetVersionInfo());
	cout << versionDisplay;
	
	
	// we load our settings file
	// if it doesn't exist we can still make one.
	if( XML.loadFile("leapRecording.xml") ){
		printf("Success: leapRecording.xml loaded!\n");
	} else{
		printf("Unable to load leapRecording.xml; check data folder\n");
	}
	int nFrames = XML.getNumTags("FRAME");
	printf("leapRecording.xml contains %d Frames.\n", nFrames);
	
	if ((nFrames > 0) && (leap.isConnected() == false)){
		bPlaying = true;
	}
	
}




//--------------------------------------------------------------
void testApp::update(){

	if (bRecording && !bPlaying){
		recordFrameXML(); // LEAP data
	}
	
	
	bool bCameraHasNewFrame = false;
#ifdef _USE_LIBDC_GRABBER
	; // needs filled in with libdc update
#else
	cameraVidGrabber.update();
	bCameraHasNewFrame = cameraVidGrabber.isFrameNew();
#endif
	
	if (bCameraHasNewFrame){
		currentFrameImg.setFromPixels(cameraVidGrabber.getPixels(), 640,480, OF_IMAGE_COLOR);
	}
	
	
	
	leap.markFrameAsOld();
}


//--------------------------------------------------------------
void testApp::draw(){

	ofBackground(0,0,0);
	
	ofSetColor(ofColor::white);
	currentFrameImg.draw(20,300,320,240);
	
	drawText();
	
	cam.begin();
		drawGrid();
		drawFrame();
	
		if (bPlaying && !bRecording){
			int nFrameTags = XML.getNumTags("FRAME");
			if (nFrameTags > 0){
				drawFrameFromXML (playingFrame);
				playingFrame = (playingFrame+1)%nFrameTags;
			}
		}
	
	cam.end();
}




/*
 
 // WORKING, FROM KYLE'S RECORDER
 
 void testApp::update() {
 // grabVideo() will place the most recent frame into curFrame. If it's a new
 // frame, grabFrame returns true. If there are multiple frames available, it
 // will drop old frames and only give you the newest. This guarantees the
 // lowest latency. If you prefer to not drop frames, set the second argument
 // (dropFrames) to false. By default, capture is non-blocking.
 if(camera.grabVideo(curFrame)) {
 curFrame.update();
 
 
 if(recording) {
 if(currentFrame < imageSequence.size()) {
 ofPixels& target = imageSequence[currentFrame];
 memcpy(target.getPixels(), curFrame.getPixels(), target.getWidth() * target.getHeight() * target.getNumChannels());
 currentFrame++;
 } else {
 recording = false;
 for(int i = 0; i < imageSequence.size(); i++) {
 imageSequence[i].rotate90(1);
 ofSaveImage(imageSequence[i], ofToString(i, 3, '0') + ".jpg");
 }
 }
 }
 }
 }
 
 void testApp::draw() {
 // If the camera isn't ready, the curFrame will be empty.
 if(camera.isReady()) {
 // Camera doesn't draw itself, curFrame does.
 curFrame.draw(0, 0);
 ofDrawBitmapString(ofToString((int) ofGetFrameRate()) + " fps " + (recording ? "recording" : "") , 10, 20);
 }
 }
 
 void testApp::keyPressed(int key) {
 if(key == 'r') {
 recording = true;
 currentFrame = 0;
 }
 if(key == 'i') {
 printf("Attempting save\n");
 curFrame.saveImage(ofToString(ofGetFrameNum()) + ".png");
 }
 }
 */


//--------------------------------------------------------------
void testApp::setColorByFinger (Finger::Type fingerType, Bone::Type boneType){

	// Set the current color, according to the type of this finger.
	// Thumb is red, Index is green, etc.
	
	switch (fingerType){
		case Finger::TYPE_THUMB:
			ofSetColor(ofColor::red);
			break;
		case Finger::TYPE_INDEX:
			ofSetColor(ofColor::green);
			break;
		case Finger::TYPE_MIDDLE:
			ofSetColor(ofColor::blue);
			break;
		case Finger::TYPE_RING:
			ofSetColor(ofColor::yellow);
			break;
		case Finger::TYPE_PINKY:
			ofSetColor(ofColor::cyan);
			break;
		default:
			ofSetColor(ofColor::gray);
			break;
	}
	
	// For the bones inside the palm, set the color to gray.
	bool bSetInternalBonesToGray = true;
	if (bSetInternalBonesToGray){
		if ( (boneType == Bone::TYPE_METACARPAL) ||
			((boneType == Bone::TYPE_PROXIMAL)   && (fingerType == Finger::TYPE_THUMB))) {
			ofSetColor(ofColor::gray);
		}
	}
}



//--------------------------------------------------------------
void testApp::drawOrientedCylinder (ofPoint pt0, ofPoint pt1, float radius){
	
	// Draw a cylinder between two points, properly oriented in space.
	float dx = pt1.x - pt0.x;
	float dy = pt1.y - pt0.y;
	float dz = pt1.z - pt0.z;
	float dh = sqrt(dx*dx + dy*dy + dz*dz);
	
	ofPushMatrix();
		ofTranslate( (pt0.x+pt1.x)/2, (pt0.y+pt1.y)/2, (pt0.z+pt1.z)/2 );
		
		ofQuaternion q;
		q.makeRotate (ofPoint(0, -1, 0), ofPoint(dx,dy,dz) );
		ofMatrix4x4 m;
		q.get(m);
		glMultMatrixf(m.getPtr());
		
		ofDrawCylinder (radius, dh);
	ofPopMatrix();
}


//--------------------------------------------------------------
void testApp::drawText(){
	
	float textY = 20;
	
	ofSetColor(ofColor::white);
	ofDrawBitmapString("Display, record & playback Leap 2.0 Controller data.", 20, textY); textY+=15;
	ofDrawBitmapString("Built in openFrameworks by Golan Levin, golan@flong.com", 20, textY); textY+=15;
	textY+=15;

	ofSetColor( (leap.isConnected()) ?  ofColor::green : ofColor(255,51,51)) ;
	ofDrawBitmapString( (leap.isConnected() ? "Leap is Connected!" : "Leap is NOT CONNECTED!"), 20, textY);
	ofDrawBitmapString( (leap.isConnected() ? "Leap is Connected!" : "Leap is NOT CONNECTED!"), 21, textY); textY+=15;
	textY+=15;
	
	ofSetColor(ofColor::white);
	ofDrawBitmapString("Press ' ' (Space key) to flip display mode", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'c' to restore camera", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'g' to toggle grid", 20, textY); textY+=15;
	textY+=15;
	
	if (leap.isConnected()){
		ofDrawBitmapString("Press 'r' to toggle RECORDING", 20, textY); textY+=15;
	}
	if (XML.getNumTags("FRAME") > 0){
		ofDrawBitmapString("Press 'p' to toggle PLAYBACK",  20, textY); textY+=15;
	}
	
	if (bPlaying){
		ofSetColor(ofColor::green);
		ofDrawBitmapString("PLAYING! " + ofToString(playingFrame), 20, textY);
	} else if (bRecording){
		ofSetColor(ofColor::red);
		ofDrawBitmapString("RECORDING! " + ofToString(recordingFrameCount), 20, textY);
	}
	
}

//--------------------------------------------------------------
void testApp::drawGrid(){
	
	// Draw a grid plane.
	if (bDrawGrid){
		ofPushMatrix();
		ofEnableSmoothing();
		ofRotate(90, 0, 0, 1);
		ofSetColor(160,160,160, 100);
		ofDrawGridPlane(200, 10, false);
		ofPopMatrix();
	}
}



//--------------------------------------------------------------
void testApp::keyPressed(int key){
	if (key == 'c'){
		// Reset the camera if the user presses 'c'.
		cam.reset();
	} else if (key == 'g'){
		// flip whether or not we're drawing a grid.
		bDrawGrid = !bDrawGrid;
		
	} else if ((key == 'p') || (key == 'P')){
		if (XML.getNumTags("FRAME") > 0){
			bPlaying = !bPlaying;
			if (bPlaying){
				bRecording = false;
				playingFrame = 0;
			}
		}
	
    } else if ((key == 'r') || (key == 'R')){
		if (leap.isConnected()){
			// Toggle Recording.
			//reset so we don't store extra tags
			bRecording = !bRecording;
			if (bRecording){
				XML.clear();
				lastTagNumber		= 0;
				recordingFrameCount = 0;
				recordingStartTimeMillis = ofGetElapsedTimeMillis();
				bRecordingThisFrame = false;
				bPlaying = false;
				
				printf("Started recording!\n");
			} else {
				XML.saveFile("leapRecording.xml");
				printf("Recording saved to XML!\n");
			}
		}
	
	} else {
		// When the user presses a key, flip the rendering mode.
		bDrawSimple = !bDrawSimple;
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
    // let's close down Leap and kill the controller
    leap.close();
}
