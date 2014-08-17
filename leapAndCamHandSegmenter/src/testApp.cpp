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


/*
 
 TODO
 
 use average orientation of the hand instead of bringing orientation towards zero
 detect folded hands
 detect hands moving too much
 detect hands oriented the wrong way. 
 
 composite laplacian edges.
 trace contour
 
 redesign (simplified) mesh for hands.
 
 */



//--------------------------------------------------------------
void testApp::setup(){
	
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
	bWorkAtHalfScale = true;
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
    bShowCalibPoints			= true;
    bRecordingForCalibration	= false;
    bRecordThisCalibFrame		= false;
    bUseCorrectedCamera			= true;
    bShowText					= true;
    bShowLargeCamImageOnTop		= false;    // temp for quickly showing on hand video only
	bUseVoronoiExpansion		= true;
	
	bShowOffBy1Frame			= false;
    framesBackToPlay			= 0;
	
	//--------------- Setup leap
	leap.open();
    leapVisualizer.setup();
	leapVisualizer.enableVoronoiRendering (imgW, imgH, bWorkAtHalfScale);
	leapVisualizer.bDrawGrid = false;
	
    leapRecorder.setup();
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

	blurKernelSize				= 4.0;
	blurredStrengthWeight		= 0.07;
	thresholdValue				= 26;
	prevThresholdValue			= 0;

	colorVideo.allocate			(cameraWidth, cameraHeight);
	colorVideoHalfScale.allocate(imgW, imgH);
	leapFboPixels.allocate		(imgW, imgH, OF_IMAGE_COLOR);
	
	
	
	grayMat.create				(imgH, imgW, CV_8UC1);
	blurred.create				(imgH, imgW, CV_8UC1);
	thresholded.create			(imgH, imgW, CV_8UC1);
	thresholdedFinal.create		(imgH, imgW, CV_8UC1);
	adaptiveThreshImg.create	(imgH, imgW, CV_8UC1);
	thresholdConstMat.create	(imgH, imgW, CV_8UC1);
	tempGrayscaleMat1.create	(imgH, imgW, CV_8UC1);
	tempGrayscaleMat2.create	(imgH, imgW, CV_8UC1);
	
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
	//-------------------------------------
	
	
	whichImageToDraw = 5;
	setupGui();
	
	// Get us ready to demo in a hurry
	string filePathCalib = "calib_chris_corrected_4";
	calibrateFromXML(filePathCalib);
	
	string filePathPlay = "play_chris_corrected_4";
	folderName = filePathPlay;
	loadAndPlayRecording(filePathPlay);
	bUseVirtualProjector = true;

	
	int morph_size = 1;
	int morph_type = cv::MORPH_ELLIPSE;
	morphStructuringElt = getStructuringElement(morph_type,
												cv::Size( 2*morph_size + 1, 2*morph_size+1 ),
												cv::Point(  morph_size,       morph_size ) );
	
}



//--------------------------------------------------------------
void testApp::initializeCamera(){
	
	//--------------- Setup camera grabber
	#ifdef _USE_LIBDC_GRABBER
		// For the ofxLibdc::PointGrey cameraLibdc;
		cout << "libdc cameras found: " << cameraLibdc.getCameraCount() << endl;
		
		cameraLibdc.setImageType(OF_IMAGE_COLOR);
		cameraLibdc.setSize (cameraWidth, cameraHeight);
		//cameraLibdc.setBayerMode(DC1394_COLOR_FILTER_GRBG); // why turns camera video grayscale???
		
		cameraLibdc.setup();
		cameraLibdc.setShutterAbs(1. / 31.0);
		cameraLibdc.setExposure(1.0);
		cameraLibdc.setBrightness(0);
		cameraLibdc.setGain(0);
		cameraLibdc.setGammaAbs(1);
		//cameraLibdc.setBlocking(true);
	
	#else
		cameraVidGrabber.setVerbose(true);
		cameraVidGrabber.initGrabber(cameraWidth,cameraHeight);
	#endif
}

//--------------------------------------------------------------
void testApp::initializeCameraCalibration(){
	
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

//--------------------------------------------------------------
void testApp::setupGui() {
	gui = new ofxUICanvas();
	gui->loadSettings(originalAppDataPath + "HandSegmenterSettings.xml");
	
	gui->addLabel("Hand Segmentation");
	gui->addSpacer();
	gui->addFPS();
	// gui->addValuePlotter("elapsedMicros", 256, 0, 30000, &elapsedMicros);
	
	gui->addSpacer();
	gui->addSlider("thresholdValue", 0.0, 64.0, &thresholdValue);
	gui->addSlider("blurKernelSize", 1, 63, &blurKernelSize);
	gui->addSlider("blurredStrengthWeight", 0.0, 1.0, &blurredStrengthWeight);
	
	gui->addSpacer();
	gui->addLabelToggle("bUseROIForFilters",			&bUseROIForFilters);
	gui->addLabelToggle("bUseRedChannelForLuminance",	&bUseRedChannelForLuminance);
	gui->addLabelToggle("bDoAdaptiveThresholding",		&bDoAdaptiveThresholding);
	gui->addLabelToggle("bDoMorphologicalOps",			&bDoMorphologicalOps);

	gui->autoSizeToFitWidgets();
	gui->loadSettings(originalAppDataPath + "HandSegmenterSettings.xml");
	
}



//--------------------------------------------------------------
void testApp::update(){
	
	updateBufferedVideoPlayback();
	
	
	// The leapVisualizer's "voronoi expansion" is a colored halo
	// that fills in unlabeled regions of the camera-based silhouette
	// that aren't covered by the geometrically-rendered LEAP hand.
	// The red/green channels indicate the local orientation of the finger
	// (which is used to suppress orthogonal creases), while the blue channel
	// indicates which finger is which (including joint information).
	if (bUseVoronoiExpansion){
		leapVisualizer.updateVoronoi();
	}
	
	updateProcessFrameImg();
	renderDiagnosticLeapFboAndExtractItsPixelData();
	
	
	// when live: use color img from processFrameImg
	// when playing, use color img from buffered video
	
	updateComputerVision();
	updateLeapHistoryRecorder();
	
}

//--------------------------------------------------------------
void testApp::updateLeapHistoryRecorder(){
	//------------- Store last frame
    if (prevLeapFrameRecorder.XML.getNumTags("FRAME") > 6){
        prevLeapFrameRecorder.XML.removeTag("FRAME",0);
    }
    prevLeapFrameRecorder.recordFrameXML(leap);
}

//--------------------------------------------------------------
void testApp::updateComputerVision(){
	
	// currentFrameImg comes directly from the camera, unless there's no camera, in which case it's just black & exists.
	// processedFrameImg is a copy of currentFrameImg if there's no undistortion being done; if it is, it's undistorted.
	// video is the playback buffered video

	if (bInPlaybackMode){
		extractLuminanceChannelFromVideoFrame();
		thresholdLuminanceImage();
		applyMorphologicalOps();
	}
}





//--------------------------------------------------------------
void testApp::extractLuminanceChannelFromVideoFrame(){
	
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
	
	//------------------------
	// Extract (or compute) the grayscale luminance channel.
	// Either use the red channel (a good proxy for skin),
	// Or use the properly weighted components.
	if (bUseRedChannelForLuminance){
		split(videoMat, rgbVideoChannelMats);
		grayMat = rgbVideoChannelMats[0];
	} else {
		convertColor(videoMat, grayMat, CV_RGB2GRAY);
	}

}




//--------------------------------------------------------------
void testApp::thresholdLuminanceImage(){
	
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
void testApp::applyMorphologicalOps(){
	
	if (bDoMorphologicalOps){
		
		// Apply the erosion operation
		cv::erode ( thresholded, thresholded,		morphStructuringElt );
		cv::erode ( thresholded, thresholded,		morphStructuringElt );
		cv::dilate( thresholded, thresholdedFinal,	morphStructuringElt );
	}
}



//--------------------------------------------------------------
void testApp::renderDiagnosticLeapFboAndExtractItsPixelData(){
	
	// For the purposes of computer vision, not display:
	// Render the Leap into leapDiagnosticFbo using diagnostic colors,
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
					if (playingFrame > 0 && bShowOffBy1Frame){
						whichFrame = (playingFrame - framesBackToPlay + nFrameTags)%nFrameTags;
					}
					
					
					// "Fluff out" the hand rendering with a voronoi-based halo
					// computed through OpenGL tricks on the graphics card. 
					if (bUseVoronoiExpansion){
						leapVisualizer.drawVoronoiFrameFromXML (whichFrame, leapVisualizer.myXML);
						leapToCameraCalibrator.projector.endAsCamera();
						glDisable(GL_DEPTH_TEST);
						leapVisualizer.drawVoronoi();
						glEnable(GL_DEPTH_TEST);
						leapToCameraCalibrator.projector.beginAsCamera();
					}
					
					// Render the actual CGI hand, on top of the voronoi diagram (using diagnostic colors)
					leapVisualizer.drawFrameFromXML(whichFrame);
				}
				
			} else { // LIVE LEAP
				
				// "Fluff out" the hand rendering with a voronoi-based halo
				// computed through OpenGL tricks on the graphics card.
				if (bUseVoronoiExpansion){
					if (bShowOffBy1Frame && (framesBackToPlay > 0)){
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
				}
				
				// draw live leap
				if (bShowOffBy1Frame && (framesBackToPlay > 0)){
					int totalFramesSaved = prevLeapFrameRecorder.XML.getNumTags("FRAME");
					int whichFrame = totalFramesSaved - framesBackToPlay;
					if (whichFrame < 0) whichFrame = 0;
					leapVisualizer.drawFrameFromXML (whichFrame, prevLeapFrameRecorder.XML);
				} else {
					leapVisualizer.drawFrame(leap); // no frame delay
				}
				
			}
			leapToCameraCalibrator.projector.endAsCamera();
		}

		glDisable(GL_DEPTH_TEST);
		leapDiagnosticFbo.end();
	}
	
	// Extract 8UC3 pixel data from leapDiagnosticFbo into leapFboPixels
	leapDiagnosticFbo.readToPixels(leapFboPixels);
	unsigned char *leapDiagnosticFboPixelData = leapFboPixels.getPixels();
	leapDiagnosticFboMat.data = leapDiagnosticFboPixelData;
	cv::flip(leapDiagnosticFboMat, leapDiagnosticFboMat, 0);
	
	
	// Composite the colored orientation image (in leapFboMat) against
	// the thresholdedFinal (in an RGBfied version), to produce the coloredBinarizedImg
	thresholdedFinalThrice[0] = thresholdedFinal;
	thresholdedFinalThrice[1] = thresholdedFinal;
	thresholdedFinalThrice[2] = thresholdedFinal;
	cv::merge(thresholdedFinalThrice, 3, thresholdedFinal8UC3);
	cv::bitwise_and(leapDiagnosticFboMat, thresholdedFinal8UC3, coloredBinarizedImg);
}





//--------------------------------------------------------------
void testApp::updateBufferedVideoPlayback(){
	
	//------------- Playback
	// Updates the BufferedVideo playback object, which fetches images from disk.
    if (bInPlaybackMode && !bRecording){
        video.update();
        video.setPlaying(playing);
    }
}

//--------------------------------------------------------------
void testApp::updateProcessFrameImg(){
	
	
    //------------- Frame grabbing.
	// Copy fresh camera data into currentFrameImg.
    bool bCameraHasNewFrame = false;
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
void testApp::draw(){

    ofBackground(20);
	gui->setPosition(20,20);
    
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
	
    if(bShowText){
       drawText();
    }
    
    if(bShowLargeCamImageOnTop){
        ofSetColor(255);
        processFrameImg.draw(0,0,1024,768);
    }
	
	//ofSetColor(255);
	//drawMat(leapFboMat, mouseX, mouseY, 320,240);
	
	
	if (true) {
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
		drawMat(leapDiagnosticFboMat,	imgW * xItem, 0); xItem++;
		drawMat(coloredBinarizedImg,	imgW * xItem, 0); xItem++;
		
		ofSetColor(ofColor::orange);
		for (int i=0; i<6; i++){
			ofDrawBitmapString( ofToString(i+1), imgW*i +5, 15/miniScale);
		}
		
		ofPopMatrix();
	}
	
	
	
	
	float insetX = ofGetWidth()-imgW;
	float insetY = ofGetHeight()-imgH;
	ofPushMatrix();
	ofTranslate (insetX, insetY);
	ofSetColor(ofColor::white);
	switch(whichImageToDraw){
		default:
		case 1:		drawMat(grayMat,				0,0, imgW,imgH);	break;
		case 2:		drawMat(thresholded,			0,0, imgW,imgH);	break;
		case 3:		drawMat(adaptiveThreshImg,		0,0, imgW,imgH);	break;
		case 4:		drawMat(thresholdedFinal,		0,0, imgW,imgH);	break;
		case 5:		drawMat(leapDiagnosticFboMat,	0,0, imgW,imgH);	break;
		case 6:		drawMat(coloredBinarizedImg,	0,0, imgW,imgH);	break;
		
	}
	ofPopMatrix();
	
	
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

//--------------------------------------------------------------
void testApp::drawLiveForRecording(){
    
    // draw live image
    ofSetColor(ofColor::white);
	#ifdef _USE_LIBDC_GRABBER
		processFrameImg.draw(drawW,0,drawW,drawH);
	#else
		processFrameImg.draw(drawW,0,drawW,drawH);
	#endif
            
    
    // draw leap
    drawLeapWorld();
    
}

//--------------------------------------------------------------
void testApp::drawPlayback(){
    
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

void testApp::drawLeapWorld(){
    
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
			if (playingFrame > 0 && bShowOffBy1Frame){
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
		
		if (bShowOffBy1Frame && (framesBackToPlay > 0)){
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
void testApp::drawText(){
	
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
	
	string usePrev = bShowOffBy1Frame ? "on" : "off";
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
		ofDrawBitmapString("Press 'p' to pause PLAYBACK",  textX, textY); textY+=15;
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
void testApp::loadPlaybackFromDialogForCalibration(){
    
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
void testApp::loadPlaybackFromDialog(){
    
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        
        string filePath = openFileResult.getName();
        folderName = filePath;
        loadAndPlayRecording(filePath);
        
    }
}

//--------------------------------------------------------------
void testApp::loadCalibrationFromDialog(){
    //Open the Open File Dialog
    ofFileDialogResult openFileResult= ofSystemLoadDialog("Choose a recording folder:",true);
    
    if (openFileResult.bSuccess){
        string filePath = openFileResult.getName();
        calibrateFromXML(filePath);
    }
}

//--------------------------------------------------------------
void testApp::finishRecording(){
    
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
void testApp::loadAndPlayRecording(string folder){
    
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
void testApp::calibrateFromXML( string calibFolderName ){
    
    leapToCameraCalibrator.setup (cameraWidth, cameraHeight);
    leapToCameraCalibrator.loadFingerTipPoints("recordings/"+calibFolderName+"/calib/fingerCalibPts.xml");
	
    if (useCorrectedCam()){
        leapToCameraCalibrator.correctCameraPNP(myCalibration);
    } else{
        leapToCameraCalibrator.correctCamera();
    }
}

//--------------------------------------------------------------
bool testApp::useCorrectedCam(){
	#ifdef _USE_CORRECTED_CAMERA
		if(bUseCorrectedCamera) return true;
		else return false;
	#else
		return false;
	#endif
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){

	if ((key == 'r') || (key == 'R')){
		if (leap.isConnected()){
			// Toggle Recording.
			//reset so we don't store extra tags
			if(bRecording){
                bEndRecording = true;
            }else{
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
        case 'F':
            bUseFbo = !bUseFbo;
            break;
        case 'g':
            leapVisualizer.bDrawGrid = !leapVisualizer.bDrawGrid;
            break;
        case 'l':
            if(bInPlaybackMode) bInPlaybackMode = false;
            break;
        case 'm':
            bInputMousePoints = !bInputMousePoints;
            break;
        case 'p':
            if(bInPlaybackMode) playing = !playing;
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
        case 'f':
            ofToggleFullscreen();
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
		
		
		case 'o':
            bShowOffBy1Frame= !bShowOffBy1Frame;
            break;
        case '{':
            framesBackToPlay = (framesBackToPlay+6 -1)%6;
			break;
        case '}':
			framesBackToPlay = (framesBackToPlay+6 +1)%6;
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
	delete gui;
}





