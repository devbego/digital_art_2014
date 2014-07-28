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
    
    cameraWidth = 640;
    cameraHeight = 480;
	
#ifdef _USE_LIBDC_GRABBER
	// For the ofxLibdc::PointGrey cameraLibdc;
	cout << "libdc cameras found: " << cameraLibdc.getCameraCount() << endl;
    
	cameraLibdc.setSize(cameraWidth, cameraHeight);
	cameraLibdc.setImageType(OF_IMAGE_COLOR);
	//cameraLibdc.setBayerMode(DC1394_COLOR_FILTER_GRBG);
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
	cameraVidGrabber.initGrabber(cameraWidth,cameraHeight);
#endif
	
    
	// Setup video saver
	bRecording = false;
	currentFrameNumber = 0;
	imageSequence.resize(300);
	currentFrameImg.allocate(cameraWidth, cameraHeight, OF_IMAGE_COLOR);
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
	
	   
	// Setup leap
	leap.open();
	cam.setOrientation(ofPoint(-55, 0, 0));

	glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH);
	
    leapVisualizer.setup();
    leapRecorder.setup();

	bPlaying    = false;
	playingFrame = 0;
	bEndRecording = false;
    
	string versionDisplay = "Using openFrameworks version: " + ofToString( ofGetVersionInfo());
	cout << versionDisplay;
	
	folderName = ofGetTimestampString();
    string basePath = ofToDataPath("", true);
    ofSetDataPathRoot("../../../../../SharedData/");
    
    playing = false;
    lastIndexVideoPos.set(0,0,0);
    lastIndexLeapPos.set(0,0,0);
    //loadAndPlayRecording("2014-07-28-15-53-28-121");
                        
    fbo.allocate(cameraHeight,cameraWidth,GL_RGB);
}




//--------------------------------------------------------------
void testApp::update(){
    
    
    //------------- Playback
    if(bPlaying && !bRecording){
        video.update();
        video.setPlaying(playing);
    }
    
    
    bool bCameraHasNewFrame = false;
    
    //------------- Recording
   // if(!bPlaying){
        
        #ifdef _USE_LIBDC_GRABBER
            if (cameraLibdc.grabVideo(currentFrameImg)) {
                bCameraHasNewFrame = true;
                currentFrameImg.update();
            }
        #else
            cameraVidGrabber.update();
            bCameraHasNewFrame = cameraVidGrabber.isFrameNew();
            if (bCameraHasNewFrame){
                currentFrameImg.setFromPixels(cameraVidGrabber.getPixels(), cameraWidth,cameraHeight, OF_IMAGE_COLOR);
            }
        #endif
    //}
        
    if(bRecording && !bPlaying){
        
        if(!bEndRecording && currentFrameNumber >= imageSequence.size()){
            bEndRecording = true;
        }
        
        if(!bEndRecording) {
            
            if(bCameraHasNewFrame){
                leapRecorder.recordFrameXML(leap);
                
                ofPixels& target = imageSequence[currentFrameNumber];
                memcpy (target.getPixels(),
                        currentFrameImg.getPixels(),
                        target.getWidth() * target.getHeight() * target.getNumChannels());
                currentFrameNumber++;
                // leap.markFrameAsOld();??????
            }
            
        } else {
            
            finishRecording();
            loadAndPlayRecording(folderName);

        }
    }
    
    
    

    
    
    
    //-------------  Leap update
	leap.markFrameAsOld();
    
}


//--------------------------------------------------------------
void testApp::draw(){

	ofBackground(0,0,0);
	
	// draw current video image if no in playback mode
    ofSetColor(ofColor::white);
	if(bRecording || !bPlaying){
        currentFrameImg.rotate90(1);
        currentFrameImg.draw(cameraHeight,0);
	}
	
    // draw some info text
    drawText();
	
    // draw leap into fbo
	fbo.begin();
    ofClear(0,0,0,255);
    cam.begin();
		
        leapVisualizer.drawGrid();
		if(!bPlaying)leapVisualizer.drawFrame(leap);
	
		if (bPlaying && !bRecording){
			int nFrameTags = leapVisualizer.XML.getNumTags("FRAME");
			if (nFrameTags > 0){
                
                playingFrame= video.getCurrentFrameID();
                leapVisualizer.drawFrameFromXML(playingFrame);
                
                if( lastIndexVideoPos.x > 0 && lastIndexVideoPos.y > 0){
                    ofSetColor(ofColor::red);
                    ofDrawSphere(lastIndexLeapPos, 5.0f);
                }
                
			}
		}
	cam.end();
    fbo.end();
    
    ofSetColor(255,255,255);
    fbo.draw(0,0);
    
    
    // draw the playback video and recorded mouse position
    if(bPlaying && !bRecording){
        if(active) {
            if (!video.isRolledOver()){
                // export the mesh
                // if (HCAAMB.bCalculatedMesh){
                //	string fileOut = ofToDataPath("", true) + "recording-kyle-ply/" + ofToString(video.getCurrentFrameID(), 3, '0') + ".ply";
                //	HCAAMB.handMesh.save(fileOut);
                //	printf("Output %s!\n", fileOut.c_str());
                // }
            }
        } else {
            ofPushStyle();
            ofSetColor(255);

            if( lastIndexVideoPos.x > 0 && lastIndexVideoPos.y > 0){
                ofNoFill();
                ofEllipse(lastIndexVideoPos,10,10);
                ofFill();
                ofEllipse(lastIndexVideoPos,2,2);
            }
            
            video.draw(480, 0);
            ofPopStyle();
        }
    }
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
	ofDrawBitmapString("Press 's' (Space key) to flip display mode", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'c' to restore camera", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'g' to toggle grid", 20, textY); textY+=15;
	textY+=15;
	
	if (leap.isConnected()){
		ofDrawBitmapString("Press 'r' to toggle RECORDING", 20, textY); textY+=15;
	}
	if (leapVisualizer.XML.getNumTags("FRAME") > 0){
		ofDrawBitmapString("Press 'p' to toggle PLAYBACK",  20, textY); textY+=15;
	}
	
	if (bPlaying){
		ofSetColor(ofColor::green);
		ofDrawBitmapString("PLAYING! " + ofToString(playingFrame), 20, textY);
	} else if (bRecording){
		ofSetColor(ofColor::red);
		ofDrawBitmapString("RECORDING! " + ofToString(leapRecorder.recordingFrameCount), 20, textY);
	}
	
}

//--------------------------------------------------------------
void testApp::finishRecording(){
    
    bRecording = false;
    bEndRecording = false;
    
    int totalImage = MIN(currentFrameNumber,imageSequence.size());
    for(int i = 0; i < totalImage; i++) {
        if(imageSequence[i].getWidth() == 0) break;
        imageSequence[i].rotate90(1);
        ofSaveImage(imageSequence[i], "recordings/"+folderName+"/camera/"+ofToString(i, 3, '0') + ".jpg");
    }
    
    // TODO: check if dir exists
    ofDirectory dir;
    dir.createDirectory("recordings/"+folderName+"/leap");
    leapRecorder.endRecording("recordings/"+folderName+"/leap/leap.xml");
    
    
    imageSequence.clear();
    imageSequence.resize(300);
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
    currentFrameImg.clear();
    currentFrameImg.allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
    
}

//--------------------------------------------------------------
void testApp::loadAndPlayRecording(string folder){
    
    
    leapVisualizer.loadXmlFile("recordings/"+folder+"/leap/leap.xml");
    video.load("recordings/"+folder+"/camera");
    
    indexRecorder.setup("recordings/"+folder+"/calib","fingerCalibPts.xml");
    indexRecorder.setDrawOffsets(cameraHeight,0);
    
    bPlaying = true;
    playing = true;
    currentFrameNumber = 0;
    
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if (key == 'c'){
		// Reset the camera if the user presses 'c'.
		cam.reset();
	} else if ((key == 'p') || (key == 'P')){
		
        if(bPlaying) playing = !playing;
    
    } else if ((key == 'r') || (key == 'R')){
		if (leap.isConnected()){
			// Toggle Recording.
			//reset so we don't store extra tags
			if(bRecording){
                bEndRecording = true;
            }else{
                bRecording = !bRecording;
                folderName = ofGetTimestampString();
				leapRecorder.startRecording();
                leapVisualizer.XML.clear();
                bPlaying = false;
                currentFrameNumber = 0;
                
			}

		}
	
	}else if(key == 's'){
		// When the user presses a key, flip the rendering mode.
        leapVisualizer.bDrawSimple = !leapVisualizer.bDrawSimple;
	}
    
    switch(key){
        case OF_KEY_LEFT:
            video.goToPrevious();
            break;
        case OF_KEY_RIGHT:
            video.goToNext();
            break;
        case 'l':
            if(bPlaying) bPlaying = false;
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
    
    if(bPlaying){
        if(x > indexRecorder.xOffset && y > indexRecorder.yOffset){
            indexRecorder.recordPosition(x, y, leapVisualizer.getIndexFingertipFromXML(video.getCurrentFrameID()),video.getCurrentFrameID());
            lastIndexVideoPos.set(x,y);
            lastIndexLeapPos = leapVisualizer.getIndexFingertipFromXML(video.getCurrentFrameID());
        }
    }
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



