//
//  SpringFingerScene.cpp
//  HandArtwork
//
//  Created by chris on 6/10/14.
//
//

#include "SpringFingerScene.h"

SpringFingerScene::SpringFingerScene(ofxPuppet* puppet, HandSkeleton* handSkeleton, HandSkeleton* immutableHandSkeleton) {
    Scene::Scene();
    Scene::setup("Spring Fingers", "Spring Fingers (Hand)", puppet, (Skeleton*)handSkeleton, (Skeleton*)immutableHandSkeleton);
}


void SpringFingerScene::setupGui() {
	SpringFingerScene::initializeGui();
}

void SpringFingerScene::setupMouseGui() {
	SpringFingerScene::initializeMouseGui();
    
}

//==========================================================================
void SpringFingerScene::update() {
	HandSkeleton* handSkeleton = (HandSkeleton*)this->skeleton;
	HandSkeleton* immutableHandSkeleton = (HandSkeleton*)this->immutableSkeleton;
    
	int mid[] = {HandSkeleton::PINKY_MID, HandSkeleton::RING_MID, HandSkeleton::MIDDLE_MID, HandSkeleton::INDEX_MID, HandSkeleton::THUMB_MID};
	int tip[] = {HandSkeleton::PINKY_TIP, HandSkeleton::RING_TIP, HandSkeleton::MIDDLE_TIP, HandSkeleton::INDEX_TIP, HandSkeleton::THUMB_TIP};
}

