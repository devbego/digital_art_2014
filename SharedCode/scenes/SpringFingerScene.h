//
//  SpringFingerScene.h
//  HandArtwork
//
//  Created by chris on 6/10/14.
//
//

#pragma once

#include "Scene.h"
#include "HandSkeleton.h"

class Spring;
class SpringNode;

class SpringFingerScene : public Scene {
protected:
	
    
public:
	
    SpringFingerScene(ofxPuppet* puppet, HandSkeleton* handSkeleton, HandSkeleton* immutableHandSkeleton);
	
    void setupGui();
	void setupMouseGui();
	void update();
	void updateMouse(float mx, float my);
	void draw();
    
    vector<Spring> springs;
    vector<SpringNode> nodesMids;
    vector<SpringNode> nodesTips;
    vector<SpringNode> nodesBase;
    vector<SpringNode> nodesExtended;
	
};

class SpringNode{
    
public:
    
    ofPoint pos, vel, acc;
    float damp, mass;
    bool bFixed;
    
    SpringNode(){
        pos.set(0,0);
        vel.set(0,0);
        acc.set(0,0);
        mass = 8;
        damp = .10;
        bFixed = false;
    }
    
    void setPosition(float x, float y){
        pos.set(x,y);
        
    }
    
    void update(){
        vel += acc;
        vel -= damp*vel;
        pos += vel;
        acc *= 0;
    }
    
    void applyForce( float fx, float fy){
        ofPoint force = ofPoint(fx,fy);
        force /= mass;
        acc += force;
    }
    
    void draw(){
        ofSetColor(255,0,0);
        ofEllipse(pos.x,pos.y,10,10);
    }
};


class Spring{
    
public:
    ofPoint anchor;
    float len;
    float k;
    
    SpringNode * a;
    SpringNode * b;
    
    Spring(){
        len = 0;
        k = 1.2;
    };
    
    void setup(SpringNode * a, SpringNode * b, float l){
        this->a = a;
        this->b = b;
        len = l;
    }
    
    void update(){
        
        ofPoint force = a->pos-b->pos;
        float d = force.length();
        float stretch = d - len;
        force.normalize();
        force.x *= -1 * k * stretch;
        force.y *= -1 * k * stretch;
        
        if(!a->bFixed) a->applyForce(force.x,force.y);
        force.x *= -1 ;
        force.y *= -1 ;
		
        if(!b->bFixed) b->applyForce(force.x,force.y);
    }
    
    
    
    void draw(){
        ofSetColor(255,255,0);
        ofLine(a->pos.x,a->pos.y,b->pos.x,b->pos.y);
    }
};