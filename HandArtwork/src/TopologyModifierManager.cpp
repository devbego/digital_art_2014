#include "TopologyModifierManager.h"

#include "PlusOne.h"
#include "MinusOne.h"

void activateRadio(ofxUIRadio* radio, int i) {
    radio->activateToggle(radio->getToggles()[i]->getName());
}

void TopologyModifierManager::setup() {
    scenes.push_back(new MinusOne());
    scenes.push_back(new PlusOne());
    
    vector<string> sceneNames;
    for(int i = 0; i < scenes.size(); i++) {
        TopologyModifier* scene = scenes[i];
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
}
void TopologyModifierManager::draw(const ofTexture& texture) {
    curScene->draw(texture);
}