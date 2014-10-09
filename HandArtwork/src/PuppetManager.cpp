//
//  PuppetManager.cpp
//  HandArtwork
//
//  Created by GL on 9/26/14.
//
//

#include "PuppetManager.h"


//--------------------------------------------------------------
void PuppetManager::setupPuppeteer (HandMeshBuilder &myHandMeshBuilder){
	
    initialSceneID = 5;
    
	// Create all of the scenes

    scenes.push_back(new WiggleScene			(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new EqualizeScene			(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new NorthScene				(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new LissajousScene			(&puppet, &threePointSkeleton, &immutableThreePointSkeleton));
	scenes.push_back(new MeanderScene			(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new SinusoidalLengthScene	(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new PulsatingPalmScene		(&puppet, &palmSkeleton, &immutablePalmSkeleton));
	scenes.push_back(new GrowingMiddleFingerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new StartrekScene			(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new SplayFingersScene		(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new SpringFingerScene		(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new SplayFingers2Scene		(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	
    ///// scenes.push_back(new SinusoidalWiggleScene	(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    ///// scenes.push_back(new PropogatingWiggleScene	(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    ///// scenes.push_back(new PinkyPuppeteerScene	(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	///// scenes.push_back(new FingerLengthPuppeteerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	///// scenes.push_back(new RetractingFingersScene	(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    ///// scenes.push_back(new StraightenFingersScene	(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    ///// scenes.push_back(new TwitchScene			(&puppet, &handSkeleton, &immutableHandSkeleton));
    ///// scenes.push_back(new WaveScene				(&puppet, &handSkeleton, &immutableHandSkeleton));
    ///// scenes.push_back(new MiddleDifferentLengthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
   	///// scenes.push_back(new NoneScene				(&puppet, &handSkeleton, &immutableHandSkeleton));
    ///// scenes.push_back(new WobbleScene			(&puppet, &handSkeleton, &immutableHandSkeleton));
    
	myHandMeshBuilder.loadDefaultMesh();
	
	//--------------------
	// Set up the puppet.
	ofMesh &mesh = myHandMeshBuilder.getMesh();
	puppet.setup (mesh);
    
    // Cache the topology subdivision.
    butterflySubdivider.topology_start(mesh);
    butterflySubdivider.topology_subdivide_boundary(4);
    refinedMesh = butterflySubdivider.topology_end();
    
	
	//--------------------
	// Set up all of the skeletons.
	previousSkeleton = NULL;
	currentSkeleton  = NULL;
	
	handSkeleton.setup(mesh);
	handWithFingertipsSkeleton.setup(mesh);
	immutableHandWithFingertipsSkeleton.setup(mesh);
	immutableHandSkeleton.setup(mesh);
	threePointSkeleton.setup(mesh);
	immutableThreePointSkeleton.setup(mesh);
	palmSkeleton.setup(mesh);
	immutablePalmSkeleton.setup(mesh);
	wristSpineSkeleton.setup(mesh);
	immutableWristSpineSkeleton.setup(mesh);
	
	// set the initial skeleton
	setSkeleton(&handSkeleton);
	
	
	bPuppetMouseControl		= false;
	bShowPuppetTexture		= true;
	bShowPuppetWireframe	= false;
	bShowPuppetControlPoints= false;
	bShowPuppetSkeleton		= false;
	bShowPuppetMeshPoints	= false;
	
	frameBasedAnimation		= false;
	showPuppetGuis			= true;
	
	elapsedPuppetMicros		= 10000;
	elapsedPuppetMicrosInt	= 10000;
    
    
    bSwappingOut = false;
    bSwappingIn = false;
    sceneSwapPosition = 0.0f;
    swapCounter = 0;
    
    bInIdleMode = false;
    nextSceneId = 0;
    sceneStartTime = 0;
}


//--------------------------------------------------------------
void PuppetManager::setSkeleton(Skeleton* skeleton) {
	// The "skeleton" is a graph of control points that animate in related ways
	// (to propagate relative movements to subsequent joints, for example).
	// Here we inform the Puppet about its control points, from the current skeleton.
	
	if (skeleton != currentSkeleton) {
		previousSkeleton = currentSkeleton;
		currentSkeleton = skeleton;
		if (previousSkeleton != NULL) {
			vector<int>& previousControlIndices = previousSkeleton->getControlIndices();
			for (int i=0; i<previousControlIndices.size(); i++) {
				puppet.removeControlPoint(previousControlIndices[i]);
			}
		}
		vector<int>& currentControlIndices = currentSkeleton->getControlIndices();
		for (int i=0; i<currentControlIndices.size(); i++) {
			puppet.setControlPoint(currentControlIndices[i]);
		}
	}
}

//--------------------------------------------------------------
void PuppetManager::setupPuppetGui(){
	
	// set up the guis for each scene
	for (int i=0; i < scenes.size(); i++) {
		sceneNames.push_back(scenes[i]->getName());
		sceneWithSkeletonNames.push_back(scenes[i]->getNameWithSkeleton());
		scenes[i]->setupGui();
		scenes[i]->setupMouseGui();
	}
    
	// create the main gui
	puppetGui = new ofxUICanvas();
	puppetGui->addLabel("Puppet");
	puppetGui->addSpacer();
	puppetGui->addFPS();
	puppetGui->addSpacer();
	sceneRadio = puppetGui->addRadio("Scene", sceneNames);
	//sceneRadio = gui->addRadio("Scene", sceneWithSkeletonNames);
	
	puppetGui->addSpacer();
	puppetGui->addLabelToggle("Show Image",			&bShowPuppetTexture);
	puppetGui->addLabelToggle("Show Wireframe",		&bShowPuppetWireframe);
	puppetGui->addLabelToggle("Show Control Pts",	&bShowPuppetControlPoints);
	puppetGui->addLabelToggle("Show Skeleton",		&bShowPuppetSkeleton);
	// puppetGui->addLabelToggle("Show Mesh Points",	&bShowPuppetMeshPoints); // Boring
	// puppetGui->addLabelToggle("Mouse Control",		&bPuppetMouseControl); // No longer hooked up.
	// puppetGui->addLabelToggle("FrameBasedAnim",		&frameBasedAnimation); // Currently not hooked up.
	
	puppetGui->autoSizeToFitWidgets();
	puppetGui->setPosition(290, 10);
	
	// set the initial scene
    sceneRadio->getToggles()[initialSceneID]->setValue(true);
}

//--------------------------------------------------------------
void PuppetManager::guiEvent(ofxUIEventArgs &e) {
	string name = e.widget->getName();
	int kind = e.widget->getKind();
    string canvasParent = e.widget->getCanvasParent()->getName();
    // cout << canvasParent << endl;
}


int getRadioSelection(ofxUIRadio* radio) {
	vector<ofxUIToggle*> toggles = radio->getToggles();
	for (int i = 0; i < toggles.size(); i++) {
		if (toggles[i]->getValue()) {
			return i;
		}
	}
	return -1;
}

void PuppetManager::setScene( int sceneIndex ){
    
    vector<ofxUIToggle*> toggles = sceneRadio->getToggles();
    if(sceneIndex >= toggles.size()) return;
    
    for (int i = 0; i < toggles.size(); i++) {
        toggles[i]->setValue(false);
    }
    
    toggles[sceneIndex]->setValue(true);
}

void PuppetManager::animateSceneChange(int dir){
    bSwappingOut = true;
    bSwappingIn = false;
    swapCounter = 0;
    
    if(dir > 0)swapTarget = ofGetHeight();
    else swapTarget = -ofGetHeight()*.5;
    
    int currScene = getRadioSelection(sceneRadio);
    vector<ofxUIToggle*> toggles = sceneRadio->getToggles();

    if(dir == 1) currScene--;
    else if(dir == -1) currScene++;
    else if(dir == 0) currScene = ofRandom(1,toggles.size() );
    
    if(currScene >= toggles.size() ) currScene = 1;
    if(currScene == 0) currScene = toggles.size()-1;
    
    nextSceneId = currScene;
    
}

void PuppetManager::setNextScene(){
    
    setScene(nextSceneId);
    sceneStartTime = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void PuppetManager::updatePuppeteer (bool bComputeAndDisplayPuppet, HandMeshBuilder &myHandMeshBuilder){
	
	long long prePuppetMicros = ofGetElapsedTimeMicros();
	
	if (bComputeAndDisplayPuppet){
		
		// Ask the HandMeshBuilder whether it built the mesh without errors.
		bool bCalculatedMesh = myHandMeshBuilder.bCalculatedMesh;
		if (bCalculatedMesh){
			
			// If so, get the mesh, and set up the puppet with it.
			// printf("%d: PuppetManager getting mesh from myHandMeshBuilder\n", (int) ofGetElapsedTimeMillis());
			ofMesh &mesh = myHandMeshBuilder.getMesh();
			// printf("%d: PuppetManager setting up mesh in Puppet\n", (int) ofGetElapsedTimeMillis());
			puppet.setup (mesh);
			// printf("%d: PuppetManager set it up!\n", (int) ofGetElapsedTimeMillis());
			
			// Provide that mesh to the various skeletons.
			// This informs each skeleton about the baseline (untransformed) location of
			// certain landmarks on the hand, such as the puppet's control points.
			// For example, this tells the skeleton that the pinky-tip is located at (x,y).
			// (The skeleton then pushes these points around.)
			handWithFingertipsSkeleton.setup (mesh);
			immutableHandWithFingertipsSkeleton.setup (mesh);
			handSkeleton.setup (mesh);
			immutableHandSkeleton.setup (mesh);
			threePointSkeleton.setup (mesh);
			immutableThreePointSkeleton.setup (mesh);
			palmSkeleton.setup (mesh);
			immutablePalmSkeleton.setup (mesh);
			wristSpineSkeleton.setup (mesh);
			immutableWristSpineSkeleton.setup (mesh);
			
			// Get the current scene; turn off all other scenes.
			int scene = getRadioSelection(sceneRadio);
			
			scenes[scene]->turnOn();
			if (showPuppetGuis){
				scenes[scene]->turnOnGui();
			} else {
				scenes[scene]->turnOffGui();
			}
			
			for (int i=0; i < scenes.size(); i++) {
				if (i != scene){ scenes[i]->turnOff(); }
			}
			
			/*
			if (bPuppetMouseControl) {
				// turn on mouse gui for current scene
				if (!scenes[scene]->mouseGuiIsOn()) scenes[scene]->turnOnMouse();
				// update the mouse
				scenes[scene]->updateMouse(mouseX, mouseY);
			} else {
				scenes[scene]->turnOffMouse();
			}
			*/
			
			
			// Update the skeleton, which scaffolds the puppet's deformation,
			// by animating the displacements of the puppet's control points.
			scenes[scene]->update();
			setSkeleton(scenes[scene]->getSkeleton());
			
			// Update the puppet using the current scene's skeleton.
			// The (animating) skeleton sets displacements of the control points.
			for (int i = 0; i<currentSkeleton->size(); i++) {
				puppet.setControlPoint(currentSkeleton->getControlIndex(i),
									   currentSkeleton->getPositionAbsolute(i));
			}
			
			// The moment of truth: when the puppet is asked to update itself.
			// This involves SVD with Accelerate, etc. and is very computationally expensive.
			puppet.update();
			// printf("%d: Puppet successfully updated with new mesh\n", (int) ofGetElapsedTimeMillis());
			
		} else {
			
			// In updatePuppeteer(), when bCalculatedMesh is false but a hand is still present,
			// we should show the undistorted video hand instead.
			// printf("HandMeshBuilder unsuccessful at %d\n", (int)ofGetElapsedTimeMillis() );
		}
		
		puppetGui->setVisible(showPuppetGuis);
		
    } else {
		// Puppeteering is disabled, so turn off GUI's etc.
		setGuiVisibility (false);
		
		/*
		puppetGui->setVisible(false);
		if (!showPuppetGuis) {
			for (int i=0; i < scenes.size(); i++) {
				scenes[i]->turnOff();
				scenes[i]->turnOffGui();
				scenes[i]->turnOffMouse();
			}
		}
		*/
		
	}
	
	// Update measurement of CPU time consumption for Puppet.
	long long postPuppetMicros = ofGetElapsedTimeMicros();
	elapsedPuppetMicros = 0.8*elapsedPuppetMicros + 0.2*(float)(postPuppetMicros - prePuppetMicros);
	elapsedPuppetMicrosInt = (int) elapsedPuppetMicros;
    
    updateSceneSwapAnimation();
}

void PuppetManager::updateSceneSwapAnimation(){
    
        if(bSwappingOut){
        
        if(swapCounter < 1){
            swapCounter+=.1;
            float pos = ofLerp(0, swapTarget, powf(swapCounter,1.975));
            sceneSwapPosition = pos;
        }else{
            swapCounter = 0;
            bSwappingIn = true;
            bSwappingOut = false;
            setNextScene();
            if(swapTarget < 0 ) swapTarget = ofGetHeight();
            else swapTarget = -ofGetHeight()*.5;
        }
    }
    
    if(bSwappingIn){
        
        if(swapCounter < 1){
            swapCounter+=.1;
            swapCounter = MAX(swapCounter,0);
            float pos = ofLerp(swapTarget, 0, powf(swapCounter,.25));
            sceneSwapPosition = pos;
        }else{
            swapCounter = 0;
            sceneSwapPosition = 0;
            bSwappingIn = false;
        }
    }
}


void PuppetManager::setGuiVisibility (bool bShowGuis){
	
	showPuppetGuis = bShowGuis;
	puppetGui->setVisible(showPuppetGuis);
	
	if (!showPuppetGuis) {
		for (int i=0; i < scenes.size(); i++) {
			scenes[i]->turnOff();
			scenes[i]->turnOffGui();
			scenes[i]->turnOffMouse();
		}
	} else {
		;
	}
}

//--------------------------------------------------------------
void PuppetManager::drawPuppet (bool bComputeAndDisplayPuppet, ofTexture &handImageTexture ){
	
	if (bComputeAndDisplayPuppet){
		bool bUseSubdivision = true; // why of course!
		
        ofPushMatrix();
        ofTranslate(0,sceneSwapPosition);
        
		ofPushStyle();
		{
			
			// Draw the main puppet texture (the image of the user's hand)
			if (bShowPuppetTexture){
				
                if (bUseSubdivision){
					// Bryce's mesh subdivision makes an enormous difference in quality!
					ofMesh& puppetDeformedMesh = puppet.getDeformedMesh();
					butterflySubdivider.fixMesh (puppetDeformedMesh, refinedMesh);
					
					// However, the subdivision causes a (LIFO?) reordering of the vertices.
					// This causes the pinky to be drawn on top, instead of on the bottom...
					// TODO: Change butterflySubdivider so that it does not reverse triangle order. 
					
                    if(!bInIdleMode){
					handImageTexture.bind();
					ofSetColor(255);
					refinedMesh.drawFaces();
					handImageTexture.unbind();
                    }else{
                        ofSetColor(25,200,255,200);
                        refinedMesh.drawFaces();
                    }
                   // ofSetColor(255);
					//refinedMesh.drawFaces();
				
				} else {
					// Working, but with no mesh subdividing
					handImageTexture.bind();
					ofSetColor(255);
					puppet.drawFaces(); // original
					handImageTexture.unbind();
                }
			}
			
			//-------------------------
			if (bShowPuppetWireframe) {
				ofSetColor(255,255,255, 180);
				if (bUseSubdivision){
					refinedMesh.drawWireframe();
				} else {
					puppet.drawWireframe();
				}
			}
			
			//-------------------------
			if (bShowPuppetSkeleton) {
				currentSkeleton->draw();
			}
			if (bShowPuppetControlPoints){
				puppet.drawControlPoints();
			}
			if (bShowPuppetMeshPoints){
				ofPushMatrix();
				ofMesh cur = puppet.getDeformedMesh();
				cur.setMode(OF_PRIMITIVE_POINTS);
				ofSetColor(255);
				glPointSize(3);
				cur.clearIndices();
				cur.draw();
				ofPopMatrix();
			}
			
			//-------------------------
			int scene = getRadioSelection(sceneRadio);
			scenes[scene]->draw();
		}
		
		ofPopStyle();
        ofPopMatrix();
	}
}




