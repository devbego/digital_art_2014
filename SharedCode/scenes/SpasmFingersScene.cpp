#include "SpasmFingersScene.h"

SpasmFingersScene::SpasmFingersScene(ofxPuppet* puppet,
                                         HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                                         HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton) {
    Scene::Scene();
    Scene::setup("Spasm Fingers", "Spasm Fingers (Hand)",
                 puppet,
                 (Skeleton*)handWithFingertipsSkeleton,
                 (Skeleton*)immutableHandWithFingertipsSkeleton);
    
    spasmFingers.resize(5);
    
    for(int i = 0; i < 5; i++) {
        spasmFingers[i].initialize();
        spasmFingers[i].resetNextSpasmTime();
    }
    
    curlFingers = true;
}


void SpasmFingersScene::setupGui() {
	SpasmFingersScene::initializeGui();
    
    //this->gui->addSlider("springy", 0.00, 1.0,  &springs[i].springy);
    this->gui->addToggle("Curl fingers", &curlFingers);
    
    this->gui->autoSizeToFitWidgets();
}

void SpasmFingersScene::setupMouseGui() {
	SpasmFingersScene::initializeMouseGui();
}

void SpasmFingersScene::update() {
    HandWithFingertipsSkeleton* handWithFingertipsSkeleton = (HandWithFingertipsSkeleton*)this->skeleton;
    
    int base[] = {
        HandWithFingertipsSkeleton::PINKY_BASE,
        HandWithFingertipsSkeleton::RING_BASE,
        HandWithFingertipsSkeleton::MIDDLE_BASE,
        HandWithFingertipsSkeleton::INDEX_BASE,
        HandWithFingertipsSkeleton::THUMB_BASE
    };
    int mid[] = {
        HandWithFingertipsSkeleton::PINKY_MID,
        HandWithFingertipsSkeleton::RING_MID,
        HandWithFingertipsSkeleton::MIDDLE_MID,
        HandWithFingertipsSkeleton::INDEX_MID,
        HandWithFingertipsSkeleton::THUMB_MID
    };
    int top[] = {
        HandWithFingertipsSkeleton::PINKY_TOP,
        HandWithFingertipsSkeleton::RING_TOP,
        HandWithFingertipsSkeleton::MIDDLE_TOP,
        HandWithFingertipsSkeleton::INDEX_TOP,
        HandWithFingertipsSkeleton::THUMB_TOP
    };
    int tip[] = {
        HandWithFingertipsSkeleton::PINKY_TIP,
        HandWithFingertipsSkeleton::RING_TIP,
        HandWithFingertipsSkeleton::MIDDLE_TIP,
        HandWithFingertipsSkeleton::INDEX_TIP,
        HandWithFingertipsSkeleton::THUMB_TIP
    };
    
    for(int i = 0; i < 5; i++) {
        int joints[] = { base[i], mid[i], top[i], tip[i] };
        
        ofVec2f positions[] = {
            handWithFingertipsSkeleton->getPositionAbsolute(joints[0]),
            handWithFingertipsSkeleton->getPositionAbsolute(joints[1]),
            handWithFingertipsSkeleton->getPositionAbsolute(joints[2]),
            handWithFingertipsSkeleton->getPositionAbsolute(joints[3])
        };
        
        float lengths[] = {
            positions[0].distance(positions[1]),
            positions[1].distance(positions[2]),
            positions[2].distance(positions[3])
        };
        float angles[] = {
            // usual trig but add pi because the hand is facing the other way
            atan((positions[0].y-positions[1].y)/(positions[0].x-positions[1].x))+PI,
            atan((positions[1].y-positions[2].y)/(positions[1].x-positions[2].x))+PI,
            atan((positions[2].y-positions[3].y)/(positions[2].x-positions[3].x))+PI
        };
        
        spasmFingers[i].update();
        
        float angleToUse = spasmFingers[i].angle;
        if(spasmFingers[i].flipped) angleToUse = -angleToUse;
        
        handWithFingertipsSkeleton->setRotation(joints[0], angleToUse);
        handWithFingertipsSkeleton->setPosition(joints[0], ofVec2f(angleToUse, 0), false);
        if(curlFingers) {
            handWithFingertipsSkeleton->setRotation(joints[1], angleToUse);
            handWithFingertipsSkeleton->setRotation(joints[2], angleToUse);
        }
    }
}

void SpasmFingersScene::draw() {
    
}

void SpasmFingersScene::updateMouse(float mx, float my) { }