#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
	
	originalAppDataPath = ofToDataPath("", true);
	
	
	string basePath = ofToDataPath("", true);
	ofSetDataPathRoot("../../../../../SharedData/");
	
	//------------------------
	// Load the video; obtain its dimensions.
	video.load("recordings/recording-kyle-01/camera/");
	ofPixels& pixels0 = video.getFrame(0);
	imgW = pixels0.getWidth();
	imgH = pixels0.getHeight();
	
	grayMat.create				(imgH, imgW, CV_8UC1);
	blurredGrayMat.create		(imgH, imgW, CV_8UC1);
	unsharpedMat.create			(imgH, imgW, CV_8UC1);
	thresholded.create			(imgH, imgW, CV_8UC1);
	thresholdedFinal.create		(imgH, imgW, CV_8UC1);
	
	adaptiveThreshImg.create	(imgH, imgW, CV_8UC1);
	thresholdConstMat.create	(imgH, imgW, CV_8UC1);
	goodEdgesImg.create			(imgH, imgW, CV_8UC1);
	edgeDetected.create			(imgH, imgW, CV_8UC1);
	tempGrayscaleMat1.create	(imgH, imgW, CV_8UC1);
	tempGrayscaleMat2.create	(imgH, imgW, CV_8UC1);
	
	graySmall.create			(imgH/4, imgW/4, CV_8UC1);
	blurredSmall.create			(imgH/4, imgW/4, CV_8UC1);
	edgesSmall.create			(imgH/4, imgW/4, CV_8UC1);
	
	thresholder = new tmVisThresholderC1 (imgW, imgH);
	
	//------------------------
	// Load the mask image; compute the maskROI and other info.
	maskImage.loadImage("recordings/recording-kyle-01/mask.png");
	computeMaskResources();
	
	
	//------------------------
	bVideoPlaying = true;
	bSecondToggler				= true;
	bUseRedChannelForLuminance	= true;
	bApplyMedianToLuminance		= false;
	bComputeUnsharpedLuminance  = true;
	bUseROIForFilters			= true;
	bDoAdaptiveThresholding		= true;
	bDoMorphologicalOps			= true;
	bDoLaplacianEdgeDetect		= true;
	
	//------------------------
	blurKernelSize				= 4.0;
	blurredStrengthWeight		= 0.07;
	thresholdValue				= 26;
	prevThresholdValue			= 0;
	luminanceMedianSize			= 5;
	minLaplaceEdgeStrength		= 184;
	edgeContourMinArea			= 128;
	
	
	laplaceDelta				= 100;
	laplaceSensitivity			= 0.025;
	
	
	minAllowableContourAreaAsAPercentOfImageSize = 0.05;
	maxAllowableContourAreaAsAPercentOfImageSize = 0.12; // approx 100000.0 / (768*1024);
	
	
	//------------------------
	whichImageToDraw			= 1;
	bDrawColorVideo				= false;
	bDrawFullScale				= false;
	elapsedMicros				= 10000;
	elapsedMicrosString			= ofToString((int)elapsedMicros);
	
	setupGui();
}


//--------------------------------------------------------------
void ofApp::setupGui() {
	gui = new ofxUICanvas();
	gui->loadSettings(originalAppDataPath + "HandSegmenterSettings.xml");
	
	gui->addLabel("Hand Segmentation");
	gui->addSpacer();
	gui->addFPS();
	gui->addValuePlotter("elapsedMicros", 256, 0, 30000, &elapsedMicros);

	gui->addSpacer();
	gui->addSlider("thresholdValue", 0.0, 64.0, &thresholdValue);
	gui->addSlider("blurKernelSize", 1, 63, &blurKernelSize);
	gui->addSlider("blurredStrengthWeight", 0.0, 1.0, &blurredStrengthWeight);
	gui->addSlider("minLaplaceEdgeStrength", 0, 255, &minLaplaceEdgeStrength);
	gui->addIntSlider ("edgeContourMinArea", 1, 1000, &edgeContourMinArea);
	gui->addSlider("laplaceDelta",	0,255, &laplaceDelta);
	gui->addSlider("laplaceSensitivity",0.0, 0.1, &laplaceSensitivity) ;
	
	gui->addSpacer();
	gui->addLabelToggle("bUseROIForFilters",			&bUseROIForFilters);
	gui->addLabelToggle("bUseRedChannelForLuminance",	&bUseRedChannelForLuminance);
	gui->addLabelToggle("bDoAdaptiveThresholding",		&bDoAdaptiveThresholding);
	gui->addLabelToggle("bDoMorphologicalOps",			&bDoMorphologicalOps);
	gui->addLabelToggle("bDoLaplacianEdgeDetect",		&bDoLaplacianEdgeDetect);
	
	
	gui->addSpacer();
	gui->addLabelToggle("bVideoPlaying",				&bVideoPlaying);
	gui->addLabelToggle("bDrawFullScale",				&bDrawFullScale);
	gui->addLabelToggle("bDrawColorVideo",				&bDrawColorVideo);
	
	gui->autoSizeToFitWidgets();
	gui->loadSettings(originalAppDataPath + "HandSegmenterSettings.xml");
	
}



//--------------------------------------------------------------
void ofApp::exit(){

    delete gui;
}

//--------------------------------------------------------------
void ofApp::guiEvent(ofxUIEventArgs &e){
	
}

//--------------------------------------------------------------
void ofApp::update(){
	bSecondToggler = ((ofGetElapsedTimeMillis()/1000)%2 == 0);
	
	video.setPlaying(bVideoPlaying);
	video.update();
	
	if(video.isFrameNew() || !bVideoPlaying) {
		
		double then = ofGetElapsedTimeMicros();
		
		extractLuminanceChannelFromVideoFrame();
		applyMaskToLuminanceImage();
		
		applyMedianFilterToLuminanceImage();
		applyMorphologicalOpsToLuminanceImage();
		
		
		
		/*
		computeUnsharpedLuminanceImage();
		unsigned char* bufferIn;
		unsigned char* bufferOut;
		bufferIn  = unsharpedMat.ptr();
		bufferOut = thresholder->threshold (bufferIn, THRESHOLD_METHOD_RECURSIVE_ISODATA, THRESH_CONTROL_STABILIZE, -32);
		thresholded = Mat(imgH, imgW, CV_8UC1, bufferOut);
		*/
		
		doAdaptiveThresholding();
		applyEdgeAmplification();
		
		
		
		//-----------------------------------------------------------
		// Extract the contour(s) of the binarized image, and FIND THE CONTOURS
		contourFinder.setMinAreaNorm( minAllowableContourAreaAsAPercentOfImageSize );
		contourFinder.setMaxAreaNorm( maxAllowableContourAreaAsAPercentOfImageSize );
		contourFinder.findContours(thresholdedFinal);
		
		// Find the index ID of the largest contour, which is most likely the hand.
		int indexOfHandContour = -1;
		float largestContourArea = 0;
		vector <ofPolyline> polylineContours = contourFinder.getPolylines();
		int nPolylineContours = polylineContours.size();
		for (int i=0; i<nPolylineContours; i++){
			//
			float contourArea = contourFinder.getContourArea(i);
			if (contourArea > largestContourArea){
				contourArea = largestContourArea;
				indexOfHandContour = i;
			}
		}
		bValidHandContourExists = false;
		if (indexOfHandContour != -1){
			bValidHandContourExists = true;
			handContourPolyline = contourFinder.getPolyline(indexOfHandContour);
			handContourCentroid = contourFinder.getCentroid(indexOfHandContour);
			//HCAAMB.process(handContourPolyline, handContourCentroid);
			
			
			
		} else {
			//HCAAMB.informThereIsNoHandPresent();
		}

		
		
		
		
		
		 
		double now  = ofGetElapsedTimeMicros();
		elapsedMicros = 0.875*elapsedMicros + 0.125*(now-then);
		elapsedMicrosString	= ofToString((int)elapsedMicros);
	}
	
	
}



//--------------------------------------------------------------
void ofApp::doAdaptiveThresholding(){
	
	
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
void ofApp::computeUnsharpedLuminanceImage(){
	
	// Unsharp mask the luminance image
	// See: http://opencv-code.com/quick-tips/sharpen-image-with-unsharp-mask/
	
	int kernelSize = 19;   // GUI
	double blurSigma = kernelSize * 1.0; // 0...1 GUI.
	float unsharpAmount = 1.0; // GUI
	
	// Only apply the costly blur function to a ROI, if that is selected.
	cv::Mat matToBlurSrc = (bUseROIForFilters) ? grayMat(maskROI)        : grayMat;
	cv::Mat matToBlurDst = (bUseROIForFilters) ? blurredGrayMat(maskROI) : blurredGrayMat;
	cv::Mat unsharpDst   = (bUseROIForFilters) ? unsharpedMat(maskROI)   : unsharpedMat;
	
	GaussianBlur (matToBlurSrc, matToBlurDst, cv::Size(kernelSize,kernelSize), blurSigma, blurSigma, BORDER_DEFAULT);
	addWeighted (matToBlurSrc, (1.0+unsharpAmount), matToBlurDst, (0.0-unsharpAmount), 0, unsharpDst);

}

//--------------------------------------------------------------
void ofApp::applyMaskToLuminanceImage(){
	// Clobber the luminance image with the "Mask" image,
	// which excludes everything except a relevant-area region.
	bitwise_and(maskMat, grayMat, grayMat);
}

//--------------------------------------------------------------
void ofApp::applyMedianFilterToLuminanceImage(){

	// Apply a median filter to the luminance Mat to remove noisy details.
	if (bApplyMedianToLuminance){
		int lumaMedianSize = forceOdd(luminanceMedianSize);
		
		// Only apply the costly median function to a ROI, if that is selected.
		cv::Mat matToFilter = (bUseROIForFilters) ? grayMat(maskROI) : grayMat;
		cv::medianBlur (matToFilter, matToFilter, lumaMedianSize);
	}
}

//--------------------------------------------------------------
void ofApp::applyMorphologicalOpsToLuminanceImage(){
	
	if (bDoMorphologicalOps){
		cv::Mat matToFilter = (bUseROIForFilters) ? grayMat(maskROI) : grayMat;

		int morph_size = 1;
		int morph_type = cv::MORPH_ELLIPSE;
		Mat morphElement = getStructuringElement( morph_type,
											cv::Size( 2*morph_size + 1, 2*morph_size+1 ),
											cv::Point(  morph_size,       morph_size ) );
		
		// Apply the erosion operation
		cv::erode ( matToFilter, matToFilter, morphElement );
		cv::dilate( matToFilter, matToFilter, morphElement );
	}
}

//--------------------------------------------------------------
void ofApp::applyEdgeAmplification(){
		
	if (bDoLaplacianEdgeDetect){
		

		cv::Mat srcForLaplacian = (bUseROIForFilters) ? grayMat(maskROI)			: grayMat;
		cv::Mat dstForLaplacian = (bUseROIForFilters) ? edgeDetected(maskROI)		: edgeDetected;
		cv::Mat matTemp1		= (bUseROIForFilters) ? tempGrayscaleMat1(maskROI)	: tempGrayscaleMat1;
		cv::Mat matTemp2        = (bUseROIForFilters) ? tempGrayscaleMat2(maskROI)	: tempGrayscaleMat2;
		cv::Mat matGoodEdges	= (bUseROIForFilters) ? goodEdgesImg(maskROI)		: goodEdgesImg;
		cv::Mat matThresholded	= (bUseROIForFilters) ? thresholded(maskROI)		: thresholded;
		cv::Mat matFinal		= (bUseROIForFilters) ? thresholdedFinal(maskROI)	: thresholdedFinal;
		
		int		kSize = 7;
		double	delta = (double)laplaceDelta;
		double	sensitivity = (double)laplaceSensitivity;
		int		edgeThreshold = (int) minLaplaceEdgeStrength;
		
		cv::Laplacian (srcForLaplacian, dstForLaplacian, -1, kSize, (double)sensitivity, delta, cv::BORDER_DEFAULT );
		cv::threshold (dstForLaplacian, matTemp1, edgeThreshold, 255, cv::THRESH_BINARY);
		
		// Extract contours of the thresholded blobs (= "edges").
		// Exclusively render these edges into a new (temporary) image, tempGrayscaleMat2
		edgeContourFinder.setMinArea(edgeContourMinArea);
		edgeContourFinder.findContours(matTemp1);
		
		matTemp2.setTo((unsigned char) (255)); // make the mat white
		int nGoodEdges = edgeContourFinder.getContours().size();
		cv::Mat dstMat = toCv(matTemp2);
		for (int i=0; i<nGoodEdges; i++){
			vector <cv::Point> aGoodEdgeContour = edgeContourFinder.getContour(i);
			const cv::Point* ppt[1] = { &(aGoodEdgeContour[0]) };
			int npt[] = { aGoodEdgeContour.size() };
			fillPoly(matTemp2, ppt, npt, 1, Scalar(0)); // draw black blobs on it
		}
		
		// Erode tempGrayscaleMat2 into goodEdgesImg
		cv::Mat structuringElt = Mat();
		cv::erode (matTemp2, matGoodEdges, structuringElt);
		
		// Mask ('and') the good edge blobs against the thresholded image.
		cv::bitwise_and(matGoodEdges, matThresholded, matFinal);
	
		
	} else {
		thresholdedFinal = thresholded.clone();
	}
	
}

//--------------------------------------------------------------
void ofApp::extractLuminanceChannelFromVideoFrame(){
	
	//------------------------
	// Fetch the (color) video
	Mat videoMat = toCv(video);
	
	//------------------------
	// Extract (or compute) the grayscale luminance channel.
	// Either use the red channel (a good proxy for skin),
	// Or use the properly weighted components.
	if (bUseRedChannelForLuminance){
		vector<Mat> rgbChannelMats;
		split(videoMat, rgbChannelMats);
		grayMat = rgbChannelMats[0];
	} else {
		convertColor(videoMat, grayMat, CV_RGB2GRAY);
	}
}


//--------------------------------------------------------------
void ofApp::computeMaskResources(){
	// This method creates the maskMat (cv::Mat) from the loaded ofImage.
	// It also calculates the maskROI, which is used to optimize calculations.
	// We can speed up many calculations if we only compute in a ROI.
	
	// Init default (guessed) values.
	bLoadedMaskImage = false;
	maskROI = cv::Rect (0,0, 768,1024);
	maskROIQuarter = cv::Rect (0,0, 768/4,1024/4);
	
	// Checking if the mask image is loaded,
	if (maskImage.isAllocated()){
		bLoadedMaskImage = true;
		maskMat = toCv(maskImage);
		
		// Fetch mask dimensions
		int maskW = maskImage.getWidth();
		int maskH = maskImage.getHeight();
		int maskBpp = maskImage.bpp;
		
		// Make absolutely certain that mask is grayscale.
		if (maskBpp > 8){
			Mat maskMatTmp = toCv(maskImage);
			convertColor(maskMatTmp, maskMat, CV_RGB2GRAY);
		}
		
		// Initialize variables for Roi search.
		int maskRoiL = maskW;
		int maskRoiR = 0;
		int maskRoiT = maskH;
		int maskRoiB = 0;
		
		// Search through the array, seeking the boundaries of the ROI.
		// Assume the mask is a white trapezoid on a black background.
		unsigned char* maskChars = maskMat.ptr();
		int maskThreshold = 127;
		for (int y=0; y<maskH; y++){
			for (int x=0; x<maskW; x++){
				int index = (y*maskW + x);
				unsigned char val = maskChars[index];
				if (val > maskThreshold){
					if (x < maskRoiL){ maskRoiL = x; }
					if (x > maskRoiR){ maskRoiR = x; }
					if (y < maskRoiT){ maskRoiT = y; }
					if (y > maskRoiB){ maskRoiB = y; }
				}
			}
		}
		
		// If the calculated dimensions are kosher, use them.
		// Otherwise, set the ROI to the entire image size.
		if ((maskRoiL < maskRoiR) && (maskRoiT < maskRoiB)){
			int maskRoiW = maskRoiR - maskRoiL;
			int maskRoiH = maskRoiB - maskRoiT;
			maskROI = cv::Rect (maskRoiL, maskRoiT, maskRoiW, maskRoiH);
			maskROIQuarter = cv::Rect (maskRoiL/4, maskRoiT/4, maskRoiW/4, maskRoiH/4);
		} else {
			maskROI = cv::Rect (0,0, maskW, maskH);
			maskROIQuarter = cv::Rect (0,0, maskW/4, maskH/4);
		}
		
		

	}
}

//--------------------------------------------------------------
void ofApp::draw(){
	
	ofBackground(32);
	
	float margin = 20;
	float fitToWindowScale = (float)ofGetHeight()/(float)(imgH + margin*2);
	if (bDrawFullScale){ fitToWindowScale = 1.0; }
	gui->setPosition((imgW+margin*2)*fitToWindowScale, margin*fitToWindowScale);
	
	ofPushMatrix();
	ofScale(fitToWindowScale, fitToWindowScale);
	ofTranslate (margin,margin);
	ofPushStyle();
	
	/*
	 grayMat.create				(imgH, imgW, CV_8UC1);
	 blurredGrayMat .create		(imgH, imgW, CV_8UC1);
	 unsharpedMat .create			(imgH, imgW, CV_8UC1);
	 thresholded .create			(imgH, imgW, CV_8UC1);
	 thresholdedFinal .create		(imgH, imgW, CV_8UC1);
	 
	 adaptiveThreshImg .create	(imgH, imgW, CV_8UC1);
	 thresholdConstMat
	 goodEdgesImg
	 edgeDetected .create			(imgH, imgW, CV_8UC1);
	 tempGrayscaleMat1 .create	(imgH, imgW, CV_8UC1);
	 tempGrayscaleMat2 .create	(imgH, imgW, CV_8UC1);
	*/
	
	ofSetColor(ofColor::white);
	switch(whichImageToDraw){
		case 1:
		default:
			drawMat(grayMat,0,0);
			break;
		case 2:
			drawMat(thresholded,0,0);
			break;
		case 3:
			drawMat(adaptiveThreshImg,0,0);
			break;
		case 4:
			drawMat(goodEdgesImg,0,0);
			break;
		case 5:
			drawMat(thresholdedFinal,0,0);
			break;
	}
	
	
	if (bDrawColorVideo){
		ofEnableAlphaBlending();
		huntForBlendFunc(2000,0,2);
		ofSetColor(ofColor::white);
		video.draw(0, 0);
		ofDisableAlphaBlending();
	}
	
	if (bValidHandContourExists){
	ofSetColor(0,255,0);
	handContourPolyline.draw();
	}
	 
	

	// Draw boundary (white) around entire image.
	// Draw boundary (yellow) around ROI.
	ofNoFill();
	ofSetColor(255,255,255, 128);
	ofRect (0,0, imgW, imgH);
	ofSetColor(255,255,0, 128);
	ofRect (maskROI.x, maskROI.y, maskROI.width, maskROI.height);
	
	
	thresholder->renderHistogram(imgW+20, 10, 512, 200, false);
	
	ofPopStyle();
	ofPopMatrix();
	
	
	if(bDrawDiagnostics) {
		ofPushMatrix();
		float miniScale = 0.2;
		ofTranslate(0, ofGetHeight() - (miniScale * imgH));
		ofScale(miniScale, miniScale);
		
		int xItem = 0;
		drawMat(grayMat,			imgW * xItem, 0); xItem++;
		drawMat(thresholded,		imgW * xItem, 0); xItem++;
		drawMat(adaptiveThreshImg,	imgW * xItem, 0); xItem++;
		drawMat(goodEdgesImg,		imgW * xItem, 0); xItem++;
		drawMat(thresholdedFinal,	imgW * xItem, 0); xItem++;
		
		ofPopMatrix();
	}
	
	
	//ofSetColor(255,255,0);
	//ofDrawBitmapString ( "FPS: " + ofToString(ofGetFrameRate()), 20, 20);
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
	
	switch(key){
		case '1':
			whichImageToDraw = 1;
			break;
		case '2':
			whichImageToDraw = 2;
			break;
		case '3':
			whichImageToDraw = 3;
			break;
		case '4':
			whichImageToDraw = 4;
			break;
		case '5':
			whichImageToDraw = 5;
			break;
			
		case 'v':
		case 'V':
			bDrawDiagnostics = !bDrawDiagnostics;
			break;
		case 'S':
			gui->saveSettings(originalAppDataPath + "HandSegmenterSettings.xml");
			break;
		case 'G':
			gui->toggleVisible();
			break;
			
	}
	
	if(key == ' ') {
		bVideoPlaying = !bVideoPlaying;
	}
	if(key == OF_KEY_LEFT) {
		video.goToPrevious();
	}
	if(key == OF_KEY_RIGHT) {
		video.goToNext();
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

//--------------------------------------------------------------
void ofApp::huntForBlendFunc(int period, int defaultSid, int defaultDid){
	// sets all possible combinations of blend functions,
	// changing modes every [period] milliseconds.
    
    // All checked out, works well.
	
	int sfact[] = {
		GL_ZERO,
		GL_ONE,
		GL_DST_COLOR,
		GL_ONE_MINUS_DST_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA,
		GL_SRC_ALPHA_SATURATE
	};
	
	int dfact[] = {
		GL_ZERO,
		GL_ONE,
		GL_SRC_COLOR,
		GL_ONE_MINUS_SRC_COLOR,
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA,
		GL_DST_ALPHA,
		GL_ONE_MINUS_DST_ALPHA
	};
	
	
	
	glEnable(GL_BLEND);
	
	if ((defaultSid == -1) && (defaultDid == -1)) {
		
		int sid =  (ofGetElapsedTimeMillis()/(8*period))%9;
		int did =  (ofGetElapsedTimeMillis()/period)%8;
		glBlendFunc(sfact[sid], dfact[did]);
		ofLog(OF_LOG_NOTICE, "SRC %d	DST %d\n", sid, did);
		
	} else if (defaultDid == -1){
		
		int did =  (ofGetElapsedTimeMillis()/period)%8;
		glBlendFunc(sfact[defaultSid], dfact[did]);
		ofLog(OF_LOG_NOTICE, "SRC %d	DST %d\n", defaultSid, did);
		
	} else if (defaultSid == -1){
		
		int sid =  (ofGetElapsedTimeMillis()/(8*period))%9;
		glBlendFunc(sfact[sid], dfact[defaultDid]);
		ofLog(OF_LOG_NOTICE, "SRC %d	DST %d\n", sid, defaultDid);
		
	} else {
		
		glBlendFunc(sfact[defaultSid], dfact[defaultDid]);
		
	}
	
}




//double then = ofGetElapsedTimeMicros();
//double now  = ofGetElapsedTimeMicros();
//printf("Elapsed = %d\n", (int)(now-then));
