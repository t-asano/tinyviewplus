#include "ofApp.h"

#define FRAME_RATE      60
#define VERTICAL_SYNC   true
#define WALL_FILE       "wallpaper.png"
#define CAMERA_MAXNUM   3
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_RATIO    1.3333
#define FONT_FILE       "mplus-1p-bold.ttf"
#define LABEL_HEIGHT     30
#define LABEL_MARGIN_X   10
#define LABEL_MARGIN_Y   (LABEL_HEIGHT + LABEL_MARGIN_X)
#define HELP_MESSAGE    "Keyboard shortcuts:\n"\
                        "[1-3] Toggle camera 1-3 visibility\n"\
                        "[h] Display help\n"\
                        "[l] Change camera label\n"\
                        "[r] Reset configuration\n"\
                        "[w] Change wallpaper\n"

void bindCameras();
void toggleCameraVisibility(int);
int getCameraIdxNthVisible(int);
void changeCameraLabel();
void changeWallImage();
void setWallParams();
void setViewParams();
void resetConfig();

class tvpCamView {
public:
    bool visible;
    int width;
    int height;
    int posX;
    int posY;
    string labelString;
    int labelPosX;
    int labelPosY;
};

ofVideoGrabber grabber[3];
ofxTrueTypeFontUC myFont;
ofImage wallImage;
float wallRatio;
int wallDrawWidth;
int wallDrawHeight;
tvpCamView camView[3];
int cameraNum;
int cameraNumVisible;
string helpMessage;

//--------------------------------------------------------------
void ofApp::setup(){
    // path
    ofDirectory dir;
    if (dir.doesDirectoryExist("../data") == false) {
        // macOS binary release
        ofSetDataPathRoot("../Resources/data");
    }
    // help
    helpMessage = ofToString(HELP_MESSAGE);
    // screen
    ofSetWindowTitle("Tiny View Plus");
    ofBackground(0, 0, 0);
    ofSetVerticalSync(VERTICAL_SYNC);
    ofSetFrameRate(FRAME_RATE);
    myFont.loadFont(FONT_FILE, LABEL_HEIGHT);
    // wallpaper
    wallImage.load(WALL_FILE);
    wallRatio = wallImage.getWidth() / wallImage.getHeight();
    setWallParams();
    // camera
    bindCameras();
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
    // wallpaper
    ofSetColor(255, 255, 255);
    wallImage.draw(0, 0, wallDrawWidth, wallDrawHeight);
    // camera
    for (int i = 0; i < cameraNum; i++) {
        // image
        if (camView[i].visible == true) {
            ofSetColor(255, 255, 255);
            grabber[i].draw(camView[i].posX, camView[i].posY, camView[i].width, camView[i].height);
        }
        // label
        if (camView[i].visible == true && camView[i].labelString != "") {
            ofSetColor(255, 215, 0);
            myFont.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        }
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == '1') {
        toggleCameraVisibility(1);
    } else if (key == '2') {
        toggleCameraVisibility(2);
    } else if (key == '3') {
        toggleCameraVisibility(3);
    } else if (key == 'h' || key == 'H') {
        ofSystemAlertDialog(helpMessage);
    } else if (key == 'l' || key == 'L') {
        changeCameraLabel();
    } else if (key == 'r' || key == 'R') {
        resetConfig();
    } else if (key == 'w' || key == 'W') {
        changeWallImage();
    }
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
    // wallpaper
    setWallParams();
    // view
    setViewParams();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

//--------------------------------------------------------------
void bindCameras() {
    cameraNum = 0;
    for (int i = 0; i < CAMERA_MAXNUM; i++) {
        camView[i].visible = false;
        camView[i].labelString = "CAM" + ofToString(i + 1);
    }
    vector<ofVideoDevice> devices = grabber[0].listDevices();
    for (vector<ofVideoDevice>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (it->deviceName.substr(0, 16) == "USB2.0 PC CAMERA") {
            cameraNum++;
            if (cameraNum <= 3) {
                int idx = cameraNum - 1;
                ofLogNotice() << "CAM" << cameraNum << ": " << it->deviceName;
                grabber[idx].setDeviceID(it->id);
                grabber[idx].initGrabber(CAMERA_WIDTH, CAMERA_HEIGHT);
                camView[idx].visible = true;
            }
            if (cameraNum == 3) {
                break;
            }
        }
    }
    cameraNumVisible = cameraNum;
    if (cameraNum == 0) {
        ofSystemAlertDialog("Error: no FPV receiver detected");
    }
}

//--------------------------------------------------------------
void toggleCameraVisibility(int num) {
    int idx = num - 1;
    if (num < 1) {
        return;
    }
    if (num > cameraNum) {
        ofSystemAlertDialog("camera" + ofToString(num) + " not found");
        return;
    }
    if (camView[idx].visible == true) {
        camView[idx].visible = false;
        cameraNumVisible -= 1;
    } else {
        camView[idx].visible = true;
        cameraNumVisible += 1;
    }
    setViewParams();
}

//--------------------------------------------------------------
int getCameraIdxNthVisible(int nth) {
    int cnt = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == true) {
            cnt++;
            if (cnt == nth) {
                return i;
            }
        }
    }
    return -1;
}

//--------------------------------------------------------------
void changeCameraLabel() {
    string str;
    for (int i = 0; i < CAMERA_MAXNUM && (i + 1) <= cameraNum; i++) {
        str = camView[i].labelString;
        str = ofSystemTextBoxDialog("camera" + ofToString(i + 1) + " name:", str);
        camView[i].labelString = str;
    }
}

//--------------------------------------------------------------
void changeWallImage() {
    ofFileDialogResult result = ofSystemLoadDialog("change wallpaper");
    if (result.bSuccess) {
        string path = result.getPath();
        ofFile file(path);
        string ext = ofToLower(file.getExtension());
        if (ext == "jpg" || ext == "jpeg" || ext == "png" || ext == "gif") {
            wallImage.clear();
            wallImage.load(path);
            wallRatio = wallImage.getWidth() / wallImage.getHeight();
            setWallParams();
        } else {
            ofSystemAlertDialog("Error: unsupported file type");
        }
    } else {
        ofSystemAlertDialog("Error: can't load file");
    }
}

//--------------------------------------------------------------
void setWallParams() {
    float sratio;
    sratio = float(ofGetWidth()) / float(ofGetHeight());
    if (sratio > wallRatio) {
        wallDrawWidth = ofGetWidth();
        wallDrawHeight = wallDrawWidth / wallRatio;
    } else {
        wallDrawHeight = ofGetHeight();
        wallDrawWidth = wallDrawHeight * wallRatio;
    }
}

//--------------------------------------------------------------
void setViewParams() {
    int idx;
    int width = ofGetWidth();
    int height = ofGetHeight();
    float ratio = float(width) / float(height);
    switch (cameraNumVisible) {
        case 1:
            // 1st visible camera
            idx = getCameraIdxNthVisible(1);
            if (idx == -1) {
                break;
            }
            camView[idx].width = width;
            camView[idx].height = camView[idx].width / CAMERA_RATIO;
            camView[idx].posX = (width / 2) - (camView[idx].width / 2);
            camView[idx].posY = (height / 2) - (camView[idx].height / 2);
            camView[idx].labelPosX = ((camView[idx].posX < 0) ? 0 : camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = ((camView[idx].posY < 0) ? 0 : camView[idx].posY) + LABEL_MARGIN_Y;
            break;
        case 2:
            // 1st visible camera
            idx = getCameraIdxNthVisible(1);
            if (idx == -1) {
                break;
            }
            camView[idx].width = width / 2;
            camView[idx].height = camView[idx].width / CAMERA_RATIO;
            camView[idx].posX = -1;
            camView[idx].posY = (height / 2) - (camView[idx].height / 2);
            camView[idx].labelPosX = ((camView[idx].posX < 0) ? 0 : camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = ((camView[idx].posY < 0) ? 0 : camView[idx].posY) + LABEL_MARGIN_Y;
            // 2nd visible camera
            idx = getCameraIdxNthVisible(2);
            if (idx == -1) {
                break;
            }
            camView[idx].width = width / 2;
            camView[idx].height = camView[idx].width / CAMERA_RATIO;
            camView[idx].posX = (width / 2) + 1;
            camView[idx].posY = (height / 2) - (camView[idx].height / 2);
            camView[idx].labelPosX = ((camView[idx].posX < 0) ? 0 : camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = ((camView[idx].posY < 0) ? 0 : camView[idx].posY) + LABEL_MARGIN_Y;
            break;
        case 3:
            // 1st visible camera
            idx = getCameraIdxNthVisible(1);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = (width / 2) - (camView[idx].width / 2);
            camView[idx].posY = 0;
            camView[idx].labelPosX = ((camView[idx].posX < 0) ? 0 : camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = ((camView[idx].posY < 0) ? 0 : camView[idx].posY) + LABEL_MARGIN_Y;
            // 2nd visible camera
            idx = getCameraIdxNthVisible(2);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = 0;
            camView[idx].posY = height * 0.45;
            camView[idx].labelPosX = ((camView[idx].posX < 0) ? 0 : camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = ((camView[idx].posY < 0) ? 0 : camView[idx].posY) + LABEL_MARGIN_Y;
            // 3rd visible camera
            idx = getCameraIdxNthVisible(3);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = width - camView[idx].width;
            camView[idx].posY = height * 0.45;
            camView[idx].labelPosX = ((camView[idx].posX < 0) ? 0 : camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = ((camView[idx].posY < 0) ? 0 : camView[idx].posY) + LABEL_MARGIN_Y;
            break;
        default:
            // none
            break;
    }
}

//--------------------------------------------------------------
void resetConfig() {
    int i;
    // wallpaper
    wallImage.clear();
    wallImage.load(WALL_FILE);
    wallRatio = wallImage.getWidth() / wallImage.getHeight();
    setWallParams();
    // camera visibility
    for (i = 0; i < cameraNum; i++) {
        camView[i].visible = true;
    }
    cameraNumVisible = cameraNum;
    setViewParams();
    // camera label
    for (i = 0; i < CAMERA_MAXNUM; i++) {
        camView[i].labelString = "CAM" + ofToString(i + 1);
    }
}
