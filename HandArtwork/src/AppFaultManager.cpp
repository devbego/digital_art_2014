//
//  appFaultManager.cpp
//  HandArtwork
//
//  Created by chris on 1/10/14.
//
//

#include "AppFaultManager.h"


void AppFaultManager::setup(){
    
    
    timeHasFault[FAULT_NOTHING_WRONG] = 0;
    timeHasFault[FAULT_NO_USER_PRESENT_BRIEF] = 0;
    timeHasFault[FAULT_NO_USER_PRESENT_LONG] = 0;
    timeHasFault[FAULT_LEAP_DROPPED_FRAME] = 0;
    timeHasFault[FAULT_NO_LEAP_HAND_TOO_SMALL] = 0;
    timeHasFault[FAULT_NO_LEAP_OBJECT_PRESENT] = 0;
    timeHasFault[FAULT_TOO_MANY_HANDS] = 0;
    timeHasFault[FAULT_HAND_TOO_FAST] = 0;
    timeHasFault[FAULT_HAND_TOO_HIGH] = 0;
    timeHasFault[FAULT_HAND_TOO_CURLED] = 0;
    timeHasFault[FAULT_HAND_TOO_VERTICAL] = 0;
    timeHasFault[FAULT_HAND_NOT_DEEP_ENOUGH] = 0;
    timeHasFault[FAULT_SAME_SCENE_TOO_LONG] = 0;

    
    timeLimit[FAULT_NOTHING_WRONG] = 0;
    timeLimit[FAULT_NO_USER_PRESENT_BRIEF] = 1;
    timeLimit[FAULT_NO_USER_PRESENT_LONG] = 5;
    timeLimit[FAULT_LEAP_DROPPED_FRAME] = 1;
    timeLimit[FAULT_NO_LEAP_HAND_TOO_SMALL] = 1;
    timeLimit[FAULT_NO_LEAP_OBJECT_PRESENT] = 1;
    timeLimit[FAULT_TOO_MANY_HANDS] = 1;
    timeLimit[FAULT_HAND_TOO_FAST] = 1;
    timeLimit[FAULT_HAND_TOO_HIGH] = 1;
    timeLimit[FAULT_HAND_TOO_CURLED] = 1;
    timeLimit[FAULT_HAND_TOO_VERTICAL] = 1;
    timeLimit[FAULT_HAND_NOT_DEEP_ENOUGH] = 1;
    timeLimit[FAULT_SAME_SCENE_TOO_LONG] = 1;
    
    int p = 0;
    priorityOrder[FAULT_NO_USER_PRESENT_LONG] = p; p++; // most important to show you need to interact
    priorityOrder[FAULT_HAND_TOO_HIGH] = p; p++;   // touching camera is not nice!
    priorityOrder[FAULT_HAND_NOT_DEEP_ENOUGH] = p; p++;    // typical mistake
    priorityOrder[FAULT_TOO_MANY_HANDS] = p; p++;
    priorityOrder[FAULT_HAND_TOO_FAST] = p; p++;
    priorityOrder[FAULT_HAND_TOO_VERTICAL] = p; p++;
    priorityOrder[FAULT_HAND_TOO_CURLED] = p; p++;
    priorityOrder[FAULT_NO_LEAP_HAND_TOO_SMALL] = p; p++;
    priorityOrder[FAULT_NO_LEAP_OBJECT_PRESENT] = p; p++;
    priorityOrder[FAULT_SAME_SCENE_TOO_LONG] = p; p++;
    priorityOrder[FAULT_LEAP_DROPPED_FRAME] = p; p++;
    priorityOrder[FAULT_NO_USER_PRESENT_BRIEF] = p; p++;
    priorityOrder[FAULT_NOTHING_WRONG] = p; p++;
    
    bShowingFault = false;
    
    font.loadFont("fonts/vagrblsb.ttf", 24);
    fontAlpha = 0;
    myFaultString = "";
    myFaultStringNL = "";
    shownFault = FAULT_NOTHING_WRONG;
    timeStartShowFault = 0.f;
    showFaultMode = 0;
    minTimeOn = 6;

}

void AppFaultManager::updateHasFault(ApplicationFault fault, float dt){
    timeHasFault[fault] += dt;
}

void AppFaultManager::updateResetFault( ApplicationFault fault){
    timeHasFault[fault] = 0;
}

bool AppFaultManager::getHasFault(ApplicationFault fault){
   
    if( timeHasFault[fault] > timeLimit[fault] ){
        return true;
    }
    
    return false;
}

ApplicationFault AppFaultManager::getMostImportantActiveFault(){
    
    ApplicationFault activeFault = FAULT_NOTHING_WRONG;
    int importance = priorityOrder.size()-1;
    
    typedef std::map<ApplicationFault,float>::iterator it_type;
    for(it_type iterator = timeHasFault.begin(); iterator != timeHasFault.end(); iterator++) {
        ApplicationFault fault = iterator->first;
        float timeOn = iterator->second;
        if( timeOn > timeLimit[fault] && priorityOrder[fault] < importance ){
            importance = priorityOrder[fault];
            activeFault = fault;
        }
    }
    
    return activeFault;
    
    
}

ApplicationFault AppFaultManager::getLongestFault(){
    
    ApplicationFault activeFault = FAULT_NOTHING_WRONG;
    float timeOn = 0;
    
    // find the fault on the longest and above its time limit
    typedef std::map<ApplicationFault,float>::iterator it_type;
    for(it_type iterator = timeHasFault.begin(); iterator != timeHasFault.end(); iterator++) {
        ApplicationFault fault = iterator->first;
        if( iterator->second > timeLimit[fault] && iterator->second > timeOn){
            activeFault = fault;
            timeOn = iterator->second;
        }
    }
    
    if( activeFault == FAULT_SAME_SCENE_TOO_LONG && getHasFault(FAULT_NO_USER_PRESENT_LONG)){
        activeFault = FAULT_NO_USER_PRESENT_LONG;
    }
    return activeFault;
}


float AppFaultManager::getDurationOfFault (ApplicationFault fault){

    float out = 0;
    // TODO: How do I get the duration that a certain fault has been active?
    // asking for timeHasFault[fault] always returns zero!
    
    //printf("%d  %f  ----     %d\n", fault, timeHasFault[fault], (int)ofGetElapsedTimeMillis());
    return out;
}


// get all detected faults
vector<ApplicationFault> AppFaultManager::getAllFaults(){
    
    vector<ApplicationFault> faults;
    typedef std::map<ApplicationFault,float>::iterator it_type;
    
    for(it_type iterator = timeHasFault.begin(); iterator != timeHasFault.end(); iterator++) {
        ApplicationFault fault = iterator->first;
        if( iterator->second > timeLimit[fault]){
            faults.push_back(fault);
        }
    }
    
    return faults;
    
}


//===================================================================
bool AppFaultManager::doCurrentFaultsIndicateLikelihoodOfBadMeshes(){
	
	ApplicationFault activeFault = FAULT_NOTHING_WRONG;
    
    bool bBadFaultIsHappening = false;
    
    // find the fault on the longest and above its time limit
    typedef std::map<ApplicationFault,float>::iterator it_type;
    for(it_type iterator = timeHasFault.begin(); iterator != timeHasFault.end(); iterator++) {
        ApplicationFault aFault = iterator->first;
		float timeFaultIsOn = iterator->second;
		if (timeFaultIsOn > 0){
			switch (aFault){
				case FAULT_LEAP_DROPPED_FRAME:
				case FAULT_NO_LEAP_OBJECT_PRESENT:
				case FAULT_TOO_MANY_HANDS:
				case FAULT_HAND_TOO_FAST:
				case FAULT_HAND_TOO_HIGH:
				case FAULT_HAND_TOO_CURLED:
				case FAULT_HAND_TOO_VERTICAL:
				case FAULT_HAND_NOT_DEEP_ENOUGH:
				case FAULT_NO_USER_PRESENT_BRIEF:
				case FAULT_NO_USER_PRESENT_LONG:
					bBadFaultIsHappening = true;
					break;
				
				default:
					bBadFaultIsHappening = false;
					break;
			}
		}
        
    }

	return bBadFaultIsHappening;
}



//===================================================================
void AppFaultManager::drawFaultHelpScreen(){
    
    // use priority to choose shown fault
    ApplicationFault activeFault = FAULT_NOTHING_WRONG;
    activeFault = getMostImportantActiveFault();
    float timeOn = timeHasFault[activeFault];
    
//    float timeOn = 0;
//    
//    // find the fault on the longest and above its time limit
//    typedef std::map<ApplicationFault,float>::iterator it_type;
//    for(it_type iterator = timeHasFault.begin(); iterator != timeHasFault.end(); iterator++) {
//        ApplicationFault fault = iterator->first;
//        if( iterator->second > timeLimit[fault] && iterator->second > timeOn){
//            activeFault = fault;
//            timeOn = iterator->second;
//        }
//    }
    
    //cout << "activeFault " << activeFault << endl;
    
   if(showFaultMode == 0){
        
       // start new
        switch (activeFault) {
            case FAULT_NOTHING_WRONG:
                myFaultString = "";
                myFaultStringNL = "";
                break;
            case FAULT_NO_USER_PRESENT_BRIEF:
                myFaultString = "Please insert your hand to begin.";
                myFaultStringNL = " Stop je hand in het apparaat om te beginnen.";
                break;
            
            case FAULT_NO_USER_PRESENT_LONG:
                myFaultString = "Please insert your hand to begin.";
                myFaultStringNL = "Stop je hand in het apparaat om te beginnen.";
                break;
            
            case FAULT_LEAP_DROPPED_FRAME:
                // TODO: need text for dropped frame
                myFaultString = "";
                myFaultStringNL = "";
                break;
            
            case FAULT_NO_LEAP_OBJECT_PRESENT:
                myFaultString = "Only hands, please.";
                myFaultStringNL = "Alleen je hand alsjeblieft.";
                break;
            
            case FAULT_TOO_MANY_HANDS:
                myFaultString = "Just one hand at a time, please.";
                myFaultStringNL = "E√©n hand tegelijk alsjeblieft.";
                break;
            
            case FAULT_HAND_TOO_FAST:
                myFaultString = "Try moving more slowly.";
                myFaultStringNL = "Probeer langzamer te bewegen.";
                break;
            
            case FAULT_HAND_TOO_HIGH:
                myFaultString = "Your hand is too high up.";
                myFaultStringNL = "Je houdt je hand te hoog.";
                break;
             
            case FAULT_HAND_TOO_CURLED:
                myFaultString = "Try keeping your hand flat.";
                myFaultStringNL = " Probeer je hand recht te houden.";
                break;
                
            case FAULT_HAND_TOO_VERTICAL:
                myFaultString = "Try keeping your hand flat.";
                myFaultStringNL = " Probeer je hand recht te houden.";
                break;
                
            case FAULT_HAND_NOT_DEEP_ENOUGH:
                // TODO: need text for hand not deep enough
                myFaultString = "";
                myFaultStringNL = "";
                break;
                
            case FAULT_NO_LEAP_HAND_TOO_SMALL:
                myFaultString = "I'm sorry! Your hand might be too small :(";
                myFaultStringNL = "Het spijt me! Je hand is waarschijnlijk te klein voor mij :(";
                break;
            
            case FAULT_SAME_SCENE_TOO_LONG:
                myFaultString = "Touch the screen for a new scene";
                myFaultStringNL = "Raak het scherm aan voor een nieuwe scene";
                break;
                
            default:
                break;
        }
        
       showFaultMode = 1;
       shownFault = activeFault;
       timeStartShowFault = ofGetElapsedTimef();
   }
    
    
    if(activeFault == FAULT_NOTHING_WRONG){
        timeStartShowFault = ofGetElapsedTimef()-(minTimeOn+1);
    }
    
    // update display mode
    if( shownFault != activeFault ){
        // we have a different fault now, turn this off if on for min time
        if( ofGetElapsedTimef() - timeStartShowFault > minTimeOn){
            showFaultMode = 2;
        }
    }else if( timeOn == 0 ){
        if( ofGetElapsedTimef() - timeStartShowFault > minTimeOn){
            showFaultMode = 2;
        }
    }
    
    // if alpha out, and mode is go off, set is off
    if( showFaultMode == 2 && fontAlpha <= 0){
        showFaultMode = 0;
    }
    
    // update alpha
    if(showFaultMode == 1 && fontAlpha < 1){
        fontAlpha += .1;
    }else if( showFaultMode == 2 && fontAlpha > 0){
        fontAlpha -= .1;
    }
    
    // make black out when hands are too high
    if(shownFault == FAULT_HAND_TOO_HIGH){
        ofPushStyle();
        ofSetColor(0,255*powf(fontAlpha,1.5));
        ofFill();
        ofRect(0,0,ofGetWidth(),ofGetHeight());
        ofPopStyle();
    }
    
    // draw to screen
    float stringWidth =  font.stringWidth(myFaultString);
    
    ofSetColor(255,255*powf(fontAlpha,1.5));
    ofPushMatrix();
        ofTranslate(50, ofGetHeight()/2+stringWidth/2.0);
        ofRotate(-90);
        font.drawString(myFaultString, 0,0);
    ofPopMatrix();
    
    stringWidth =  font.stringWidth(myFaultStringNL);
    
    ofSetColor(255,255*powf(fontAlpha,.75));
    ofPushMatrix();
    ofTranslate(100, ofGetHeight()/2+stringWidth/2.0);
    ofRotate(-90);
    font.drawString(myFaultStringNL, 0,0);
    ofPopMatrix();
    
    
}


/*
 Please insert your hand to begin.
 Stop je hand in het apparaat om te beginnen.
 
 Put your hand in this zone.
 Hou je hand in dit gebied.
 
 Touch the screen for a new scene.
 Raak het scherm aan voor een nieuwe sc√®ne.
 
 Oops! Try moving more slowly.
 Oeps! Probeer langzamer te bewegen.
 
 Oops! Try keeping your hand flat.
 Oeps! Probeer je hand recht te houden.
 
 Oops! Your hand is too high up.
 Oeps! Je houdt je hand te hoog.
 
 Hey! Just one hand at a time, please.
 Hee! E√©n hand tegelijk alsjeblieft.
 
 Hey! Hold still for a moment, please.
 Hee! Hou je hand stil alsjeblieft.
 
 I'm sorry! Your hand might be too small :(
 Het spijt me! Je hand is waarschijnlijk te klein voor mij :(
 
 Try taking your hand out, and putting it in again.
 Probeer eens om je hand eruit te halen en deze er weer in te stoppen.
 
 Hey! Only hands, please.
 Hee! Alleen je hand alsjeblieft.
 
 Hey! That's not a hand.
 Hee! Dat is geen hand.
 
 */



void AppFaultManager::drawDebug(int x, int y){
    
    float timeVal = timeHasFault[FAULT_NO_USER_PRESENT_BRIEF];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_NO_USER_PRESENT_BRIEF],0,255),255);
    ofDrawBitmapString("No User Present Brief",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_NO_USER_PRESENT_LONG];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_NO_USER_PRESENT_LONG],0,255),255);
    ofDrawBitmapString("No User Present Long",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_LEAP_DROPPED_FRAME];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_LEAP_DROPPED_FRAME],0,255),255);
    ofDrawBitmapString("Leap dropped frame",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_NO_LEAP_HAND_TOO_SMALL];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_NO_LEAP_HAND_TOO_SMALL],0,255),255);
    ofDrawBitmapString("Hands too small",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_NO_LEAP_OBJECT_PRESENT];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_NO_LEAP_OBJECT_PRESENT],0,255),255);
    ofDrawBitmapString("Object present",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_TOO_MANY_HANDS];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_TOO_MANY_HANDS],0,255),255);
    ofDrawBitmapString("Too many hands",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_HAND_TOO_FAST];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_HAND_TOO_FAST],0,255),255);
    ofDrawBitmapString("Hands too fast",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_HAND_TOO_HIGH];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_HAND_TOO_HIGH],0,255),255);
    ofDrawBitmapString("Hands too high",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_HAND_TOO_CURLED];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_HAND_TOO_CURLED],0,255),255);
    ofDrawBitmapString("Hands too curled",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_HAND_TOO_VERTICAL];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_HAND_TOO_VERTICAL],0,255),255);
    ofDrawBitmapString("Hands too vertical",x,y); y+= 15;
    
    timeVal = timeHasFault[FAULT_SAME_SCENE_TOO_LONG];
    ofSetColor(0,ofMap(timeVal,0,timeLimit[FAULT_SAME_SCENE_TOO_LONG],0,255),255);
    ofDrawBitmapString("Same scene too long",x,y); y+= 15;
    
}

