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
    
    // 0 = finger base springs
    // 1 = finger mid springs
    // 2 = finger tip springs
    // this is a weird way of doing things, but it lets us use one
    // value for all springs on the same parts of each finger
    for(int i = 0; i < 3; i++) {
        springy[i]     = 0.7;
        damp[i]        = 0.8;
        toChildDamp[i] = 0.15;
        lockLinear[i]  = false;
        lockTorsion[i] = false;
        angleLimit[i]  = 6.28;
    }
}


void SpringFingers2Scene::setupGui() {
	SpringFingers2Scene::initializeGui();
    
    this->gui->addSlider("Springiness base", 0, 1, &springy[0]);
    this->gui->addSlider("Springiness mid",  0, 1, &springy[1]);
    this->gui->addSlider("Springiness tip",  0, 1, &springy[2]);
    
    this->gui->addSlider("Dampening base", 0, 1, &damp[0]);
    this->gui->addSlider("Dampening mid",  0, 1, &damp[1]);
    this->gui->addSlider("Dampening tip",  0, 1, &damp[2]);
    
    this->gui->addSlider("Child dampening base", 0, 1, &toChildDamp[0]);
    this->gui->addSlider("Child dampening mid",  0, 1, &toChildDamp[1]);
    this->gui->addSlider("Child dampening tip",  0, 1, &toChildDamp[2]);
    
    this->gui->addSlider("Angular limit base", 0, 50.28, &angleLimit[0]);
    this->gui->addSlider("Angular limit mid",  0, 50.28, &angleLimit[1]);
    this->gui->addSlider("Angular limit tip",  0, 50.28, &angleLimit[2]);
    
    this->gui->addToggle("Linear lock base", &lockLinear[0]);
    this->gui->addToggle("Linear lock mid", &lockLinear[1]);
    this->gui->addToggle("Linear lock tip", &lockLinear[2]);
    
    this->gui->addToggle("Torsion lock base", &lockTorsion[0]);
    this->gui->addToggle("Torsion lock mid", &lockTorsion[1]);
    this->gui->addToggle("Torsion lock tip", &lockTorsion[2]);
    
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
        
        // this is weird looking, but we're just setting
        // spring constants and other options for all three
        // springs on the current finger
        for(int s = 0; s<3; s+=1) {
            springs[i+s*5].setSpringValues(
                springy[s],
                damp[s],
                toChildDamp[s],
                angleLimit[s],
                lockLinear[s],
                lockTorsion[s]
            );
        }
        
        if(springs[i].initialized) {
            // update the three springs for the current finger i
            
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

void SpringFingers2Scene::draw() {}

void SpringFingers2Scene::updateMouse(float mx, float my) { }