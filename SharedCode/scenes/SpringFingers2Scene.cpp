#include "SpringFingers2Scene.h"

SpringFingers2Scene::SpringFingers2Scene(ofxPuppet* puppet,
                                         HandWithFingertipsSkeleton* handWithFingertipsSkeleton,
                                         HandWithFingertipsSkeleton* immutableHandWithFingertipsSkeleton) {
    Scene::Scene();
    Scene::setup("Spring Fingers 2", "Spring Fingers 2 (Hand)",
                 puppet,
                 (Skeleton*)handWithFingertipsSkeleton,
                 (Skeleton*)immutableHandWithFingertipsSkeleton);
    
    springs.resize(15);
    
    for(int i = 0; i < 15; i++) {
        springs[i].springy = 0.7;
        springs[i].damp = 0.8;
        springs[i].toChildDamp = 0.15;
        springs[i].lockAngle = false;
        springs[i].lockLength = false;
    }
}


void SpringFingers2Scene::setupGui() {
	SpringFingers2Scene::initializeGui();
    
    // way too many springs to have settings for every one of them in gui...
    // solution?
    for(int i = 0; i < 15; i+=3) {
        this->gui->addSlider("springy",			0.00, 1.0,  &springs[i].springy);
        this->gui->addSlider("damp",			0.00, 1.0,  &springs[i].damp);
        this->gui->addSlider("toChildDamp",		0.00, 1.0,  &springs[i].toChildDamp);
        this->gui->addToggle("lockAngle",                   &springs[i].lockAngle);
        this->gui->addToggle("lockLength",                  &springs[i].lockLength);
    }
    
    this->gui->autoSizeToFitWidgets();
}

void SpringFingers2Scene::setupMouseGui() {
	SpringFingers2Scene::initializeMouseGui();
}

void SpringFingers2Scene::update() {
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
        
        if(springs[i].initialized) {
            // update the three springs for the current finger i...and
            
            springs[i].update(positions[1], positions[0],
                              ofPoint(0,0),
                              lengths[0], angles[0]);
            
            springs[i+5].update(positions[2], springs[i].currentPosition,
                                springs[i].currentForce,
                                lengths[1], angles[1]);
            
            springs[i+10].update(positions[3], springs[i+5].currentPosition,
                                 springs[i+5].currentForce,
                                 lengths[2], angles[2]);
            
            // change the positions of the fingers after applying springy physics to them!
            
            handWithFingertipsSkeleton->setPosition(joints[1], springs[i].currentPosition);
            handWithFingertipsSkeleton->setPosition(joints[2], springs[i+5].currentPosition);
            handWithFingertipsSkeleton->setPosition(joints[3], springs[i+10].currentPosition);
        } else {
            // init springs if they're not initialized already...
            
            springs[i]   .initialize(positions[1]);
            springs[i+5] .initialize(positions[2]);
            springs[i+10].initialize(positions[3]);
        }
    }
}

void SpringFingers2Scene::draw() {
    
}

void SpringFingers2Scene::updateMouse(float mx, float my) { }