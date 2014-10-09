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
    
    // make five springs for five fingers
    springs.resize(15);
    nodesMids.resize(5);
    nodesTips.resize(5);
    nodesExtended.resize(5);
	nodesBase.resize(5);
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
    //int base [] = {HandSkeleton::PINKY_BASE, HandSkeleton::RING_BASE, HandSkeleton::MIDDLE_BASE, HandSkeleton::INDEX_BASE, HandSkeleton::THUMB_BASE};
    
    for( int i = 0; i < 5; i++){
        
        int indexTip = tip[i];
        int indexMid = mid[i];
        //int indexBase = base[i];
        
        ofPoint tipPoint = immutableSkeleton->getPositionAbsolute(indexTip);
        ofPoint midPoint = immutableSkeleton->getPositionAbsolute(indexMid);
        //ofPoint basePoint = immutableSkeleton->getPositionAbsolute(indexBase);

        nodesMids[i].setPosition(midPoint.x, midPoint.y);
        nodesMids[i].bFixed = true;
        
        //nodesBase[i].setPosition(basePoint.x,basePoint.y);
        //nodesBase[i].bFixed = true;
        
        ofVec2f dir = tipPoint-midPoint;
        dir.normalize();
        ofVec2f newPos = tipPoint + 50*dir;
        nodesExtended[i].setPosition(newPos.x, newPos.y);
        nodesExtended[i].bFixed = true;
		
        if(springs[i].len == 0 ){
            nodesTips[i].setPosition(tipPoint.x, tipPoint.y);
            //nodesMids[i].setPosition(midPoint.x, midPoint.y); //n
        }
    }
    
    
    for( int i = 0; i < 5; i++){
        
        int indexTip = tip[i];
        int indexMid = mid[i];
        //int indexBase = base[i];

        ofPoint tipPoint = immutableSkeleton->getPositionAbsolute(indexTip);
        ofPoint midPoint = immutableSkeleton->getPositionAbsolute(indexMid);
       // ofPoint basePoint = immutableSkeleton->getPositionAbsolute(indexBase);

        // set up spring
        if(springs[i].len == 0 ){
            
            ofVec2f lenDiff = tipPoint - midPoint;
            if(lenDiff.length() > 1){
                springs[i].setup(&nodesMids[i],&nodesTips[i], lenDiff.length());
                springs[i+5].setup(&nodesTips[i], &nodesExtended[i], 25);
                //springs[i+10].setup(&nodesBase[i], &nodesMids[i], 25);

            }
        }
    }
    
    for( int i = 0; i < 5; i++){
        if(springs[i].len > 0 ){
            
            nodesTips[i].update();
            nodesMids[i].update();
            nodesExtended[i].update();
            
            springs[i].update();
            springs[i+5].update();
            //springs[i+10].update();

            int indexTip = tip[i];
            handSkeleton->setPosition(indexTip, nodesTips[i].pos, true, true);
            //handSkeleton->setPosition(mid[i], nodesMids[i].pos, true, true);
        }
    }
    
    
}

void SpringFingerScene::draw() {
    
    /*
     for( int i = 0; i < nodesTips.size(); i++){
	 nodesTips[i].draw();
	 nodesMids[i].draw();
	 nodesExtended[i].draw();
	 }
	 
	 for( int i = 0; i < springs.size(); i++){
	 springs[i].draw();
	 }
	 */
    
}

void SpringFingerScene::updateMouse(float mx, float my) {
}
