#pragma once

#include "HandMeshBuilder.h"
#include "TopologyModifier.h"

class TopologyModifierManager {
private:
    TopologyModifier* curScene;
    vector<TopologyModifier*> scenes;
    
    ofxUICanvas* gui;
    ofxUIRadio* sceneRadio;
    
public:
    void setup();
    void setGuiVisibility(bool visibility);
    void setScene(int sceneIndex);
    int getSceneCount() const;
    void update(HandMeshBuilder& handMeshBuilder);
    void draw(const ofTexture& texture);
};