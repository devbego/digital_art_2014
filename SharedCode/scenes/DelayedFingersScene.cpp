#include "DelayedFingersScene.h"

DelayedFingersScene::DelayedFingersScene(ofxPuppet* puppet,
                                         HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                                         HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton) {
    Scene::Scene();
    Scene::setup("Delayed Fingers", "Delayed Fingers (Hand)",
                 puppet,
                 (Skeleton*)handWithFingertipsSkeleton,
                 (Skeleton*)immutableHandWithFingertipsSkeleton);
    
    // delay = 30 means delay by 1 second (30 fps)
    pinkyDelayListSize = 0;
    ringDelayListSize = 10;
    middleDelayListSize = 20;
    indexDelayListSize = 30;
    thumbDelayListSize = 30;
}


void DelayedFingersScene::setupGui() {
	DelayedFingersScene::initializeGui();
    
    this->gui->addIntSlider("pinkyDelay",  0, 60, &pinkyDelayListSize);
    this->gui->addIntSlider("ringDelay",   0, 60, &ringDelayListSize);
    this->gui->addIntSlider("middleDelay", 0, 60, &middleDelayListSize);
    this->gui->addIntSlider("indexDelay",  0, 60, &indexDelayListSize);
    this->gui->addIntSlider("thumbDelay",  0, 60, &thumbDelayListSize);
    
    this->gui->autoSizeToFitWidgets();
}

void DelayedFingersScene::setupMouseGui() {
	DelayedFingersScene::initializeMouseGui();
}

void DelayedFingersScene::update() {
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
            atan((positions[0].y-positions[1].y)/(positions[0].x-positions[1].x)),
            atan((positions[1].y-positions[2].y)/(positions[1].x-positions[2].x)),
            atan((positions[2].y-positions[3].y)/(positions[2].x-positions[3].x))
        };
        
        if(i == 0 && pinkyDelayListSize > 0) {
            delayFinger(angles[0],
                        &pinkyDelayList,
                        handWithFingertipsSkeleton,
                        joints[0],
                        pinkyDelayListSize);
        } else if(i == 1 && ringDelayListSize > 0) {
            delayFinger(angles[0],
                        &ringDelayList,
                        handWithFingertipsSkeleton,
                        joints[0],
                        ringDelayListSize);
        } else if(i == 2 && middleDelayListSize > 0) {
            delayFinger(angles[0],
                        &middleDelayList,
                        handWithFingertipsSkeleton,
                        joints[0],
                        middleDelayListSize);
        } else if(i == 3 && indexDelayListSize > 0) {
            delayFinger(angles[0],
                        &indexDelayList,
                        handWithFingertipsSkeleton,
                        joints[0],
                        indexDelayListSize);
        } else if(i == 4 && thumbDelayListSize > 0) {
            // thumb angle is a bit shaky. see what you can do about this...
            /*
            delayFinger(angles[0],
                        &thumbDelayList,
                        handWithFingertipsSkeleton,
                        joints[0],
                        thumbDelayListSize);
             */
        }
    }
}

void DelayedFingersScene::delayFinger(float newestAngle,
                                      list<float>* delayList,
                                      HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                                      int jointIndex,
                                      int delayListSize) {
    
    // put the most recent angle on the end of the list
    delayList->push_back(newestAngle);
    
    if(delayList->size() == delayListSize) {
        // if we have enough values to use, we take the oldest value off the list (which
        // is at the front) and we take it off the list.
        
        float angleToUse = (delayList->front())*57.29+180;//convert to degrees and flip
        handWithFingertipsSkeleton->setRotation(jointIndex, angleToUse, true);
        
        delayList->pop_front();
    } else {
        // we don't have enough values yet, so just use the latest value.
        
        float angleToUse = (delayList->back())*57.29+180;//convert to degrees and flip
        handWithFingertipsSkeleton->setRotation(jointIndex, angleToUse, true);
    }
    while(delayList->size() > delayListSize) {
        // this only happens if we change the max size: get rid of extra values
        
        delayList->pop_front();
    }
}

void DelayedFingersScene::draw() {
    
}

void DelayedFingersScene::updateMouse(float mx, float my) { }