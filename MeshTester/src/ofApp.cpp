#include "ofApp.h"
#include "SharedUtils.h"


void updatePuppet(Skeleton* skeleton, ofxPuppet& puppet) {
	for(int i = 0; i < skeleton->size(); i++) {
		puppet.setControlPoint(skeleton->getControlIndex(i), skeleton->getPositionAbsolute(i));
	}
}

//--------------------------------------------------------------
void ofApp::setup(){

    scenes.push_back(new NoneScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new WaveScene(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new WiggleScene(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new WobbleScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new EqualizeScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new NorthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new MeanderScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new SinusoidalLengthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new MiddleDifferentLengthScene(&puppet, &handSkeleton, &immutableHandSkeleton));
	scenes.push_back(new StartrekScene(&puppet, &handSkeleton, &immutableHandSkeleton));
    scenes.push_back(new PropogatingWiggleScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new PulsatingPalmScene(&puppet, &palmSkeleton, &immutablePalmSkeleton));
    scenes.push_back(new RetractingFingersScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    scenes.push_back(new StraightenFingersScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    scenes.push_back(new SplayFingersScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new PropogatingWiggleScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    scenes.push_back(new SinusoidalWiggleScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    scenes.push_back(new GrowingMiddleFingerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    scenes.push_back(new PinkyPuppeteerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
	scenes.push_back(new FingerLengthPuppeteerScene(&puppet, &handWithFingertipsSkeleton, &immutableHandWithFingertipsSkeleton));
    scenes.push_back(new LissajousScene(&puppet, &threePointSkeleton, &immutableThreePointSkeleton));

    sharedSetup();
    string basePath = ofToDataPath("", true);
	ofSetDataPathRoot("../../../../../SharedData2013/"); // for Golan's machine. Don't delete, please!
    
    setupGui();
    
    hand.loadImage("hand/genericHandCenteredNew.jpg");
	mesh.load("hand/handmarksNew.ply");
    
    
    for (int i = 0; i < mesh.getNumVertices(); i++) {
		mesh.addTexCoord(mesh.getVertex(i));
	}
    
    //----------------------------
	// Set up the puppet.
	puppet.setup(mesh);
    
	//----------------------------
	// Set up all of the skeletons.
	previousSkeleton = NULL;
	currentSkeleton  = NULL;
    
    handSkeleton.setup(mesh);
	immutableHandSkeleton.setup(mesh);
    handWithFingertipsSkeleton.setup(mesh);
    immutableHandWithFingertipsSkeleton.setup(mesh);
    palmSkeleton.setup(mesh);
	immutablePalmSkeleton.setup(mesh);
	wristSpineSkeleton.setup(mesh);
	immutableWristSpineSkeleton.setup(mesh);
    threePointSkeleton.setup(mesh);
	immutableThreePointSkeleton.setup(mesh);
	
    // Initialize gui features
	mouseControl  = false;
	showImage     = true;
	showWireframe = true;
	showSkeleton  = true;
	frameBasedAnimation = false;
    
	showGuis = true;
    
    sceneRadio = 0;

}

void ofApp::setupGui() {
	// set up the guis for each scene
	for (int i=0; i < scenes.size(); i++) {
		sceneNames.push_back(scenes[i]->getName());
		sceneWithSkeletonNames.push_back(scenes[i]->getNameWithSkeleton());
		scenes[i]->setupGui();
		scenes[i]->setupMouseGui();
	}
}

//--------------------------------------------------------------
void ofApp::update(){
    
    handSkeleton.setup(mesh);
	immutableHandSkeleton.setup(mesh);
    
    // get the current scene
	int scene = sceneRadio;//getSelection(0);
    
    
    scenes[scene]->update();
	setSkeleton(scenes[scene]->getSkeleton());
    
	// update the puppet using the current scene's skeleton
	updatePuppet(currentSkeleton, puppet);
	puppet.update();
    
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(0);
    
    ofSetColor(255);
    
	if (showImage) {
		hand.bind();
		puppet.drawFaces();
		hand.unbind();
	}
	if(showWireframe) {
		puppet.drawWireframe();
		puppet.drawControlPoints();
	}
	if(showSkeleton) {
		currentSkeleton->draw();
	}
    
	int scene = sceneRadio;
	scenes[scene]->draw();
}

void ofApp::setSkeleton(Skeleton* skeleton) {
	if(skeleton != currentSkeleton) {
		previousSkeleton = currentSkeleton;
		currentSkeleton = skeleton;
		if(previousSkeleton != NULL) {
			vector<int>& previousControlIndices = previousSkeleton->getControlIndices();
			for(int i = 0; i < previousControlIndices.size(); i++) {
				puppet.removeControlPoint(previousControlIndices[i]);
			}
		}
		vector<int>& currentControlIndices = currentSkeleton->getControlIndices();
		for(int i = 0; i < currentControlIndices.size(); i++) {
			puppet.setControlPoint(currentControlIndices[i]);
		}
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
    switch(key){
        case OF_KEY_RIGHT :
            sceneRadio = (sceneRadio+1) % scenes.size();
            cout << "scene " << sceneRadio << endl;
            break;
        case OF_KEY_LEFT :
            sceneRadio -=1;
            if(sceneRadio<0)sceneRadio = scenes.size()-1;
            cout << "scene " << sceneRadio << endl;
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
