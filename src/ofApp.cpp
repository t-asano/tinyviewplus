#include "ofApp.h"

#ifdef TARGET_OSX
#define FEATURE_SPEECH
#endif /* TARGET_OSX */

// view
#define FRAME_RATE      60
#define VERTICAL_SYNC   true
#define WALL_FILE       "wallpaper.png"
#define CAMERA_MAXNUM   3
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_RATIO    1.3333
#define FONT_FILE       "mplus-1p-bold.ttf"
#define LABEL_HEIGHT    30
#define LABEL_MARGIN_X  10
#define LABEL_MARGIN_Y  (10 + LABEL_HEIGHT)
#define LAP_HEIGHT      LABEL_HEIGHT
#define LAP_MARGIN_X    10
#define LAP_MARGIN_Y    (10 + LABEL_MARGIN_Y + LAP_HEIGHT)
// osc
#define OSC_LISTEN_PORT 4000
// help
#ifdef FEATURE_SPEECH
#define HELP_MSG_SPEECH "[S] Toggle speech on/off\n"
#else /* FEATURE_SPEECH */
#define HELP_MSG_SPEECH ""
#endif /* FEATURE_SPEECH */
#define HELP_MESSAGE    "Keyboard shortcuts:\n"\
                        "[1-3] Toggle camera 1-3 on/off\n"\
                        "[H] Display help\n"\
                        "[L] Change camera label\n"\
                        "[R] Reset configuration\n"\
                        HELP_MSG_SPEECH\
                        "[W] Change wallpaper\n"

void bindCameras();
void toggleCameraVisibility(int);
int getCameraIdxNthVisible(int);
void changeCameraLabel();
void changeWallImage();
void setWallParams();
void setViewParams();
void resetConfig();
void recvOsc();
void recvOscCameraString(int, string, string);
void recvOscCameraFloat(int, string, float);
#ifdef FEATURE_SPEECH
void toggleSpeech();
void recvOscSpeech(string, string);
void speakLap(int, float);
void speakAny(string, string);
#endif /* FEATURE_SPEECH */

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
    int lapPosX;
    int lapPosY;
    float lap;
};

ofVideoGrabber grabber[CAMERA_MAXNUM];
ofxTrueTypeFontUC myFont;
ofImage wallImage;
float wallRatio;
int wallDrawWidth;
int wallDrawHeight;
tvpCamView camView[CAMERA_MAXNUM];
int cameraNum;
int cameraNumVisible;
string helpMessage;
ofxOscReceiver oscReceiver;
#ifdef FEATURE_SPEECH
bool speechEnabled;
#endif /* FEATURE_SPEECH */

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
    // osc
    oscReceiver.setup(OSC_LISTEN_PORT);
#ifdef FEATURE_SPEECH
    // speech
    speechEnabled = false;
#endif /* FEATURE_SPEECH */
}

//--------------------------------------------------------------
void ofApp::update(){
    // camera
    for (int i = 0; i < cameraNum; i++) {
        grabber[i].update();
    }
    // osc
    recvOsc();
}

//--------------------------------------------------------------
void ofApp::draw(){
    // wallpaper
    ofSetColor(255, 255, 255);
    wallImage.draw(0, 0, wallDrawWidth, wallDrawHeight);
    // camera
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false) {
            continue;
        }
        // image
        ofSetColor(255, 255, 255);
        grabber[i].draw(camView[i].posX, camView[i].posY, camView[i].width, camView[i].height);
        // label
        if (camView[i].labelString != "") {
            ofSetColor(255, 215, 0);
            myFont.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        }
        // laptime
        if (camView[i].lap != 0) {
            string sout;
            stringstream stream;
            stream << fixed << setprecision(2) << camView[i].lap;
            ofSetColor(255, 255, 255);
            sout = ofToString("Lap:") + stream.str() + "s";
            myFont.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
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
#ifdef FEATURE_SPEECH
    } else if (key == 's' || key == 'S') {
        toggleSpeech();
#endif /* FEATURE_SPEECH */
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
    vector<ofVideoDevice> devices = grabber[0].listDevices();
    for (vector<ofVideoDevice>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (it->deviceName.substr(0, 16) == "USB2.0 PC CAMERA") {
            cameraNum++;
            if (cameraNum <= CAMERA_MAXNUM) {
                int idx = cameraNum - 1;
                ofLogNotice() << "Pilot" << cameraNum << ": " << it->deviceName;
                grabber[idx].setDeviceID(it->id);
                grabber[idx].initGrabber(CAMERA_WIDTH, CAMERA_HEIGHT);
                camView[idx].visible = true;
                camView[idx].labelString = "Pilot" + ofToString(idx + 1);
                camView[idx].lap = 0;
            }
            if (cameraNum == CAMERA_MAXNUM) {
                break;
            }
        }
    }
    cameraNumVisible = cameraNum;
    if (cameraNum == 0) {
        ofSystemAlertDialog("no FPV receiver detected");
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
    for (int i = 0; (i + 1) <= cameraNum; i++) {
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
            camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
            camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
            camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
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
            camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
            camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
            camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
            // 2nd visible camera
            idx = getCameraIdxNthVisible(2);
            if (idx == -1) {
                break;
            }
            camView[idx].width = width / 2;
            camView[idx].height = camView[idx].width / CAMERA_RATIO;
            camView[idx].posX = (width / 2) + 1;
            camView[idx].posY = (height / 2) - (camView[idx].height / 2);
            camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
            camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
            camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
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
            camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
            camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
            camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
            // 2nd visible camera
            idx = getCameraIdxNthVisible(2);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = 0;
            camView[idx].posY = height * 0.45;
            camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
            camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
            camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
            // 3rd visible camera
            idx = getCameraIdxNthVisible(3);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = width - camView[idx].width;
            camView[idx].posY = height * 0.45;
            camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
            camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
            camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
            camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
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
    // camera label, laptime
    for (i = 0; i < cameraNum; i++) {
        camView[i].labelString = "Pilot" + ofToString(i + 1);
        camView[i].lap = 0;
    }
#ifdef FEATURE_SPEECH
    // speech
    speechEnabled = true;
#endif /* FEATURE_SPEECH */
}

//--------------------------------------------------------------
void recvOsc() {
    while (oscReceiver.hasWaitingMessages() == true) {
        ofxOscMessage oscm;
        string addr;
        string method;
        oscReceiver.getNextMessage(oscm);
        addr = oscm.getAddress();
        if (addr.find("/v1/camera/") == 0) {
            string str;
            int camid;
            int argtype;
            if (addr.length() <= 14 || addr[12] != '/') {
                continue;
            }
            // camera id
            str = ofToString(addr[11]);
            camid = ofToInt(str);
            // method
            method = ofToString(&addr[13]);
            // argument
            if (oscm.getNumArgs() != 1) {
                continue;
            }
            argtype = oscm.getArgType(0);
            switch (argtype) {
                case OFXOSC_TYPE_STRING:
                    recvOscCameraString(camid, method, oscm.getArgAsString(0));
                    break;
                case OFXOSC_TYPE_FLOAT:
                    recvOscCameraFloat(camid, method, oscm.getArgAsFloat(0));
                    break;
                case OFXOSC_TYPE_INT32:
                case OFXOSC_TYPE_INT64:
                    recvOscCameraFloat(camid, method, oscm.getArgAsFloat(0));
                    break;
                default:
                    break;
            }
        }
#ifdef FEATURE_SPEECH
        else if (addr.find("/v1/speech/") == 0) {
            if (oscm.getNumArgs() != 1 || oscm.getArgType(0) != OFXOSC_TYPE_STRING) {
                continue;
            }
            if (addr == "/v1/speech/en/say") {
                recvOscSpeech("en", oscm.getArgAsString(0));
            }
            else if (addr == "/v1/speech/jp/say") {
                recvOscSpeech("jp", oscm.getArgAsString(0));
            }
        }
#endif /* FEATURE_SPEECH */
    }
}

//--------------------------------------------------------------
void recvOscCameraString(int camid, string method, string argstr) {
    ofLogNotice() << "osc cam(s): " << method << "," << camid << "," << argstr;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (method == "display") {
        if (argstr == "on" && camView[camid - 1].visible == false) {
            toggleCameraVisibility(camid);
        }
        if (argstr == "off" && camView[camid - 1].visible == true) {
            toggleCameraVisibility(camid);
        }
    }
    else if (method == "label") {
        camView[camid - 1].labelString = argstr;
    }
}

//--------------------------------------------------------------
void recvOscCameraFloat(int camid, string method, float argfloat) {
    ofLogNotice() << "osc cam(f): " << method << "," << argfloat;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (method != "laptime") {
        return;
    }
#ifdef FEATURE_SPEECH
    if (speechEnabled == true){
        speakLap(camid, argfloat);
    }
#endif /* FEATURE_SPEECH */
    camView[camid - 1].lap = argfloat;
}

#ifdef FEATURE_SPEECH
//--------------------------------------------------------------
void toggleSpeech() {
    speechEnabled = !speechEnabled;
    if (speechEnabled == true) {
        ofSystemAlertDialog("Speech ON");
    } else {
        ofSystemAlertDialog("Speech OFF");
    }
}

//--------------------------------------------------------------
void recvOscSpeech(string lang, string text) {
    int pid;
    ofLogNotice() << "osc spc(s): " << lang << "," << text;
    if (speechEnabled == false) {
        return;
    }
    pid = fork();
    if (pid == 0) {
        // child process
        if (lang == "en") {
            execlp("say", "", "-r", "240", "-v", "Victoria", text.c_str(), NULL);
        }
        else if (lang == "jp") {
            execlp("say", "", "-r", "240", "-v", "Kyoko", text.c_str(), NULL);
        }
        OF_EXIT_APP(-1);
    }
}

//--------------------------------------------------------------
void speakLap(int camid, float sec) {
    int pid;
    if (camid < 1 || camid > cameraNum || sec == 0.0) {
        return;
    }
    pid = fork();
    if (pid == 0) {
        // child process
        execlp("afplay", "", "/System/Library/Sounds/Ping.aiff", NULL);
        OF_EXIT_APP(-1);
    }
    pid = fork();
    if (pid == 0) {
        // child process
        stringstream stream;
        string ssec, sout;
        stream << fixed << setprecision(2) << sec;
        ssec = stream.str();
        sout = camView[camid - 1].labelString + ", ";
        sout += ofToString(int(sec)) + "秒";
        sout += ssec.substr(ssec.length() - 2, 1) + " ";
        sout += ssec.substr(ssec.length() - 1, 1);
        system("sleep 0.5");
        execlp("say", "", "-r", "240", "-v", "Kyoko", sout.c_str(), NULL);
        OF_EXIT_APP(-1);
    }
}
#endif /* FEATURE_SPEECH */
