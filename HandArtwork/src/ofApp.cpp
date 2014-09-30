#include "ofApp.h"

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


/*
 
 TODO
 
 Refactor ofApp.cpp/h to have a CameraAndLeapRecorder.
 Enable saving of the various GUIs into XML files.
 Hide/reveal the cursor when we are in play/diagnostic modes.
 
 Scenes: 
 -- completely disable display of sub-GUI
 -- ensure display of Puppet is flush to the right edge of the canvas.
 -- fix "North" scene to correct for 90 rotation. 
 -- In updatePuppeteer(), when bCalculatedMesh is false but a hand is still present, we should show the undistorted video hand instead.
 
 Meshing: 
 -- Count triangles. Deal with missing triangles. (e.g. frame 21)
 
 
 composite laplacian edges.
 
 redesign (simplified) mesh for hands.

 use ROI

 Improve detection of HANDMARK_PINKY_SIDE in HandContourAnalyzer::computePinkySide() with local curvature search. 
 Prevent darkness from climbing up the hand by placing a white stripe at base of arm
 

deal with dark skin thresholding
 == base on map from liz to miranda
 
 White out the wrist before contour detection.

 check all ofmaps' for div-0
 
 dist from wrist better than dist from palm for finger calcs - check thumb in frame 87, 341, 342 in csugrue
 
  computing amount of leap motion doesn't work with delayed data
 leapVisualizer.getMotionAmountFromHandPointVectors() fails when we are using temporally offset data from prevLeapFrameRecorder
  
 feedback if hand is not far in enough, or in too far
 if there's two hands in the scene, use the leap hand whose centroid is closer to the blob

-- Do the crotch search as we already are, but search along the contour between knuckles 0 and 4
-- Sort discovered crotches by index, in the right direction
-- Fingernail-fit between crotches.
-- Compute quality of crotches
-- For crotches of sufficiently low quality, 
 -- Do a pass for edge detection, with malorientation-suppression. 
 -- Threshold to select pixels which represent sufficiently-strong edges
 -- Compute the "perfect line", from the crotch to the midpoint between knuckles
 -- For each thresholded edge pixel, classify according to which perfect ray it's closest to. 
 -- Fit a line through the classified points, creating a best-fit line. 
 -- At regular intervals, Search along the best-fit line for the darkest point of the trough
 -- Collect those points, somehow add them to the contour
 
 Use direction-extremality * fingernailfit to compute tip quality
 make it work with left hands
 
 */



//--------------------------------------------------------------
void ofApp::setup(){
	
	// App settings
    // ofSetFrameRate(60);
    ofSetVerticalSync(false);
	ofSetLogLevel(OF_LOG_WARNING);
	ofSetCylinderResolution (16, 1, 16);
    
    // All data will be in SharedData
    string basePath = ofToDataPath("", true);
    ofSetDataPathRoot("../../../../../SharedData/");
    
    cameraWidth		= 1024;
    cameraHeight	= 768;
	drawW			= cameraWidth/2;
    drawH			= cameraHeight/2;
	
	imgW			= cameraWidth;
    imgH			= cameraHeight;
	
	bWorkAtHalfScale = true; // HAS to be true, cuz something no longer works otherwise.
	if (bWorkAtHalfScale){
		imgW		= cameraWidth/2;
		imgH		= cameraHeight/2;
	}
	
    
    initializeCamera();
	initializeCameraCalibration();
	
    
    //--------------- Setup video saver
	bRecording = false;
	currentFrameNumber = 0;
	imageSequence.resize(500);
	
	// currentFrameImg and processFrameImg are both at the camera resolution.
    currentFrameImg.allocate (cameraWidth, cameraHeight, OF_IMAGE_COLOR);
    processFrameImg.allocate (cameraWidth, cameraHeight, OF_IMAGE_COLOR);
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate (cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
    

    playingFrame				= 0;
    bInPlaybackMode				= false;
	bEndRecording				= false;
    playing						= false;
    bUseVirtualProjector		= false;
    bUseFbo						= true;
    bInputMousePoints			= false;
    bShowCalibPoints			= false;
    bRecordingForCalibration	= false;
    bRecordThisCalibFrame		= false;
    bUseCorrectedCamera			= true;
    bShowText					= true;
    bShowLargeCamImageOnTop		= false;    // temp for quickly showing on hand video only
	bDrawContourAnalyzer		= true;
	bComputeAndDisplayPuppet	= false;
	bFullscreen					= false;
	bComputePixelBasedFrameDifferencing = false;
	bDoCompositeThresholdedImageWithLeapFboPixels = true;
	
	
	
	//--------------- Setup leap
	leap.open();
    leapVisualizer.setup();
	leapVisualizer.enableVoronoiRendering (imgW, imgH, bWorkAtHalfScale);
	leapVisualizer.bDrawGrid = false;
    leapRecorder.setup();
	
	bShowOffsetByNFrames		= false;
    framesBackToPlay			= 0;
	maxNPrevLeapFrames			= 10;
	prevLeapFrameRecorder.setup();
	
	cam.setOrientation(ofPoint(-55, 0, 0));

	leapColorFbo.allocate		(imgW,imgH, GL_RGBA);
    leapDiagnosticFbo.allocate	(imgW,imgH, GL_RGB);

	
    folderName = ofGetTimestampString();
    lastIndexVideoPos.set(0,0,0);
    lastIndexLeapPos.set(0,0,0);
    
    ofEnableAlphaBlending();
	
    glEnable(GL_NORMALIZE); // needed??
	glDisable (GL_DEPTH);
    glDisable(GL_DEPTH_TEST);
    
    string versionDisplay = "Using openFrameworks version: " + ofToString( ofGetVersionInfo());
	cout << versionDisplay;
	
	
	
	//-------------------------------------------
	bUseROIForFilters			= false;
	bUseRedChannelForLuminance	= true;
	bDoMorphologicalOps			= true;
	bDoAdaptiveThresholding		= false;
	bDoLaplacianEdgeDetect		= true;

	blurKernelSize				= 4.0;
	laplaceKSize				= 7;
	blurredStrengthWeight		= 0.07;
	thresholdValue				= 26;
	prevThresholdValue			= 0;
	laplaceDelta				= 125.0;
	laplaceSensitivity			= 0.59;
	
	colorVideo.allocate			(cameraWidth, cameraHeight);
	
	colorVideoHalfScale.allocate(imgW, imgH);
	leapFboPixels.allocate		(imgW, imgH, OF_IMAGE_COLOR);
	
	
	grayMat.create				(imgH, imgW, CV_8UC1);
	prevGrayMat.create			(imgH, imgW, CV_8UC1);
	diffGrayMat.create			(imgH, imgW, CV_8UC1);
	
	blurred.create				(imgH, imgW, CV_8UC1);
	thresholded.create			(imgH, imgW, CV_8UC1);
	thresholdedFinal.create		(imgH, imgW, CV_8UC1);
	edgesMat1.create			(imgH, imgW, CV_8UC1);
	adaptiveThreshImg.create	(imgH, imgW, CV_8UC1);
	thresholdConstMat.create	(imgH, imgW, CV_8UC1);
	tempGrayscaleMat1.create	(imgH, imgW, CV_8UC1);
	tempGrayscaleMat2.create	(imgH, imgW, CV_8UC1);
	handPartIDImg.create		(imgH, imgW, CV_8UC1);
	handPartIDTmpImg.create		(imgH, imgW, CV_8UC1);
	
	videoMat.create				(imgH, imgW, CV_8UC3);
	leapDiagnosticFboMat.create	(imgH, imgW, CV_8UC3); // 3-channel
	coloredBinarizedImg.create	(imgH, imgW, CV_8UC3); // 3-channel
	thresholdedFinal8UC3.create (imgH, imgW, CV_8UC3);
	
	graySmall.create			(imgH/4, imgW/4, CV_8UC1);
	blurredSmall.create			(imgH/4, imgW/4, CV_8UC1);
	
	rgbVideoChannelMats[0].create (imgH, imgW, CV_8UC1);
	rgbVideoChannelMats[1].create (imgH, imgW, CV_8UC1);
	rgbVideoChannelMats[2].create (imgH, imgW, CV_8UC1);
	
	leapDiagnosticFboChannelMats[0].create (imgH, imgW, CV_8UC1);
	leapDiagnosticFboChannelMats[1].create (imgH, imgW, CV_8UC1);
	leapDiagnosticFboChannelMats[2].create (imgH, imgW, CV_8UC1);

	
	//-------------------------------------
	// Clear the contents of all images.
	grayMat.setTo(0);
	blurred.setTo(0);
	thresholded.setTo(0);
	thresholdedFinal.setTo(0);
	adaptiveThreshImg.setTo(0);
	thresholdConstMat.setTo(0);
	tempGrayscaleMat1.setTo(0);
	tempGrayscaleMat2.setTo(0);
	
	leapDiagnosticFboMat.setTo(0);
	coloredBinarizedImg.setTo(0);
	graySmall.setTo(0);
	blurredSmall.setTo(0);
	//------------------------
	
	
	whichImageToDraw = 5;
	
	
	// Get us ready to demo in a hurry
	string filePathCalib = "sep15_CALIBRATION"; //"calib_chris_corrected_4";
	calibrateFromXML(filePathCalib);
	
	string filePathPlay = "sep15-golan-perfect"; //"play_chris_corrected_4";
	folderName = filePathPlay;
	loadAndPlayRecording(filePathPlay);
	bUseVirtualProjector = true;

	amountOfPixelMotion01 = 0;
	amountOfLeapMotion01 = 0;
	zHandExtent = 0.00;
	motionAlpha = 0.95;
	zExtentAlpha = 0.3;
	fingerCurlAlpha = 0.65;
	amountOfFingerCurl01 = 0;
	
	elapsedMicros = 0;
	elapsedMicrosInt = 0;
	
	int morph_size = 1;
	int morph_type = cv::MORPH_ELLIPSE;
	morphStructuringElt = getStructuringElement(morph_type,
												cv::Size( 2*morph_size + 1, 2*morph_size+1 ),
												cv::Point(  morph_size,       morph_size ) );
	
	myHandContourAnalyzer.setup(imgW, imgH);
	myHandMeshBuilder.initialize(imgW, imgH);
	myHandMeshBuilder.setWorkAtHalfScale(bWorkAtHalfScale);
	
	minHandInsertionPercent = 0.29;
	maxAllowableMotion		= 15.0;
	maxAllowableFingerCurl	= 0.3;
	maxAllowableExtentZ		= 0.5;
	
	
	myPuppetManager.setupPuppeteer (myHandMeshBuilder);
	myPuppetManager.setupPuppetGui ();
	
	// must be last
	setupGui();
	
}



//--------------------------------------------------------------
void ofApp::initializeCamera(){
	
	//--------------- Setup camera grabber
	#ifdef _USE_LIBDC_GRABBER
		// For the ofxLibdc::PointGrey cameraLibdc;
		cout << "libdc cameras found: " << cameraLibdc.getCameraCount() << endl;
		
        ofSetVerticalSync(false);
		cameraLibdc.setImageType(OF_IMAGE_COLOR);
		cameraLibdc.setSize (cameraWidth, cameraHeight);
        cameraLibdc.setImageType(OF_IMAGE_COLOR);
        cameraLibdc.setBayerMode(DC1394_COLOR_FILTER_GRBG); // why turns camera video grayscale???
    
        cameraLibdc.setBlocking(true);
        cameraLibdc.setFrameRate(30);
    
        cameraLibdc.setExposure(1.0);
        cameraLibdc.setup();
    
		cameraLibdcShutterInv = 40.0;
		cameraLibdcBrightness = 0;
		cameraLibdcGain = 0.0;
		cameraLibdcGamma = 1.0;
    
        cameraLibdc.setBrightness(cameraLibdcBrightness);
        cameraLibdc.setGain(cameraLibdcGain);
        cameraLibdc.setGammaAbs(cameraLibdcGamma);
		cameraLibdc.setShutterAbs(1.0 / cameraLibdcShutterInv);
		
		
	
	#else
		cameraVidGrabber.setVerbose(true);
		cameraVidGrabber.initGrabber(cameraWidth,cameraHeight);
	#endif
}

//--------------------------------------------------------------
void ofApp::initializeCameraCalibration(){
	
	//--------------- Set up corrected camera calibration
	#ifdef _USE_CORRECTED_CAMERA
		// We are undistorting the camera.
		ofxCv::FileStorage settings (ofToDataPath("settingsForCameraCalibrator.yml"), ofxCv::FileStorage::READ);
		if(settings.isOpened()) {
			
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
}

//========================================================================
void ofApp::setupGui() {
	
	guiTabBar = new ofxUITabBar();
	guiTabBar->setName("GUI");
	
	//------------------------------------
	ofxUICanvas* gui0 = new ofxUICanvas();
	gui0->setName("GUI0");
	gui0->addLabel("GUI0");
	
	// Camera controls.
	gui0->addSlider("LibDC Camera Shutter",		32.0, 100.0,	&cameraLibdcShutterInv);
	gui0->addSlider("LibDC Camera Brightness",	0.0, 1.0,		&cameraLibdcBrightness);
	gui0->addSlider("LibDC Camera Gain",		0.00, 1.0,		&cameraLibdcGain);
	gui0->addSlider("LibDC Camera Gamma",		0.00, 3.0,		&cameraLibdcGamma);
	
	// Display of time consumption for vision & meshing.
	gui0->addSpacer();
	gui0->addFPS();
	gui0->addValuePlotter("Vision: Micros", 256, 0, 50000, &elapsedMicros);
	gui0->addIntSlider("Vision: Micros", 0, 50000, &elapsedMicrosInt);
	
	// Display of time consumption for puppeteering.
	gui0->addSpacer();
	gui0->addLabelToggle("Fullscreen",					&bFullscreen);
	gui0->addLabelToggle("Do Puppet",					&bComputeAndDisplayPuppet);
	gui0->addValuePlotter("Puppet: Micros", 256, 0, 50000, &(myPuppetManager.elapsedPuppetMicros));
	gui0->addIntSlider("Puppet: Micros", 0, 50000, &(myPuppetManager.elapsedPuppetMicrosInt));
	
	gui0->autoSizeToFitWidgets();
	ofAddListener(gui0->newGUIEvent,this,&ofApp::guiEvent);
	guiTabBar->addCanvas(gui0);
	guis.push_back(gui0);
	
	//------------------------------------
	ofxUICanvas* gui1 = new ofxUICanvas();
	gui1->setName("GUI1");
	gui1->addLabel("GUI1");
	
	gui1->loadSettings(originalAppDataPath + "HandSegmenterSettings.xml");
	gui1->addSpacer();
	
	gui1->addSlider("thresholdValue", 0.0, 128.0, &thresholdValue);
	gui1->addSlider("blurKernelSize", 1, 63, &blurKernelSize);
	gui1->addSlider("blurredStrengthWeight", 0.0, 1.0, &blurredStrengthWeight);
	gui1->addSlider("laplaceDelta",		0,	255, &laplaceDelta);
	gui1->addSlider("laplaceSensitivity",0.0, 4.0, &laplaceSensitivity);
	
	gui1->addSpacer();
	gui1->addLabelToggle("bUseROIForFilters",			&bUseROIForFilters);
	gui1->addLabelToggle("bUseRedChannelForLuminance",	&bUseRedChannelForLuminance);
	gui1->addLabelToggle("bDoAdaptiveThresholding",		&bDoAdaptiveThresholding);
	gui1->addLabelToggle("bDoMorphologicalOps",			&bDoMorphologicalOps);
	gui1->addLabelToggle("bDoLaplacianEdgeDetect",		&bDoLaplacianEdgeDetect);

	gui1->loadSettings(originalAppDataPath + "HandSegmenterSettings.xml");
	gui1->autoSizeToFitWidgets();
	ofAddListener(gui1->newGUIEvent,this,&ofApp::guiEvent);
	guiTabBar->addCanvas(gui1);
	guis.push_back(gui1);
	
	
	//------------------------------------
	ofxUICanvas* gui2 = new ofxUICanvas();
	gui2->setName("GUI2");
	gui2->addLabel("GUI2");
	
	gui2->addIntSlider("smoothingOfLowpassContour", 1, 15,	&(myHandContourAnalyzer.smoothingOfLowpassContour));
	gui2->addSlider("crotchCurvaturePowf",  0.0, 2.0,		&(myHandContourAnalyzer.crotchCurvaturePowf));
	gui2->addSlider("crotchDerivativePowf", 0.0, 2.0,		&(myHandContourAnalyzer.crotchDerivativePowf));
	gui2->addIntSlider("crotchSearchRadius", 1, 32,			&(myHandContourAnalyzer.crotchSearchRadius));
	gui2->addSlider("crotchContourSearchMask", 0.0, 0.5,&(myHandContourAnalyzer.crotchContourSearchTukeyMaskPct));
	
	gui2->autoSizeToFitWidgets();
	ofAddListener(gui2->newGUIEvent,this,&ofApp::guiEvent);
	guiTabBar->addCanvas(gui2);
	guis.push_back(gui2);
	
	//------------------------------------
	// GUI for Application State
	ofxUICanvas* gui3 = new ofxUICanvas();
	gui3->setName("GUI3");
	gui3->addLabel("GUI3");
	gui3->addSlider("minHandInsertionPercent",  0.0, 1.0,	&minHandInsertionPercent);
	
	gui3->addSpacer();
	gui3->addValuePlotter("amountOfLeapMotion01", 256,	0.00, 25.0, &amountOfLeapMotion01, 32);
	gui3->addSlider("amountOfLeapMotion01",				0.00, 25.0, &amountOfLeapMotion01);
	gui3->addSlider("maxAllowableMotion",				0.00, 25.0, &maxAllowableMotion );
	gui3->addSpacer();
	gui3->addValuePlotter("amountOfFingerCurl01", 256,	0.00, 1.00, &amountOfFingerCurl01, 32);
	gui3->addSlider("amountOfFingerCurl01",				0.00, 1.00, &amountOfFingerCurl01 );
	gui3->addSlider("maxAllowableFingerCurl",			0.00, 1.00, &maxAllowableFingerCurl );
	gui3->addSpacer();
	gui3->addValuePlotter("zHandExtent",		  256,	0.00, 2.00, &zHandExtent, 32);
	gui3->addSlider("zHandExtent",						0.00, 2.00, &zHandExtent );
	gui3->addSlider("maxAllowableExtentZ",				0.00, 2.00, &maxAllowableExtentZ );

	
	gui3->autoSizeToFitWidgets();
	ofAddListener(gui3->newGUIEvent,this,&ofApp::guiEvent);
	guiTabBar->addCanvas(gui3);
	guis.push_back(gui3);
	

}


//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e) {
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    string canvasParent = e.widget->getCanvasParent()->getName();
    // cout << canvasParent << endl;
}

int getSelection(ofxUIRadio* radio) {
	vector<ofxUIToggle*> toggles = radio->getToggles();
	for (int i = 0; i < toggles.size(); i++) {
		if (toggles[i]->getValue()) {
			return i;
		}
	}
	return -1;
}



//--------------------------------------------------------------
void ofApp::update(){
	
	long long computerVisionStartTime = ofGetElapsedTimeMicros();
	
	updateBufferedVideoPlaybackIfUsingStoredVideo();
	updateLiveVideoIfUsingCameraSource();
	
	updateProcessFrameImg();
	updateRecordingSystem();
	
	computeHandStatistics();
	leapVisualizer.updateHandPointVectors();
	leapVisualizer.updateVoronoiExpansion();

	renderDiagnosticLeapFboAndExtractItsPixelData();

	updateComputerVision();
	updateHandMesh();
	updateLeapHistoryRecorder();
	
	// Update the app's main state machine, including feedback to the user.
	applicationStateMachine();

	// Update our measurement of NON-PUPPET cpu consumption:
	long long computerVisionEndTime = ofGetElapsedTimeMicros();
	float elapsedMicrosThisFrame = (float)(computerVisionEndTime - computerVisionStartTime);
	elapsedMicros = 0.8*elapsedMicros + 0.2*elapsedMicrosThisFrame;
	elapsedMicrosInt = (int) elapsedMicros;
	
	
	
	/*
	printf("%d: currentFrameNumber %d\n", (int) ofGetElapsedTimeMillis(), playingFrame);
	if (playingFrame == 99){
		myHandMeshBuilder.handMesh.save("roxy-99.ply");
		printf("Saved Roxy99\n"); 
	}
	*/
	
	// Update all aspects of the puppet geometry
	myPuppetManager.updatePuppeteer( bComputeAndDisplayPuppet, myHandMeshBuilder);

}

//--------------------------------------------------------------
void ofApp::updateHandMesh(){
	
	ofPolyline &theHandContour = myHandContourAnalyzer.theHandContourResampled;
	Handmark *theHandmarks = myHandContourAnalyzer.Handmarks;
	ofVec3f theHandCentroid = myHandContourAnalyzer.handCentroidLeap;
	myHandMeshBuilder.buildMesh (theHandContour, theHandCentroid, theHandmarks);
}



//--------------------------------------------------------------
void ofApp::updateBufferedVideoPlaybackIfUsingStoredVideo(){
	
	//------------- Playback
	// Updates the BufferedVideo playback object, which fetches images from disk.
    if (bInPlaybackMode && !bRecording){
        video.update();
        video.setPlaying(playing);
    }
}

//--------------------------------------------------------------
void ofApp::updateLiveVideoIfUsingCameraSource(){
	
    //------------- Frame grabbing.
	// Copy fresh camera data into currentFrameImg.
    bCameraHasNewFrame = false;
	
#ifdef _USE_LIBDC_GRABBER
	
	if (cameraLibdc.grabVideo(currentFrameImg)) {
		
		cameraLibdc.setShutterAbs	(1.0 / cameraLibdcShutterInv);
		cameraLibdc.setBrightness	(cameraLibdcBrightness);
		cameraLibdc.setGain			(cameraLibdcGain);
		cameraLibdc.setGammaAbs		(cameraLibdcGamma);
		
		currentFrameImg.update();
		bCameraHasNewFrame = true;
	}
#else
	cameraVidGrabber.update();
	bCameraHasNewFrame = cameraVidGrabber.isFrameNew();
	if (bCameraHasNewFrame){
		currentFrameImg.setFromPixels(cameraVidGrabber.getPixels(), cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
#endif


}

//--------------------------------------------------------------
void ofApp::updateProcessFrameImg(){
	
	//------------- Camera undistortion.
    // If selected, undistort the currentFrameImg into processFrameImg.
	// (Otherwise, just copy currentFrameImg into processFrameImg).
    if (useCorrectedCam()){
        if (myCalibration.size() > 0) {
            myCalibration.undistort(ofxCv::toCv(currentFrameImg), ofxCv::toCv(processFrameImg));
            processFrameImg.update();
        }
    } else{
        processFrameImg = currentFrameImg;
        processFrameImg.update();
    }
}


//--------------------------------------------------------------
void ofApp::updateRecordingSystem(){
	
	//------------- Recording (state machine).
	// Stash camera images to disk, as appropriate.
    if (bRecording && !bInPlaybackMode){
        if (!bEndRecording && currentFrameNumber >= imageSequence.size()){
            bEndRecording = true;
        }
        
        if (!bEndRecording) {
            if(bCameraHasNewFrame){
                
                // If in calibration record, mode only record some frames
                if( (bRecordingForCalibration && bRecordThisCalibFrame) || !bRecordingForCalibration){
                    leapRecorder.recordFrameXML(leap);
                    
                    ofPixels& target = imageSequence[currentFrameNumber];
                    memcpy (target.getPixels(),
                            processFrameImg.getPixels(),
                            target.getWidth() * target.getHeight() * target.getNumChannels());
					
                    currentFrameNumber++;
                    
                    if(bRecordingForCalibration){
                        bRecordThisCalibFrame = false;
                    }
                    // leap.markFrameAsOld();??????
                }
            }
            
        } else {
            finishRecording();
        }
    }
    
	
    //-------------  Leap update
	leap.markFrameAsOld();
}


//--------------------------------------------------------------
void ofApp::updateLeapHistoryRecorder(){
	//------------- Store last frame
    if (prevLeapFrameRecorder.XML.getNumTags("FRAME") > maxNPrevLeapFrames){
        prevLeapFrameRecorder.XML.removeTag("FRAME",0);
    }
    prevLeapFrameRecorder.recordFrameXML(leap);
}

//--------------------------------------------------------------
void ofApp::updateComputerVision(){
	
	// currentFrameImg comes directly from the camera, unless there's no camera, in which case it's just black & exists.
	// processedFrameImg is a copy of currentFrameImg if there's no undistortion being done; if it is, it's undistorted.
	// video is the playback buffered video

	// when live: use color img from processFrameImg
	// when playing, use color img from buffered video.
	if (bInPlaybackMode){
		extractVideoMatFromBufferedVideoFrame();		// scale down and extract grayscale.
	} else {
		extractVideoMatFromLiveVideo();
	}

	extractLuminanceChannelFromSourceVideoMat(); 
	computeFrameDifferencing();							// not used presently
	thresholdLuminanceImage();
	applyMorphologicalOps();
	applyEdgeAmplification();
	compositeThresholdedImageWithLeapFboPixels();
	
	myHandContourAnalyzer.update (thresholdedFinal, leapDiagnosticFboMat, leapVisualizer);
}



//--------------------------------------------------------------
void ofApp::extractVideoMatFromLiveVideo(){
	// processFrameImg contains the live color video from the camera.
	if (bWorkAtHalfScale){
		colorVideo.setFromPixels(processFrameImg.getPixelsRef());
		colorVideoHalfScale.scaleIntoMe (colorVideo);
		videoMat = toCv(colorVideoHalfScale);
	} else {
		videoMat = toCv(processFrameImg);
	}
}


//--------------------------------------------------------------
void ofApp::extractVideoMatFromBufferedVideoFrame(){
	
	// videoMat is a cv::Mat version of the current video frame.
	// It's an intermediary on the way to computing grayMat.
	
	//------------------------
	// Fetch the (color) video
	if (bWorkAtHalfScale){
		colorVideo.setFromPixels(video.getPixelsRef());
		colorVideoHalfScale.scaleIntoMe (colorVideo);
		videoMat = toCv(colorVideoHalfScale);
	} else {
		videoMat = toCv(video);
	}
}

//--------------------------------------------------------------
void ofApp::extractLuminanceChannelFromSourceVideoMat(){
	//------------------------
	// Extract (or compute) the grayscale luminance channel.
	// Either use just the red channel (a very good proxy for skin),
	// Or use the properly weighted components (more computationally expensive).
	if (bUseRedChannelForLuminance){
		split(videoMat, rgbVideoChannelMats);
		grayMat = rgbVideoChannelMats[0];
	} else {
		convertColor(videoMat, grayMat, CV_RGB2GRAY);
	}

}


//--------------------------------------------------------------
void ofApp::computeFrameDifferencing(){
	if (bComputePixelBasedFrameDifferencing){
	
		// Compute frame-differencing in order to estimate the amount of motion in the frame.
		// When there's too much motion in the frame (i.e. hands shaking around too much),
		// then we'll use this information to decide whether or not to do the special effects.
		// By not running the special effects when there's too much motion, we hope to
		// encourage the visitor to hold still and move slowly :)
		// Uses grayMat, prevGrayMat, and diffGrayMat.
		
		//frame-differencing parameters:
		float diffThreshValue = 20;
		
		// Take the abs value of the difference between curr and prev,
		// store in diff, then swap curr into prev:
		absdiff (grayMat, prevGrayMat, diffGrayMat );
		grayMat.copyTo (prevGrayMat);
		
		// Threshold the diffGrayMat into itself
		cv::threshold (diffGrayMat, diffGrayMat, diffThreshValue, 255, cv::THRESH_BINARY);
		
		int count = countNonZero( diffGrayMat );
		float motionf = (float) count / (float)(imgW * imgH);
		amountOfPixelMotion01 = motionAlpha*amountOfPixelMotion01 + (1.0-motionAlpha)*motionf;
	}
}



//--------------------------------------------------------------
void ofApp::thresholdLuminanceImage(){
	
	if (bDoAdaptiveThresholding){
		
		// Copy the gray image to a very small version, which is faster to operate on;
		cv::Size blurredSmallSize = cv::Size(imgW/4, imgH/4);
		resize(grayMat, graySmall, blurredSmallSize, 0,0, INTER_NEAREST );
		
		// Blur the heck out of that very small version
		int kernelSize = ((int)(blurKernelSize)*2 + 1);
		float thresholdBlurSigma = kernelSize/2.0;
		cv::Size blurSize = cv::Size(kernelSize, kernelSize);
		GaussianBlur (graySmall, blurredSmall, blurSize, thresholdBlurSigma);
		
		// Upscale the small blurry version into a large blurred version
		cv::Size blurredSize = cv::Size(imgW,imgH);
		resize(blurredSmall, blurred, blurredSize, 0,0, INTER_LINEAR );
		
		// Fill the thresholdConstMat with the threshold value (but only if it has changed).
		if (thresholdValue != prevThresholdValue){
			thresholdConstMat.setTo( (unsigned char) ((int)thresholdValue));
		}
		prevThresholdValue = thresholdValue;
		
		// Create the adaptiveThreshImg by adding a weighted blurred + constant image.
		cv::scaleAdd (blurred, blurredStrengthWeight, thresholdConstMat, adaptiveThreshImg);
		
		// Do the actual (adaptive thresholding.
		cv::subtract (grayMat, adaptiveThreshImg, tempGrayscaleMat1);
		int thresholdMode = cv::THRESH_BINARY; // cv::THRESH_BINARY_INV
		cv::threshold (tempGrayscaleMat1, thresholded, 1, 255, thresholdMode);
		
	} else {
		
		// If we are not adaptive thresholding, just use a regular threshold.
		threshold(grayMat, thresholded, thresholdValue);
	}
}

//--------------------------------------------------------------
void ofApp::applyMorphologicalOps(){
	
	if (bDoMorphologicalOps){
		
		// Apply the erosion operation
		cv::erode ( thresholded, thresholded,		morphStructuringElt );
		cv::erode ( thresholded, thresholded,		morphStructuringElt );
		cv::dilate( thresholded, thresholdedFinal,	morphStructuringElt );
		
	} else {
		
		thresholded.copyTo (thresholdedFinal);
	}
}


//--------------------------------------------------------------
void ofApp::applyEdgeAmplification(){
	
	if (bDoLaplacianEdgeDetect){
		
		/*
		// Canny edge detection
		int edgeThresh = 1;
		int lowThreshold = (int) laplaceDelta;
		int const max_lowThreshold = 100;
		int ratio = 3;
		blur ( grayMat, edgesMat1, cv::Size(3,3) );
		int kernel_size = 3;
		cv::Canny( edgesMat1, edgesMat1, lowThreshold, lowThreshold*ratio, kernel_size );
		*/
		

		/*
		// Laplacian edge detection
		blur ( grayMat, grayMat, cv::Size(5,5) );
		int		kSize = 7; //laplaceKSize; //7;
		double	sensitivity = (double)laplaceSensitivity / 100.0;
		cv::Laplacian (grayMat, edgesMat1, -1, kSize, (double)sensitivity, (double)laplaceDelta, cv::BORDER_DEFAULT );
		*/
		
		
		/* 
		// Sobel edge detection
		int scale = 1;
		int delta = 0;
		int ddepth = CV_16S;
		
		Mat grad_x, grad_y;
		Mat abs_grad_x, abs_grad_y;
		Mat src = grayMat;
		blur ( src, src, cv::Size(7,7) );

		// Gradient X
		Sobel( src, grad_x, ddepth, 1, 0, 3, scale, delta, BORDER_DEFAULT );
		convertScaleAbs( grad_x, abs_grad_x );
		
		// Gradient Y
		Sobel( src, grad_y, ddepth, 0, 1, 3, scale, delta, BORDER_DEFAULT );
		convertScaleAbs( grad_y, abs_grad_y );
		
		// Total Gradient (approximate)
		addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, edgesMat1 );
		*/
		

		// Some combination techniques
		// cv::addWeighted( abs_grad_x, 0.5, abs_grad_y, 0.5, 0, edgesMat1 );
		// cv::max(edgesMat1, laplaceEdgesMat, edgesMat1);
		// cv::multiply(edgesMat1, laplaceEdgesMat, edgesMat1, 1.0/256.0);
		
	} else {
		// thresholdedFinal = thresholded.clone();
	}
	
}







//--------------------------------------------------------------
void ofApp::computeHandStatistics(){
	// determine the amount of movement, the vertical extent, and the finger curledness.
	// can be used later to decide whether or not to run more advanced processing.
	if (leapToCameraCalibrator.calibrated && bUseVirtualProjector){
		
		// Update the calculated amount of hand movement (derived from leap)
		float movement = leapVisualizer.getMotionAmountFromHandPointVectors();
		float curl     = 0.1 * leapVisualizer.getCurlFromHandPointVectors();
		
		ofVec2f zRange = leapVisualizer.getZExtentFromHandPointVectors();
		float minZ = 100.0 * zRange.x;
		float maxZ = 100.0 * zRange.y;
		float zextent = fabs(maxZ - minZ);
		
		if ((bInPlaybackMode && playing) || (!bInPlaybackMode)){
			amountOfLeapMotion01 = (motionAlpha*amountOfLeapMotion01)		+ (1.0-motionAlpha)*movement;
			zHandExtent          = (zExtentAlpha*zHandExtent)				+ (1.0-zExtentAlpha)*zextent;
			zHandHeight          = (zExtentAlpha*zHandHeight)				+ (1.0-zExtentAlpha)*(100.0 - minZ);
			amountOfFingerCurl01 = (fingerCurlAlpha*amountOfFingerCurl01)	+ (1.0-fingerCurlAlpha)*curl;
		}
	}
}

//--------------------------------------------------------------
void ofApp::renderDiagnosticLeapFboAndExtractItsPixelData(){
	
	// This is for the purposes of computer vision -- not display:
	// Render the Leap into leapDiagnosticFbo using diagnostic colors:
	// Colors which encode the orientation and identity of the fingers, etc.
	{
		leapDiagnosticFbo.begin();
		ofClear(0,0,0,0);
		glEnable(GL_DEPTH_TEST);
		
		if (leapToCameraCalibrator.calibrated && bUseVirtualProjector){
			leapToCameraCalibrator.projector.beginAsCamera();
			
			leapVisualizer.bDrawDiagnosticColors = true;
			leapVisualizer.setProjector(leapToCameraCalibrator.projector);
			ofFill();
									
			if (bInPlaybackMode && !bRecording){ // CANNED LEAP
				// draw leap from pre-loaded xml recording
				int nFrameTags = leapVisualizer.myXML.getNumTags("FRAME");
				if (nFrameTags > 0){
					
					// Compute the index of which frame to draw.
					playingFrame = video.getCurrentFrameID();
					int whichFrame = playingFrame;
					if (playingFrame > 0 && bShowOffsetByNFrames){
						whichFrame = (playingFrame - framesBackToPlay + nFrameTags)%nFrameTags;
					}
					
					
					// "Fluff out" the hand rendering with a Voronoi-based expansion halo
					// computed through OpenGL tricks on the graphics card.
					leapVisualizer.drawVoronoiFrameFromXML (whichFrame, leapVisualizer.myXML);
					leapToCameraCalibrator.projector.endAsCamera();
					glDisable(GL_DEPTH_TEST);
					leapVisualizer.drawVoronoi();
					glEnable(GL_DEPTH_TEST);
					leapToCameraCalibrator.projector.beginAsCamera();
					
					// Render the actual CGI hand, on top of the voronoi diagram (using diagnostic colors)
					leapVisualizer.drawFrameFromXML(whichFrame);
					
				}
				
			} else { // LIVE LEAP
				
								
				// "Fluff out" the hand rendering with a Voronoi-based expansion halo
				// computed through OpenGL tricks on the graphics card.
				if (bShowOffsetByNFrames && (framesBackToPlay > 0)){
					int totalFramesSaved = prevLeapFrameRecorder.XML.getNumTags("FRAME");
					int whichFrame = totalFramesSaved - framesBackToPlay;
					if (whichFrame < 0) whichFrame = 0;
					leapVisualizer.drawVoronoiFrameFromXML (whichFrame, prevLeapFrameRecorder.XML);
				} else {
					// the usual case: no frame delay
					leapVisualizer.drawVoronoiFrame(leap);
				}
				
				leapToCameraCalibrator.projector.endAsCamera();
				glDisable(GL_DEPTH_TEST);
				leapVisualizer.drawVoronoi();
				glEnable(GL_DEPTH_TEST);
				leapToCameraCalibrator.projector.beginAsCamera();
				
				
				// draw live leap
				if (bShowOffsetByNFrames && (framesBackToPlay > 0)){
					int totalFramesSaved = prevLeapFrameRecorder.XML.getNumTags("FRAME");
					int whichFrame = totalFramesSaved - framesBackToPlay;
					if (whichFrame < 0) whichFrame = 0;
					leapVisualizer.drawFrameFromXML (whichFrame, prevLeapFrameRecorder.XML); // working
				} else {
					leapVisualizer.drawFrame(leap); // no frame delay. working
				}
				
			}
			leapToCameraCalibrator.projector.endAsCamera();
		}

		glDisable(GL_DEPTH_TEST);
		leapDiagnosticFbo.end();
	}
}

//--------------------------------------------------------------
void ofApp::compositeThresholdedImageWithLeapFboPixels(){
	
	// Extract 8UC3 pixel data from leapDiagnosticFbo into leapFboPixels
	leapDiagnosticFbo.readToPixels(leapFboPixels);
	unsigned char *leapDiagnosticFboPixelData = leapFboPixels.getPixels();
	leapDiagnosticFboMat.data = leapDiagnosticFboPixelData;
	cv::flip(leapDiagnosticFboMat, leapDiagnosticFboMat, 0);
		
	if (bDoCompositeThresholdedImageWithLeapFboPixels){
		// Composite the colored orientation image (in leapFboMat) against
		// the thresholdedFinal (in an RGBfied version), to produce the coloredBinarizedImg
		thresholdedFinalThrice[0] = thresholdedFinal;
		thresholdedFinalThrice[1] = thresholdedFinal;
		thresholdedFinalThrice[2] = thresholdedFinal;
		cv::merge(thresholdedFinalThrice, 3, thresholdedFinal8UC3);
		cv::bitwise_and(leapDiagnosticFboMat, thresholdedFinal8UC3, coloredBinarizedImg);
	}
}









//--------------------------------------------------------------
void ofApp::draw(){

	ofSetFullscreen(bFullscreen);
    ofBackground(20);
	guiTabBar->setPosition(20,20);
	
    
    if (!bInPlaybackMode){
        drawLiveForRecording();
    } else {
        drawPlayback();
        
        // draw mouse cursor for calibration input
        if(bInPlaybackMode && bInputMousePoints){
            ofPushStyle();
            ofNoFill();
            ofSetColor(0,255,0);
            ofEllipse(mouseX, mouseY-20, 16, 16);
            ofLine(mouseX,mouseY,mouseX,mouseY-12);
            ofLine(mouseX,mouseY-28,mouseX,mouseY-40);
            ofLine(mouseX-8,mouseY-20,mouseX-20,mouseY-20);
            ofLine(mouseX+8,mouseY-20,mouseX+20,mouseY-20);
            ofPopStyle();
        }
    }
	
	
	
    if (bShowText){
       drawText();
    }
    
    if (bShowLargeCamImageOnTop){
        ofSetColor(255);
        processFrameImg.draw(0,0,1024,768);
    }
	
	//ofSetColor(255);
	//drawMat(leapFboMat, mouseX, mouseY, 320,240);
	
	bool bDrawMiniImages = true;
	if (bDrawMiniImages) {
		
		ofPushMatrix();
		float miniScale = 0.15;
		ofTranslate(0, ofGetHeight() - (miniScale * imgH));
		ofScale(miniScale, miniScale);
		
		int xItem = 0;
		ofSetColor(ofColor::white);
		drawMat(grayMat,				imgW * xItem, 0); xItem++;
		drawMat(thresholded,			imgW * xItem, 0); xItem++;
		drawMat(adaptiveThreshImg,		imgW * xItem, 0); xItem++;
		drawMat(thresholdedFinal,		imgW * xItem, 0); xItem++;
		drawMat(edgesMat1,				imgW * xItem, 0); xItem++;
		drawMat(leapDiagnosticFboMat,	imgW * xItem, 0); xItem++;
		drawMat(coloredBinarizedImg,	imgW * xItem, 0); xItem++;
		
		ofSetColor(ofColor::orange);
		for (int i=0; i<6; i++){
			ofDrawBitmapString( ofToString(i+1), imgW*i +5, 15/miniScale);
		}
		
		ofPopMatrix();
	}
	
	

	//-----------------------------------
	float sca = (bShowAnalysisBig) ? 2.0 : 1.0;
	float insetX = (ofGetWidth() - sca*imgW);
	float insetY = (ofGetHeight()- sca*imgH);
	ofPushMatrix();
	ofPushStyle();
	{
		ofTranslate (insetX, insetY);
		ofScale(sca, sca);
		
		// Draw the contour analyzer and associated CV images.
		if (bDrawContourAnalyzer){
			ofSetColor(ofColor::white);
			switch(whichImageToDraw){
				default:
				case 1:		drawMat(grayMat,				0,0, imgW,imgH);	break;
				case 2:		drawMat(thresholded,			0,0, imgW,imgH);	break;
				case 3:		drawMat(adaptiveThreshImg,		0,0, imgW,imgH);	break;
				case 4:		drawMat(thresholdedFinal,		0,0, imgW,imgH);	break;
				case 5:		drawMat(edgesMat1,				0,0, imgW,imgH);	break;
				case 6:		drawMat(leapDiagnosticFboMat,	0,0, imgW,imgH);	break;
				case 7:		drawMat(coloredBinarizedImg,	0,0, imgW,imgH);	break;
				case 8:		if(bInPlaybackMode ){
								video.draw(0, 0, imgW,imgH);
							} else {
								processFrameImg.draw(0,0,imgW,imgH);
							}
							break;
			}
			myHandContourAnalyzer.draw();
		}
		
		/*
		// Draw the actual puppet mesh.
		ofPushStyle();
		{
			// Bind the texture
			if(bInPlaybackMode ){
				video.getTextureReference().bind();
			} else {
				processFrameImg.getTextureReference().bind();
			}
			
			// DRAW THE MESH
			myHandMeshBuilder.drawMesh();
			//ofSetColor(ofColor::white);
			//puppet.drawFaces();
			
			// Unbind the texture
			if(bInPlaybackMode ){
				video.getTextureReference().unbind();
			} else {
				processFrameImg.getTextureReference().unbind();
			}
			
			// DRAW THE MESH WIREFRAME
			// myHandMeshBuilder.drawMeshWireframe();
		}
		ofPopStyle();
		*/

	}
	ofPopStyle();
	ofPopMatrix();
    
	
	bool bDrawMeshBuilderWireframe = false;
	if (bDrawMeshBuilderWireframe){
		ofPushMatrix()  ;
		ofScale (2,2);
		ofTranslate(mouseX,-50);
		//myHandMeshBuilder.drawRefinedMesh();
		myHandMeshBuilder.drawMeshWireframe();
		ofPopMatrix();
	}
	
	
	//-----------------------------------
	bool bDrawMouseOrientationProbe = false;
	if (bDrawMouseOrientationProbe){
		int px = mouseX - insetX;
		int py = mouseY - insetY;
		if ((px > 0) && (px < imgW) &&
			(py > 0) && (py < imgH)){
			
			int index1 = py * imgW + px;
			int index3 = index1 * 3;
			
			unsigned char *pixels = leapDiagnosticFboMat.data;
			float pr = (float) pixels[index3+0];
			float pg = (float) pixels[index3+1];
			float pb = (float) pixels[index3+2];
			
			float orientation = leapVisualizer.getDiagnosticOrientationFromColor(pr,pg,pb);
			
			float angX = 60.0 * cos(orientation);
			float angY = 60.0 * sin(orientation);
			
			ofSetColor(255);
			ofLine (mouseX, mouseY, mouseX+angX, mouseY+angY);
			ofDrawBitmapString( ofToString( RAD_TO_DEG*orientation), mouseX, mouseY-10);
		}
	}
	
	//---------------------------
	// COMPUTE AND DISPLAY PUPPET
	if (bComputeAndDisplayPuppet){

		// Get the texture from the camera or the stored video, depending on the playback mode.
		ofTexture &handImageTexture = (bInPlaybackMode)? (video.getTextureReference()) : (processFrameImg.getTextureReference());
		ofPushMatrix();

		// Position the right edge of the puppet at the right edge of the window.
		if (bWorkAtHalfScale){
			ofTranslate( ofGetWindowWidth() - imgW*2.0, 0, 0);
			ofScale (2.0,2.0);
		}
		
		// Draw the puppet.
		myPuppetManager.drawPuppet(bComputeAndDisplayPuppet, handImageTexture);
		ofPopMatrix();
	}

		
}



//--------------------------------------------------------------
void ofApp::applicationStateMachine(){
	
	bool bHandJustInserted	= false;
	bool bHandJustDeparted	= false;
	long currentHandStartTime = 0;
	
	//--------------------------------------// What the system should do when:
	int		STATE_SHOWING_SCREENSAVER;		// Nobody has been around for a while: show a screensaver
	int		STATE_SHOWING_AUGMENTATION;		// Ideal operating conditions: show the processed hand
	int		STATE_SHOWING_PLAIN_VIDEO;		// Momentarily faulty conditions: pass through the video
	int		STATE_SHOWING_ERROR;			// Sustained faulty conditions: show the video, plus a message
	
	/*
	int		FAULT_NOTHING_WRONG;			// Everything appears to be working normally
	int		FAULT_NO_USER_PRESENT_BRIEF;	// Hand may have been momentarily withdrawn, not present for ~1 second
	int		FAULT_NO_USER_PRESENT_LONG;		// Nobody is present in the camera nor leap, for more than ~10 seconds
	int		FAULT_LEAP_DROPPED_FRAME;		// There was a hand in the camera (as recently as a second ago), but there's a leap dropout.
	int		FAULT_NO_LEAP_HAND_TOO_SMALL;	// There's a handlike object in the camera, but it may be too small for the leap to work
	int		FAULT_NO_LEAP_OBJECT_PRESENT;	// Some bastard put something in the box
	int		FAULT_TOO_MANY_HANDS;			// There's more than one hand in view
	int		FAULT_HAND_TOO_FAST;			// The hand is moving too quickly
	int		FAULT_HAND_TOO_HIGH;			// The hand is physically too close to the cameras
	int		FAULT_HAND_TOO_CURLED;			// The hand is a fist or curled up, or has a curled finger
	int		FAULT_HAND_TOO_VERTICAL;		// The hand is turned away from the camera
	int		FAULT_HAND_NOT_DEEP_ENOUGH;		// THe hand needs to be inserted deeper into the box. 
	*/
	
	ApplicationFault theApplicationFault = FAULT_NOTHING_WRONG;
	
	
	float	insertionPercentage		= myHandContourAnalyzer.getInsertionPercentageOfHand(); // gets percentage of left side of blob.
	float	distanceOfBlobFromEntry	= myHandContourAnalyzer.getDistanceOfBlobFromEntry();	// gets percentage of right side of blob.
	int		nBlobsInScene			= myHandContourAnalyzer.getNumberOfBlobs();
	
	bool	bLeapIsConnected		= leap.isConnected();
	int		nLeapHandsInScene		= leapVisualizer.nLeapHandsInScene;
	
	//256, 0.00, 20.0, &amountOfLeapMotion01);
	//gui1->addValuePlotter("zHandExtent", 256, 0.00, 1.50, &zHandExtent);
	//gui1->addValuePlotter("amountOfFingerCurl01", 256, 0.00, 1.00, &amountOfFingerCurl01);
	
	
	
	
	//printf ("amountOfLeapMotion01 = %f\n", amountOfLeapMotion01);
	
	
	// int nBlobs = contourFinder.size()
	// things we need to know:
	bool bSomethingIsPresentInCamera;
	
	// high values of distanceOfBlobFromEntry indicate either (A) bad thresholding or (B) object in scene.
	
	// An object is in the scene:
	// -- does not move (is stationary)
	// -- no hand in leap
	// -- area is larger than minRecognizeableObject area
	// -- nBlobsInScene is > 0
	
	
}


/*
 Please insert your hand to begin.
 Stop je hand in het apparaat om te beginnen.
 
 Put your hand in this zone.
 Hou je hand in dit gebied.
 
 Touch the screen for a new scene.
 Raak het scherm aan voor een nieuwe scène.
 
 Oops! Try moving more slowly.
 Oeps! Probeer langzamer te bewegen.
 
 Oops! Try keeping your hand flat.
 Oeps! Probeer je hand recht te houden.
 
 Oops! Your hand is too high up.
 Oeps! Je houdt je hand te hoog.
 
 Hey! Just one hand at a time, please.
 Hee! Eén hand tegelijk alsjeblieft.
 
 Hey! Hold still for a moment, please.
 Hee! Hou je hand stil alsjeblieft.
 
 I'm sorry! Your hand might be too small :(
 Het spijt me! Je hand is waarschijnlijk te klein voor mij :(
 
 Try taking your hand out, and putting it in again.
 Probeer eens om je hand eruit te halen en deze er weer in te stoppen.
 
 Hey! Only hands, please.
 Hee! Alleen je hand alsjeblieft.
 
 Hey! That's not a hand.
 Hee! Dat is geen hand.

 */




//--------------------------------------------------------------
void ofApp::drawLiveForRecording(){
    
    // Draw the live (or pre-recorded) camera image underneath...
    ofSetColor(ofColor::white);
	#ifdef _USE_LIBDC_GRABBER
		processFrameImg.draw(drawW,0,drawW,drawH);
	#else
		processFrameImg.draw(drawW,0,drawW,drawH);
	#endif
    
    // Draw the Leap CGI visualization over top.
    drawLeapWorld();
}

//--------------------------------------------------------------
void ofApp::drawPlayback(){
    
    drawLeapWorld();
    
    // if calibrated and want to see view from projector
    if (leapToCameraCalibrator.calibrated && bShowCalibPoints && bUseVirtualProjector){
        ofPushMatrix();
        ofScale(drawW/(float)cameraWidth, drawH/(float)cameraHeight);
        leapToCameraCalibrator.drawImagePoints();
        ofPopMatrix();
    }
    
    if(bInPlaybackMode && !bRecording){
        ofPushStyle();
        ofSetColor(255);
        video.draw(drawW, 0,drawW,drawH);
        
        if(bInputMousePoints){
            indexRecorder.drawPointHistory(video.getCurrentFrameID() );
        }

        ofPopStyle();
    }
}

//--------------------------------------------------------------
void ofApp::drawLeapWorld(){
    
    // draw video underneath calibration to see alignment
    if (leapToCameraCalibrator.calibrated && bUseVirtualProjector){
        ofSetColor(255);
        if(bInPlaybackMode ){
            video.draw(0, 0, drawW,drawH);
        } else{
            #ifdef _USE_LIBDC_GRABBER
                processFrameImg.draw(0,0,drawW,drawH);
            #else
                processFrameImg.draw(0,0,drawW,drawH);
            #endif
        }
    }
    
    // start leapColorFbo
	leapVisualizer.bDrawDiagnosticColors = false;
    if (bUseFbo){
        leapColorFbo.begin();
        ofClear(0,0,0,0);
    }
    
    // start camera (either calibrated projection or easy cam)
    if (leapToCameraCalibrator.calibrated && bUseVirtualProjector){
        leapToCameraCalibrator.projector.beginAsCamera();
		
		//glEnable(GL_DEPTH);
		glEnable(GL_DEPTH_TEST);
		
    } else {
		glEnable(GL_DEPTH);
		glEnable(GL_DEPTH_TEST);
        cam.begin();
    }
    
    // draw grid
    ofSetColor(255);
    leapVisualizer.drawGrid();
    
    // draw world points
    if (leapToCameraCalibrator.calibrated && bShowCalibPoints){
        leapToCameraCalibrator.drawWorldPoints();
    }
    
    // draw leap from xml
    if (bInPlaybackMode && !bRecording){
        int nFrameTags = leapVisualizer.myXML.getNumTags("FRAME");
        if (nFrameTags > 0){
			
            playingFrame = video.getCurrentFrameID();
			int whichFrame = playingFrame;
			if (playingFrame > 0 && bShowOffsetByNFrames){
				whichFrame = (playingFrame - framesBackToPlay + nFrameTags)%nFrameTags;
			}
			
			ofFill();
            leapVisualizer.drawFrameFromXML(whichFrame);
			
            if( lastIndexVideoPos.x > 0 && lastIndexVideoPos.y > 0){
                ofSetColor(ofColor::red);
                ofDrawSphere(lastIndexLeapPos, 5.0f);
            }
        }
    } else{
        // draw live leap
		
		if (bShowOffsetByNFrames && (framesBackToPlay > 0)){
            int totalFramesSaved = prevLeapFrameRecorder.XML.getNumTags("FRAME");
            int whichFrame = totalFramesSaved - framesBackToPlay;
            if (whichFrame < 0) whichFrame = 0;
            leapVisualizer.drawFrameFromXML (whichFrame, prevLeapFrameRecorder.XML);
        } else {
			// Just show the live leap, without any frame delay
            leapVisualizer.drawFrame(leap);
        }

    }
    
    // end camera
	if (leapToCameraCalibrator.calibrated && bUseVirtualProjector){
        leapToCameraCalibrator.projector.endAsCamera();
		glDisable(GL_DEPTH_TEST);
    } else {
        // draw calibration projector for debugging
        if (leapToCameraCalibrator.calibrated){
            leapToCameraCalibrator.projector.draw();
        }
        cam.end();
		glDisable(GL_DEPTH_TEST);
		//glDisable(GL_DEPTH);
    }
    
    // end fbo
    if (bUseFbo){
        leapColorFbo.end();
    }
    
    //-------------------------- Display semitransparent leapColorFbo
    // if we are calibrated draw leapFbo with transparency to see overlay better
    if (leapToCameraCalibrator.calibrated) {
		ofSetColor(255,255,255,200);
	} else {
		ofSetColor(255,255,255,255);
	}
    
	// draw the fbo
	ofPushMatrix();
	if (bUseVirtualProjector){
		ofScale(1,-1,1);    // flip fbo
		leapColorFbo.draw(0,-drawH,drawW,drawH);
	} else {
		leapColorFbo.draw(0,0,drawW,drawH);
	}
	ofPopMatrix();

}

//--------------------------------------------------------------
void ofApp::drawText(){
	
	float textY = 500;
	
	ofSetColor(ofColor::white);
	//ofDrawBitmapString("Display, record & playback Leap 2.0 Controller data.", 20, textY); textY+=15;
	//ofDrawBitmapString("Built in openFrameworks by Golan Levin, golan@flong.com", 20, textY); textY+=15;
	//textY+=15;

	ofSetColor( (leap.isConnected()) ?  ofColor::green : ofColor(255,51,51)) ;
	ofDrawBitmapString( (leap.isConnected() ? "Leap is Connected!" : "Leap is NOT CONNECTED!"), 20, textY);
	ofDrawBitmapString( (leap.isConnected() ? "Leap is Connected!" : "Leap is NOT CONNECTED!"), 21, textY); textY+=15;
	textY+=15;
	
	ofSetColor(ofColor::white);
	ofDrawBitmapString("Press '1' to open playback recording from a directory", 20, textY); textY+=15;
    ofDrawBitmapString("Press '2' to apply calibration from a directory", 20, textY); textY+=15;
    ofDrawBitmapString("Press '3' to open calibration frames for calibration input", 20, textY); textY+=15;
    textY+=15;
    ofDrawBitmapString("Press 's' to toggle hand skeleton/cylinders", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'c' to restore easy-cam orientation", 20, textY); textY+=15;
	ofDrawBitmapString("Press 'g' to toggle grid", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'v' to toggle virtual projector", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'w' to toggle calibration points draw", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'C' to load current playback folder's calibration", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'm' to allow mouse input points", 20, textY); textY+=15;
    ofDrawBitmapString("Press 'left/right' to advance frame by frame", 20, textY); textY+=15;
    ofDrawBitmapString("Press '' (space) to pause/play", 20, textY); textY+=15;
	
	string usePrev = bShowOffsetByNFrames ? "on" : "off";
    ofDrawBitmapString("Press 'o' toggle use prev "+usePrev+" {} frames behind: "+ofToString(framesBackToPlay), 20, textY); textY+=15;

	textY+=15;
	
    textY = 500;
    int textX = 610;
    
    ofDrawBitmapString("All folders must be in SharedData/recordings", textX, textY); textY+=15;
    textY+=15;
    
    ofDrawBitmapString("Press 'l' to return to live mode", textX, textY); textY+=15;
    
    if (leap.isConnected()){
		ofDrawBitmapString("Press 'r' to toggle RECORDING", textX, textY); textY+=15;
        ofDrawBitmapString("Press 'R' to toggle RECORDING for CALIBRATION", textX, textY); textY+=15;
        ofDrawBitmapString("Press ' ' to record CALIBRATION frame", textX, textY); textY+=15;

	}
	if (leapVisualizer.myXML.getNumTags("FRAME") > 0){
		ofDrawBitmapString("Press ' ' to pause PLAYBACK",  textX, textY); textY+=15;
	}
	
	if (bInPlaybackMode){
		ofSetColor(ofColor::green);
		ofDrawBitmapString("PLAYING! " + ofToString(playingFrame), textX, textY); textY+=15;
	} else if (bRecording){
		ofSetColor(ofColor::red);
		ofDrawBitmapString("RECORDING! " + ofToString(leapRecorder.recordingFrameCount), textX, textY); textY+=15;
	}
    
    textY+=15;
    ofSetColor(ofColor::white);
    ofDrawBitmapString("Playback folder: "+folderName,  textX, textY); textY+=15;
    ofDrawBitmapString("Calibration file: "+leapToCameraCalibrator.dirNameLoaded,  textX, textY); textY+=15;

	
}

//--------------------------------------------------------------
void ofApp::loadPlaybackFromDialogForCalibration(){
    
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        
        string filePath = openFileResult.getName();
        folderName = filePath;
        loadAndPlayRecording(filePath);
        video.setPlaying(true);
        video.update();
        playing = false;
        indexRecorder.setup("recordings/"+folderName+"/calib","fingerCalibPts.xml");
        indexRecorder.setDrawOffsets(drawW,0,cameraWidth/drawW,cameraHeight/drawH);    }
}

//--------------------------------------------------------------
void ofApp::loadPlaybackFromDialog(){
    
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        
        string filePath = openFileResult.getName();
        folderName = filePath;
        loadAndPlayRecording(filePath);
        
    }
}

//--------------------------------------------------------------
void ofApp::loadCalibrationFromDialog(){
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        string filePath = openFileResult.getName();
        calibrateFromXML(filePath);
    }
}

//--------------------------------------------------------------
void ofApp::finishRecording(){
    
    bRecording = false;
    bEndRecording = false;
    
    ofFileDialogResult openFileResult= ofSystemSaveDialog(folderName,"Make a folder in SharedData/recordings:");
    
    if (openFileResult.bSuccess){
        folderName = openFileResult.getName();
        
        int totalImage = MIN(currentFrameNumber,imageSequence.size());
        for(int i = 0; i < totalImage; i++) {
            if(imageSequence[i].getWidth() == 0) break;
            ofSaveImage(imageSequence[i], "recordings/"+folderName+"/camera/"+ofToString(i, 3, '0') + ".jpg");
        }
        
        ofDirectory dir;
        dir.open("recordings/"+folderName+"/leap");
        if(!dir.exists())dir.createDirectory("recordings/"+folderName+"/leap");
        leapRecorder.endRecording("recordings/"+folderName+"/leap/leap.xml");
        
        loadAndPlayRecording(folderName);

        if(bRecordingForCalibration){
            playing = false;
        }

    }
    
    imageSequence.clear();
    imageSequence.resize(300);
	for(int i = 0; i < imageSequence.size(); i++) {
		imageSequence[i].allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
	}
    currentFrameImg.clear();
    currentFrameImg.allocate(cameraWidth,cameraHeight, OF_IMAGE_COLOR);
}

//--------------------------------------------------------------
void ofApp::loadAndPlayRecording(string folder){
    
    leapVisualizer.loadXmlFile("recordings/"+folder+"/leap/leap.xml");
    video.load("recordings/"+folder+"/camera");
    
    indexRecorder.setup("recordings/"+folder+"/calib","fingerCalibPts.xml");
    indexRecorder.setDrawOffsets(drawW,0,cameraWidth/drawW,cameraHeight/drawH);
    
    // open calibration if exists?
    
    bInPlaybackMode = true;
    playing = true;
    currentFrameNumber = 0;
}

//--------------------------------------------------------------
void ofApp::calibrateFromXML( string calibFolderName ){
    
    leapToCameraCalibrator.setup (cameraWidth, cameraHeight);
    leapToCameraCalibrator.loadFingerTipPoints("recordings/"+calibFolderName+"/calib/fingerCalibPts.xml");
	
    if (useCorrectedCam()){
        leapToCameraCalibrator.correctCameraPNP(myCalibration);
    } else{
        leapToCameraCalibrator.correctCamera();
    }
}

//--------------------------------------------------------------
bool ofApp::useCorrectedCam(){
	#ifdef _USE_CORRECTED_CAMERA
		if(bUseCorrectedCamera) return true;
		else return false;
	#else
		return false;
	#endif
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

	if ((key == 'r') || (key == 'R')){
		if (leap.isConnected()){
			// Toggle Recording.
			//reset so we don't store extra tags
			if (bRecording){
                bEndRecording = true;
            } else{
                bRecording = !bRecording;
                folderName = ofGetTimestampString();
				leapRecorder.startRecording();
                leapVisualizer.myXML.clear();
                bInPlaybackMode = false;
                currentFrameNumber = 0;
                if( key == 'R') bRecordingForCalibration = true;
                else bRecordingForCalibration = false;
			}

		}
	
	}
    
    switch(key){
        case OF_KEY_LEFT:
            if(video.isLoaded()) video.goToPrevious();
            break;
        case OF_KEY_RIGHT:
             if(video.isLoaded()) video.goToNext();
            break;
        case 'c':
            cam.reset();
            break;
        case 'C':
            calibrateFromXML(folderName);
            break;

        case 'g':
		case 'G':
			guiTabBar->toggleVisible();
			myPuppetManager.showPuppetGuis = !(myPuppetManager.showPuppetGuis);
			break;
        case 'l':
            if(bInPlaybackMode) bInPlaybackMode = false;
            break;
        case 'm':
            bInputMousePoints = !bInputMousePoints;
            break;
            
        case 'E':
            myHandMeshBuilder.getMesh().save("handmesh-" + ofToString(playingFrame) + ".ply");
            break; 
		
		case 'P':
        case 'p':
            bComputeAndDisplayPuppet = !bComputeAndDisplayPuppet;
            break;
        case 's':
            leapVisualizer.bDrawSimple = !leapVisualizer.bDrawSimple;
            break;
        case 'v':
            bUseVirtualProjector = !bUseVirtualProjector;
            break;
        case '1':
            loadPlaybackFromDialog();
            break;
        case '2':
            loadCalibrationFromDialog();
            bUseVirtualProjector = true;
            break;
        case '3':
            loadPlaybackFromDialogForCalibration();
            break;
        case 'w':
            bShowCalibPoints = !bShowCalibPoints;
            break;
        case ' ':
            if(bInPlaybackMode) playing = !playing;
            else if(bRecordingForCalibration) bRecordThisCalibFrame = true;
            break;
        case 'u':
            bUseCorrectedCamera = !bUseCorrectedCamera;
            break;
        case '0':
            bShowLargeCamImageOnTop = !bShowLargeCamImageOnTop;
            break;
			
		//case 'F':
        case 'f':
            bFullscreen = !bFullscreen;
            break;
			
        case '9':
            if(drawW == 640){
                drawW = 1024;
                drawH = 768;
                bShowText = false;
                
            } else {
                drawW = 640;
                drawH = 480;
                bShowText = true;
            }
            break;
		
		case '!':	whichImageToDraw = 1; break;
		case '@':	whichImageToDraw = 2; break;
		case '#':	whichImageToDraw = 3; break;
		case '$':	whichImageToDraw = 4; break;
		case '%':	whichImageToDraw = 5; break;
		case '^':	whichImageToDraw = 6; break;
		case '&':	whichImageToDraw = 7; break;
		case '*':	whichImageToDraw = 8; break;
		
		case 'o':
            bShowOffsetByNFrames= !bShowOffsetByNFrames;
            break;
        case '{':
            framesBackToPlay = (framesBackToPlay+maxNPrevLeapFrames -1)%maxNPrevLeapFrames;
			break;
        case '}':
			framesBackToPlay = (framesBackToPlay+maxNPrevLeapFrames +1)%maxNPrevLeapFrames;
            break;
		case '.':
			bShowAnalysisBig = !bShowAnalysisBig;
			break;
		case ',':
			bDrawContourAnalyzer = !bDrawContourAnalyzer;
			break;
            
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
    
    if(bInPlaybackMode && bInputMousePoints){
        if(x > indexRecorder.xOffset && y > indexRecorder.yOffset &&
           x < indexRecorder.xOffset+drawW && y < drawH){
            indexRecorder.recordPosition(x, y-20, leapVisualizer.getIndexFingertipFromXML(video.getCurrentFrameID()),video.getCurrentFrameID());
            lastIndexVideoPos.set(x,y-20);
            lastIndexLeapPos = leapVisualizer.getIndexFingertipFromXML(video.getCurrentFrameID());
        }
    }
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

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
	/* delete gui; */
	
	for(vector<ofxUICanvas *>::iterator it = guis.begin(); it != guis.end(); ++it){
        ofxUICanvas *g = *it;
        delete g;
    }
    delete guiTabBar;
}
