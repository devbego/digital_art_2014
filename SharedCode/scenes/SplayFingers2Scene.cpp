#pragma once

#include "SplayFingers2Scene.h"

SplayFingers2Scene::SplayFingers2Scene(ofxPuppet* puppet, HandWithFingertipsSkeleton* handWithFingertipsSkeleton, HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton) {
	Scene::Scene();
	Scene::setup("Splay Fingers 2", "Splay Fingers 2 (Hand With Fingertips)", puppet, (Skeleton*)handWithFingertipsSkeleton, (Skeleton*)immutableHandWithFingertipsSkeleton);

	this->maxPalmAngleLeft = 60;
	this->maxPalmAngleRight = -60;
	this->maxBaseAngleLeft = 20;
	this->maxBaseAngleRight = -20;

	this->splayAxis = 192;
	this->maxAngle = 50;
    this->averageAngleOffset = 0;
}

//=================================================================
void SplayFingers2Scene::setupGui() {
	SplayFingers2Scene::initializeGui();

	this->gui->addSlider("Max Angle", 0, 90, &maxAngle);
	this->gui->addSpacer();

	this->gui->autoSizeToFitWidgets();
}

//=================================================================
void SplayFingers2Scene::setupMouseGui() {
	SplayFingers2Scene::initializeMouseGui();

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
void SplayFingers2Scene::update() {
	HandWithFingertipsSkeleton* handWithFingertipsSkeleton = (HandWithFingertipsSkeleton*)this->skeleton;

	int palm = HandWithFingertipsSkeleton::PALM;
	ofVec2f palmPos = handWithFingertipsSkeleton->getPositionAbsolute(palm);

    
    int tip[] = {HandWithFingertipsSkeleton::PINKY_TIP, HandWithFingertipsSkeleton::RING_TIP, HandWithFingertipsSkeleton::MIDDLE_TIP, HandWithFingertipsSkeleton::INDEX_TIP, HandWithFingertipsSkeleton::THUMB_TIP};
    int top[] = {HandWithFingertipsSkeleton::PINKY_TOP, HandWithFingertipsSkeleton::RING_TOP, HandWithFingertipsSkeleton::MIDDLE_TOP, HandWithFingertipsSkeleton::INDEX_TOP, HandWithFingertipsSkeleton::THUMB_TOP};
    int mid[] = {HandWithFingertipsSkeleton::PINKY_MID,	HandWithFingertipsSkeleton::RING_MID, HandWithFingertipsSkeleton::MIDDLE_MID, HandWithFingertipsSkeleton::INDEX_MID, HandWithFingertipsSkeleton::THUMB_MID};
    int base[] = {
        HandWithFingertipsSkeleton::PINKY_BASE,
        HandWithFingertipsSkeleton::RING_BASE,
        HandWithFingertipsSkeleton::MIDDLE_BASE,
        HandWithFingertipsSkeleton::INDEX_BASE,
        HandWithFingertipsSkeleton::THUMB_BASE};

    // have the angle offset based on a running average, for stability
    float angleOffset = ofMap (palmPos.x, 256,440, 1,0);
    angleOffset = ofClamp (angleOffset, 0,1);
    angleOffset = maxAngle * powf (angleOffset, 0.75);
    averageAngleOffset = 0.94*averageAngleOffset + 0.06* angleOffset;
    
    
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
        
        
        // compute the angleOffsetToUse for this finger.
        // The angleOffsetToUse is +/- based on the distance of the finger base to the splayaxis.
        float absDistanceFromSplayAxis = abs(basePos.y - splayAxis);
        float frac = ofClamp( (absDistanceFromSplayAxis / 10.0), 0, 1);
        if (basePos.y >= splayAxis) {
            angleOffsetToUse = -abs(averageAngleOffset) * frac;
        } else {
            angleOffsetToUse =  abs(averageAngleOffset) * frac;
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

//=================================================================
void SplayFingers2Scene::updateMouse(float mx, float my) {
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


void SplayFingers2Scene::draw() {
	//ofSetColor(255);
	//ofLine(0, splayAxis, ofGetWidth(),splayAxis);
}