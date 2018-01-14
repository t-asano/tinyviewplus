#include "ofApp.h"

#define FRAME_RATE      60
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_RATIO    1.3333
#define FONT_FILE       "mplus-1p-bold.ttf"
#define NAME_HEIGHT     30
#define NAME_MARGIN_X   10
#define NAME_MARGIN_Y   (NAME_HEIGHT + NAME_MARGIN_X)

void setViewParams();
ofVideoGrabber grabber[3];
int cameraNum;
int camViewPosX[3];
int camViewPosY[3];
int camViewWidth[3];
int camViewHeight[3];
ofxTrueTypeFontUC myFont;
int namePosX[3];
int namePosY[3];

//--------------------------------------------------------------
void ofApp::setup(){
    // path
    ofDirectory dir;
    if (dir.doesDirectoryExist("../data") == false) {
        // macOS binary release
        ofSetDataPathRoot("../Resources/data");
    }
    // screen
    ofSetWindowTitle("Tiny View Plus");
    ofBackground(0, 0, 0);
    ofSetVerticalSync(true);
    ofSetFrameRate(FRAME_RATE);
    myFont.loadFont(FONT_FILE, NAME_HEIGHT);
    // video
    cameraNum = 0;
    vector<ofVideoDevice> devices = grabber[0].listDevices();
    for (vector<ofVideoDevice>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (it->deviceName.substr(0, 16) == "USB2.0 PC CAMERA") {
            cameraNum++;
            if (cameraNum <= 3) {
                ofLogNotice() << "CAM" << cameraNum << ": " << it->deviceName;
                grabber[cameraNum - 1].setDeviceID(it->id);
                grabber[cameraNum - 1].initGrabber(CAMERA_WIDTH, CAMERA_HEIGHT);
            }
            if (cameraNum == 3) {
                break;
            }
        }
    }
    // view
    setViewParams();
}

//--------------------------------------------------------------
void ofApp::update(){
    for (int i = 0; i < cameraNum; i++) {
        grabber[i].update();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    if (cameraNum == 0) {
        ofSetColor(255, 215, 0);
        myFont.drawString("no FPV receiver detected", NAME_MARGIN_X, NAME_MARGIN_Y);
        return;
    }
    for (int i = 0; i < cameraNum; i++) {
        // video
        ofSetColor(255, 255, 255);
        grabber[i].draw(camViewPosX[i], camViewPosY[i], camViewWidth[i], camViewHeight[i]);
        // name
        ofSetColor(255, 215, 0);
        myFont.drawString("CAM" + ofToString(i + 1), namePosX[i], namePosY[i]);
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    setViewParams();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

void setViewParams() {
    int width = ofGetWidth();
    int height = ofGetHeight();
    float ratio = float(width) / float(height);
    switch (cameraNum) {
        case 1:
            // CAM1
            camViewWidth[0] = width;
            camViewHeight[0] = camViewWidth[0] / CAMERA_RATIO;
            camViewPosX[0] = (width / 2) - (camViewWidth[0] / 2);
            camViewPosY[0] = (height / 2) - (camViewHeight[0] / 2);
            namePosX[0] = ((camViewPosX[0] < 0) ? 0 : camViewPosX[0]) + NAME_MARGIN_X;
            namePosY[0] = ((camViewPosY[0] < 0) ? 0 : camViewPosY[0]) + NAME_MARGIN_Y;
            break;
        case 2:
            // CAM1
            camViewWidth[0] = width / 2;
            camViewHeight[0] = camViewWidth[0] / CAMERA_RATIO;
            camViewPosX[0] = -1;
            camViewPosY[0] = (height / 2) - (camViewHeight[0] / 2);
            namePosX[0] = ((camViewPosX[0] < 0) ? 0 : camViewPosX[0]) + NAME_MARGIN_X;
            namePosY[0] = ((camViewPosY[0] < 0) ? 0 : camViewPosY[0]) + NAME_MARGIN_Y;
            // CAM2
            camViewWidth[1] = width / 2;
            camViewHeight[1] = camViewWidth[1] / CAMERA_RATIO;
            camViewPosX[1] = (width / 2) + 1;
            camViewPosY[1] = (height / 2) - (camViewHeight[1] / 2);
            namePosX[1] = ((camViewPosX[1] < 0) ? 0 : camViewPosX[1]) + NAME_MARGIN_X;
            namePosY[1] = ((camViewPosY[1] < 0) ? 0 : camViewPosY[1]) + NAME_MARGIN_Y;
            break;
        case 3:
            // CAM1
            camViewHeight[0] = height * 0.55;
            camViewWidth[0] = camViewHeight[0] * CAMERA_RATIO;
            camViewPosX[0] = (width / 2) - (camViewWidth[0] / 2);
            camViewPosY[0] = 0;
            namePosX[0] = ((camViewPosX[0] < 0) ? 0 : camViewPosX[0]) + NAME_MARGIN_X;
            namePosY[0] = ((camViewPosY[0] < 0) ? 0 : camViewPosY[0]) + NAME_MARGIN_Y;
            // CAM2
            camViewHeight[1] = height * 0.55;
            camViewWidth[1] = camViewHeight[1] * CAMERA_RATIO;
            camViewPosX[1] = 0;
            camViewPosY[1] = height * 0.45;
            namePosX[1] = ((camViewPosX[1] < 0) ? 0 : camViewPosX[1]) + NAME_MARGIN_X;
            namePosY[1] = ((camViewPosY[1] < 0) ? 0 : camViewPosY[1]) + NAME_MARGIN_Y;
            // CAM3
            camViewHeight[2] = height * 0.55;
            camViewWidth[2] = camViewHeight[2] * CAMERA_RATIO;
            camViewPosX[2] = width - camViewWidth[2];
            camViewPosY[2] = height * 0.45;
            namePosX[2] = ((camViewPosX[2] < 0) ? 0 : camViewPosX[2]) + NAME_MARGIN_X;
            namePosY[2] = ((camViewPosY[2] < 0) ? 0 : camViewPosY[2]) + NAME_MARGIN_Y;
            break;
        default:
            // none
            break;
    }
}
