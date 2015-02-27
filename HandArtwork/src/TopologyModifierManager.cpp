#include "TopologyModifierManager.h"

#include "PlusOne.h"
#include "MinusOne.h"
#include "Stubby.h"
#include "TwoThumbs.h"
#include "OppositeThumb.h"
#include "SwellingFingers.h"
#include "ExtraKnuckles.h"
#include "FractalHand.h"

void activateRadio(ofxUIRadio* radio, int i) {
    radio->activateToggle(radio->getToggles()[i]->getName());
}

void TopologyModifierManager::setup() {
    scenes.push_back(new MinusOne());
    scenes.push_back(new PlusOne());
    scenes.push_back(new Stubby());
    scenes.push_back(new TwoThumbs());
    scenes.push_back(new OppositeThumb());
    scenes.push_back(new SwellingFingers());
    scenes.push_back(new ExtraKnuckles());
    scenes.push_back(new FractalHand());
    
    vector<string> sceneNames;
    for(int i = 0; i < scenes.size(); i++) {
        TopologyModifier* scene = scenes[i];
		scene->initialize();
        string sceneName = scene->getName();
        sceneNames.push_back(sceneName);
    }
    gui = new ofxUICanvas();
    gui->addLabel("TopologyModifierManager");
    gui->addSpacer();
    sceneRadio = gui->addRadio("Scene", sceneNames);
    gui->autoSizeToFitWidgets();
    gui->setPosition(290, 10);
    
    setScene(0);
}
void TopologyModifierManager::setGuiVisibility(bool visibility) {
    gui->setVisible(visibility);
}
void TopologyModifierManager::setScene(int sceneIndex) {
    curScene = scenes[sceneIndex];
    activateRadio(sceneRadio, sceneIndex);
}
int TopologyModifierManager::getSceneCount() const {
    return scenes.size();
}
void TopologyModifierManager::update(HandMeshBuilder& handMeshBuilder) {
    int scene = sceneRadio->getValue();
    setScene(scene);
    
    ofMesh& mesh = handMeshBuilder.getMesh();
    curScene->update(mesh);
    
    //show gui for current scene and hide the guis for the rest
    curScene->gui->setVisible(true);
    for(int i = 0; i < scenes.size(); i++) {
        if(i != scene) {
            scenes[i]->gui->setVisible(false);
        }
    }
}
void TopologyModifierManager::draw(const ofTexture& texture) {
    curScene->draw(texture);
}

void TopologyModifierManager::saveMeshes(){
	curScene->saveMeshes();
}