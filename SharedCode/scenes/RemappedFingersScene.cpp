#include "RemappedFingersScene.h"

RemappedFingersScene::RemappedFingersScene(ofxPuppet* puppet,
                                           HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                                           HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton) {
    Scene::Scene();
    Scene::setup("Remapped Fingers", "Remapped Fingers (Hand)",
                 puppet,
                 (Skeleton*)handWithFingertipsSkeleton,
                 (Skeleton*)immutableHandWithFingertipsSkeleton);
    
    baseAngles[0] = 3.73;
    baseAngles[1] = 3.41;
    baseAngles[2] = 3.17;
    baseAngles[3] = 2.94;
    baseAngles[4] = 2.20;
    
    useRawAngles = false;
    
    addFingerRemapping(0, 1);
    addFingerRemapping(0, 2);
    addFingerRemapping(0, 3);
    addFingerRemapping(0, 4);
}

void RemappedFingersScene::addFingerRemapping(int s, int c) {
    sourceFingers.push_back(s);
    controlledFingers.push_back(c);
}

void RemappedFingersScene::setupGui() {
	RemappedFingersScene::initializeGui();
    
    this->gui->addToggle("Fingers follow absolute angles", &useRawAngles);
    
    this->gui->autoSizeToFitWidgets();
}

void RemappedFingersScene::setupMouseGui() {
	RemappedFingersScene::initializeMouseGui();
}

void RemappedFingersScene::update() {
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
            //usual trig but add pi because the hand's backwards
            atan((positions[0].y-positions[1].y)/(positions[0].x-positions[1].x))+PI,
            atan((positions[1].y-positions[2].y)/(positions[1].x-positions[2].x))+PI,
            atan((positions[2].y-positions[3].y)/(positions[2].x-positions[3].x))+PI
        };
        
        currentAngles[i] = angles[0];
        
        for(int fs = 0; fs < sourceFingers.size(); fs++) {
            
            int sourceFinger = sourceFingers[fs];
            int controlledFinger = controlledFingers[fs];
            
            if(i == controlledFinger) {
                float newAngle;
                
                if(useRawAngles) {
                    newAngle = currentAngles[sourceFinger];
                } else {
                    float angleDiff = currentAngles[sourceFinger] - baseAngles[sourceFinger];
                    newAngle = baseAngles[controlledFinger] + angleDiff;
                }
                
                int jointIndex = 0;
                if(i == 4) jointIndex = 1; // use second joint if it's the thumb. looks better
                
                handWithFingertipsSkeleton->setRotation(joints[jointIndex], newAngle*57.29, true);
            }
        }
    }
}

void RemappedFingersScene::draw() {
    
}

void RemappedFingersScene::updateMouse(float mx, float my) { }