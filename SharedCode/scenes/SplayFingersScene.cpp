#pragma once

#include "SplayFingersScene.h"

SplayFingersScene::SplayFingersScene(ofxPuppet* puppet, HandWithFingertipsSkeleton* handWithFingertipsSkeleton, HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton) {
	Scene::Scene();
	Scene::setup("Splay Fingers", "Splay Fingers (Hand With Fingertips)", puppet, (Skeleton*)handWithFingertipsSkeleton, (Skeleton*)immutableHandWithFingertipsSkeleton);

	this->maxPalmAngleLeft = 60;
	this->maxPalmAngleRight = -60;
	this->maxBaseAngleLeft = 20;
	this->maxBaseAngleRight = -20;

	this->maxAngle = 50;
    this->averageAngleOffset = 0;
    this->effectStrength = 1.0;
    this->insertionFracRunningAvg = 0.5;
    this->angularSpreadRunningAvg = 60.0;
}

//=================================================================
void SplayFingersScene::setupGui() {
	SplayFingersScene::initializeGui();

	this->gui->addSlider("Max Angle", 0, 90, &maxAngle);
	this->gui->addSpacer();
    this->gui->addSlider("Effect Strength", 0, 3, &effectStrength);
    this->gui->addSpacer();

	this->gui->autoSizeToFitWidgets();
}

//=================================================================
void SplayFingersScene::setupMouseGui() {
	SplayFingersScene::initializeMouseGui();

	vector<string> mouseOptions;
	mouseOptions.push_back("Palm Position");
	mouseOptions.push_back("Palm Rotation");
	mouseOptions.push_back("Finger Base Rotation");
	this->mouseRadio = this->mouseGui->addRadio("Mouse Control Options", mouseOptions);
	this->mouseRadio->getToggles()[0]->setValue(true);
	this->mouseGui->addSpacer();

	this->mouseGui->autoSizeToFitWidgets();
}

//=================================================================
void SplayFingersScene::update() {
	HandWithFingertipsSkeleton* handWithFingertipsSkeleton = (HandWithFingertipsSkeleton*)this->skeleton;

	int palm = HandWithFingertipsSkeleton::PALM;
	ofVec2f palmPos = handWithFingertipsSkeleton->getPositionAbsolute(palm);
    
    
    
    
    
    
    // get range of finger splay: the maximum difference in angle between the thumb and pinky.
    // The angular spread seems to range from ~30-90 degrees.
    
    // pinky:
    ofVec2f pinkyPt0 = handWithFingertipsSkeleton->getPositionAbsolute(HandWithFingertipsSkeleton::PINKY_BASE);
    ofVec2f pinkyPt1 = handWithFingertipsSkeleton->getPositionAbsolute(HandWithFingertipsSkeleton::PINKY_MID);
    ofVec2f pinkyDir = pinkyPt1 - pinkyPt0;
    
    // thumb:
    ofVec2f thumbPt1 = handWithFingertipsSkeleton->getPositionAbsolute(HandWithFingertipsSkeleton::THUMB_MID);
    ofVec2f thumbPt2 = handWithFingertipsSkeleton->getPositionAbsolute(HandWithFingertipsSkeleton::THUMB_TOP);
    ofVec2f thumbDir = thumbPt2 - thumbPt1;
    
    float pinkyAngle = RAD_TO_DEG * atan2f ( pinkyDir.y, pinkyDir.x );
    float thumbAngle = RAD_TO_DEG * atan2f ( thumbDir.y, thumbDir.x );
    float angularSpread = (180.0 - abs(pinkyAngle)) + (180.0 - abs(thumbAngle));
    angularSpreadRunningAvg = 0.95*angularSpreadRunningAvg + 0.05*angularSpread;
    // printf ("angularSpreadRunningAvg = %f \n", angularSpreadRunningAvg);
    
    
    

	if (true) {
		int tip[] = {HandWithFingertipsSkeleton::PINKY_TIP, HandWithFingertipsSkeleton::RING_TIP, HandWithFingertipsSkeleton::MIDDLE_TIP, HandWithFingertipsSkeleton::INDEX_TIP, HandWithFingertipsSkeleton::THUMB_TIP};
		int top[] = {HandWithFingertipsSkeleton::PINKY_TOP, HandWithFingertipsSkeleton::RING_TOP, HandWithFingertipsSkeleton::MIDDLE_TOP, HandWithFingertipsSkeleton::INDEX_TOP, HandWithFingertipsSkeleton::THUMB_TOP};
		int mid[] = {HandWithFingertipsSkeleton::PINKY_MID,	HandWithFingertipsSkeleton::RING_MID, HandWithFingertipsSkeleton::MIDDLE_MID, HandWithFingertipsSkeleton::INDEX_MID, HandWithFingertipsSkeleton::THUMB_MID};
		int base[] = {
            HandWithFingertipsSkeleton::PINKY_BASE,
            HandWithFingertipsSkeleton::RING_BASE,
            HandWithFingertipsSkeleton::MIDDLE_BASE,
            HandWithFingertipsSkeleton::INDEX_BASE,
            HandWithFingertipsSkeleton::THUMB_BASE};

        // have the agnel offset based on a running average, for stability
		float angleOffset = ofMap (palmPos.x, 256,440, 1,0);
        angleOffset = ofClamp (angleOffset, 0,1);
        angleOffset = maxAngle * powf (angleOffset, 0.75);
        averageAngleOffset = 0.94*averageAngleOffset + 0.06* angleOffset;
        
        
        
        float insertionFrac = ofMap (palmPos.x, 256,440, 1,0);
        insertionFrac = ofClamp (insertionFrac, 0,1);
        insertionFrac = powf (insertionFrac, 1.0); // nullop
        insertionFracRunningAvg = 0.95*insertionFracRunningAvg + 0.05*insertionFrac; // 0...1
        
        
		int fingerCount = 5;
		for (int i=0; i < fingerCount; i++) {
			int joints[] = {base[i], mid[i], top[i], tip[i]};
            float angleOffsetToUse = averageAngleOffset;

			ofVec2f basePos = handWithFingertipsSkeleton->getPositionAbsolute(joints[0]);
            bool bDoThumb = (fingerCount > 4);
            if (bDoThumb && (i == 4)){
               basePos = handWithFingertipsSkeleton->getPositionAbsolute(joints[1]);
            }
            
        
			ofVec2f positions[] = {
                handWithFingertipsSkeleton->getPositionAbsolute(joints[0]),
                handWithFingertipsSkeleton->getPositionAbsolute(joints[1]),
                handWithFingertipsSkeleton->getPositionAbsolute(joints[2]),
                handWithFingertipsSkeleton->getPositionAbsolute(joints[3])};
			float lengths[] = {
                positions[0].distance(positions[1]),
                positions[1].distance(positions[2]),
                positions[2].distance(positions[3])};
            
			ofVec2f dir = positions[1] - positions[0];
            if (bDoThumb && (i == 4)){
                dir = positions[2] - positions[1];
            }
			dir.normalize();
            
        
            
            
            float dx = dir.x;
            float dy = dir.y;
            float fingerOrientationDegrees = RAD_TO_DEG * atan2f(dy,dx);
            
            float newFingerOrientationDegrees = fingerOrientationDegrees;
            if (fingerOrientationDegrees > 0){  // ~135 (at extreme)...180
                float amountToExaggerate = 180 - fingerOrientationDegrees;
                amountToExaggerate *= (1.0 + effectStrength);//*insertionFracRunningAvg);
                newFingerOrientationDegrees = 180 - amountToExaggerate;
                
            } else {                            // ~ -135 (at extreme)...-180
                float amountToExaggerate = fingerOrientationDegrees - (-180);
                amountToExaggerate *= (1.0 + effectStrength);//*insertionFracRunningAvg);
                newFingerOrientationDegrees = -180 + amountToExaggerate;
            }
            angleOffsetToUse = newFingerOrientationDegrees - fingerOrientationDegrees;
            if (bDoThumb && (i == 4)){
                angleOffsetToUse *= 0.75; // just a little less so for the thumb, please....looks better
            }

			int fingerPartCount = 3;
			for (int j=0; j<fingerPartCount; j++) {
				dir = dir.getRotated (angleOffsetToUse);
				dir.normalize();
				dir = dir * lengths[j];

				ofVec2f parent = handWithFingertipsSkeleton->getPositionAbsolute(joints[j]);

				handWithFingertipsSkeleton->setPosition(joints[j+1], parent, true, false);
				handWithFingertipsSkeleton->setPosition(joints[j+1], dir,   false, false);
			}
		}
	}
}

//=================================================================
void SplayFingersScene::updateMouse(float mx, float my) {
	ofVec2f mouse(mx, my);

	HandWithFingertipsSkeleton* handWithFingertipsSkeleton = (HandWithFingertipsSkeleton*)this->skeleton;
	HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton = (HandWithFingertipsSkeleton*)this->immutableSkeleton;

	ofVec2f xAxis(1, 0);

	const int fingerCount = 5;

	int wrist = HandWithFingertipsSkeleton::WRIST;
	int palm = HandWithFingertipsSkeleton::PALM;
	int base[] = {HandWithFingertipsSkeleton::THUMB_BASE, HandWithFingertipsSkeleton::INDEX_BASE, HandWithFingertipsSkeleton::MIDDLE_BASE, HandWithFingertipsSkeleton::RING_BASE, HandWithFingertipsSkeleton::PINKY_BASE};
	int mid[] = {HandWithFingertipsSkeleton::THUMB_MID, HandWithFingertipsSkeleton::INDEX_MID, HandWithFingertipsSkeleton::MIDDLE_MID, HandWithFingertipsSkeleton::RING_MID, HandWithFingertipsSkeleton::PINKY_MID};
	int top[] = {HandWithFingertipsSkeleton::THUMB_TIP, HandWithFingertipsSkeleton::INDEX_TIP, HandWithFingertipsSkeleton::MIDDLE_TIP, HandWithFingertipsSkeleton::RING_TIP, HandWithFingertipsSkeleton::PINKY_TIP};

	ofVec2f origWristPos = puppet->getOriginalMesh().getVertex(handWithFingertipsSkeleton->getControlIndex(wrist));
	ofVec2f origPalmPos = puppet->getOriginalMesh().getVertex(handWithFingertipsSkeleton->getControlIndex(palm));
	ofVec2f origBasePos[fingerCount]; 
	ofVec2f origMidPos[fingerCount]; 
	ofVec2f origTopPos[fingerCount]; 
	for (int i=0; i < fingerCount; i++) {
		origBasePos[i] = puppet->getOriginalMesh().getVertex(handWithFingertipsSkeleton->getControlIndex(base[i]));
		origMidPos[i] = puppet->getOriginalMesh().getVertex(handWithFingertipsSkeleton->getControlIndex(mid[i]));
		origTopPos[i] = puppet->getOriginalMesh().getVertex(handWithFingertipsSkeleton->getControlIndex(top[i]));
	}

	ofVec2f origPalmDir;
	ofVec2f origFingerDir;
	float curRot;
	float newRot;

	float correction = 0;
	float baseCorrection[] = {26.75, -3, 1.75, 7.75, 9.75};
	float midCorrection[] = {6.75, 2, -1.5, -1.75, -3.5};

	switch(getSelection(mouseRadio)) {
		case 0: // palm position
			handWithFingertipsSkeleton->setPosition(HandWithFingertipsSkeleton::PALM, mouse, true);
			immutableHandWithFingertipsSkeleton->setPosition(HandWithFingertipsSkeleton::PALM, mouse, true);
			break;
		case 1: // palm rotation
			origPalmDir = origPalmPos - origWristPos;
			
			curRot = origPalmDir.angle(xAxis);

			newRot;
			if (mx <= 384) {
				newRot = ofMap(mx, 0, 384, -(curRot+correction+maxPalmAngleLeft), -(curRot+correction));
			}
			else {
				newRot = ofMap(mx, 384, 768, -(curRot+correction), -(curRot+correction+maxPalmAngleRight));
			}

			handWithFingertipsSkeleton->setRotation(palm, newRot, true, false);
			immutableHandWithFingertipsSkeleton->setRotation(palm, newRot, true, false);
			break;
		case 2: // finger base rotation
			for (int i=0; i < fingerCount; i++) {
				origFingerDir = origBasePos[i] - origPalmPos;
				curRot = origFingerDir.angle(xAxis);

				if (mx <= 384) {
					newRot = ofMap(mx, 0, 384, -(curRot+baseCorrection[i]+maxBaseAngleLeft), -(curRot+baseCorrection[i]));
				}
				else {
					newRot = ofMap(mx, 384, 768, -(curRot+baseCorrection[i]), -(curRot+baseCorrection[i]+maxBaseAngleRight));
				}

				handWithFingertipsSkeleton->setRotation(base[i], newRot, true, false);
				immutableHandWithFingertipsSkeleton->setRotation(base[i], newRot, true, false);
			}
			break;
	}
}
void SplayFingersScene::draw() {
	//ofSetColor(255);
	//ofLine(0, splayAxis, ofGetWidth(),splayAxis);
}