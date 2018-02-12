#include "ofApp.h"

#ifdef TARGET_OSX
#define FEATURE_SPEECH
#endif /* TARGET_OSX */

// view
#define FRAME_RATE      60
#define VERTICAL_SYNC   true
#define WALL_FILE       "default_background.png"
#define CAMERA_MAXNUM   3
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_RATIO    1.3333
#define FONT_FILE       "mplus-1p-bold.ttf"
#define ICON_FILE       "default_pilot_icon.png"
#define ICON_PATH       "pilots/"
#define ICON_WIDTH      40
#define ICON_HEIGHT     40
#define ICON_MARGIN_X   10
#define ICON_MARGIN_Y   10
#define LABEL_HEIGHT    30
#define LABEL_MARGIN_X  60
#define LABEL_MARGIN_Y  50
#define BASE_MARGIN_X   0
#define BASE_MARGIN_Y   40
#define BASE_HEIGHT     20
#define BASE_1_RED      201
#define BASE_1_GREEN    58
#define BASE_1_BLUE     64
#define BASE_2_RED      160
#define BASE_2_GREEN    194
#define BASE_2_BLUE     56
#define BASE_3_RED      0
#define BASE_3_GREEN    116
#define BASE_3_BLUE     191
#define BASE_ALPHA      128
#define LAP_HEIGHT      30
#define LAP_MARGIN_X    10
#define LAP_MARGIN_Y    100
// osc
#define OSC_LISTEN_PORT 4000
// help
#ifdef FEATURE_SPEECH
#define HELP_MSG_SPEECH "[S] Toggle speech on/off\n"
#define DFLT_SPCH_ENBLD false
#else /* FEATURE_SPEECH */
#define HELP_MSG_SPEECH ""
#endif /* FEATURE_SPEECH */
#define HELP_MESSAGE    "Keyboard shortcuts:\n"\
                        "[H] Display help\n"\
                        "[1-3] Toggle camera 1-3 on/off\n"\
                        "[Q,W,E] Change camera 1-3 icon\n"\
                        "[L] Change camera label\n"\
                        "[B] Change background image\n"\
                        HELP_MSG_SPEECH\
                        "[R] Reset configuration\n"

void bindCameras();
void toggleCameraVisibility(int);
int getCameraIdxNthVisible(int);
void setupBaseColors();
void changeCameraLabel();
void changeCameraIcon(int);
void changeCameraIconPath(int, string);
void autoSelectCameraIcon(int, string);
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
    ofColor baseColor;
    int basePosX;
    int basePosY;
    int baseWidth;
    int baseHeight;
    ofImage iconImage;
    int iconPosX;
    int iconPosY;
    string labelString;
    int labelPosX;
    int labelPosY;
    int lapPosX;
    int lapPosY;
    float lap;
};

ofVideoGrabber grabber[CAMERA_MAXNUM];
ofxTrueTypeFontUC myFontLabel, myFontLap;
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
    myFontLabel.load(FONT_FILE, LABEL_HEIGHT);
    myFontLap.load(FONT_FILE, LAP_HEIGHT);
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
    speechEnabled = DFLT_SPCH_ENBLD;
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
        // camera image
        ofSetColor(255, 255, 255);
        grabber[i].draw(camView[i].posX, camView[i].posY, camView[i].width, camView[i].height);
        // base
        ofSetColor(camView[i].baseColor, BASE_ALPHA);
        ofDrawRectangle(camView[i].basePosX, camView[i].basePosY, camView[i].baseWidth, camView[i].baseHeight);
        // icon
        ofSetColor(255, 255, 255);
        camView[i].iconImage.draw(camView[i].iconPosX, camView[i].iconPosY, ICON_WIDTH, ICON_HEIGHT);
        // label
        if (camView[i].labelString != "") {
            ofSetColor(255, 215, 0);
            myFontLabel.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        }
        // laptime
        if (camView[i].lap != 0) {
            string sout;
            stringstream stream;
            stream << fixed << setprecision(2) << camView[i].lap;
            ofSetColor(255, 255, 255);
            sout = ofToString("Lap:") + stream.str() + "s";
            myFontLap.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
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
    } else if (key == 'b' || key == 'B') {
        changeWallImage();
    } else if (key == 'q' || key == 'Q') {
        changeCameraIcon(1);
    } else if (key == 'w' || key == 'W') {
        changeCameraIcon(2);
    } else if (key == 'e' || key == 'E') {
        changeCameraIcon(3);
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
            if (cameraNum < CAMERA_MAXNUM) {
                int idx = cameraNum;
                grabber[idx].setDeviceID(it->id);
                if (grabber[idx].initGrabber(CAMERA_WIDTH, CAMERA_HEIGHT) == true) {
                    ofLogNotice() << "Pilot" << (cameraNum + 1) << ": " << it->deviceName;
                    camView[idx].visible = true;
                    camView[idx].iconImage.load(ICON_FILE);
                    camView[idx].labelString = "Pilot" + ofToString(idx + 1);
                    camView[idx].lap = 0;
                    cameraNum++;
                }
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
    setupBaseColors();
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
void setupBaseColors() {
    for (int i = 0; i < CAMERA_MAXNUM; i++) {
        switch(i) {
            case 0:
                camView[i].baseColor.r = BASE_1_RED;
                camView[i].baseColor.g = BASE_1_GREEN;
                camView[i].baseColor.b = BASE_1_BLUE;
                break;
            case 1:
                camView[i].baseColor.r = BASE_2_RED;
                camView[i].baseColor.g = BASE_2_GREEN;
                camView[i].baseColor.b = BASE_2_BLUE;
                break;
            case 2:
                camView[i].baseColor.r = BASE_3_RED;
                camView[i].baseColor.g = BASE_3_GREEN;
                camView[i].baseColor.b = BASE_3_BLUE;
                break;
            defaut:
                break;
        }
    }
}

//--------------------------------------------------------------
void changeCameraLabel() {
    string str;
    for (int i = 0; (i + 1) <= cameraNum; i++) {
        str = camView[i].labelString;
        str = ofSystemTextBoxDialog("camera" + ofToString(i + 1) + " label:", str);
        camView[i].labelString = str;
        autoSelectCameraIcon(i + 1, str);
    }
}

//--------------------------------------------------------------
void changeCameraIcon(int camid) {
    string str;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    str = "change camera" + ofToString(camid) + " icon";
    ofFileDialogResult result = ofSystemLoadDialog(str);
    if (result.bSuccess) {
        string path = result.getPath();
        changeCameraIconPath(camid, path);
    }
}

//--------------------------------------------------------------
void changeCameraIconPath(int camid, string path) {
    ofFile file(path);
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    string ext = ofToLower(file.getExtension());
    if (ext == "jpg" || ext == "png") {
        int idx = camid - 1;
        camView[idx].iconImage.clear();
        camView[idx].iconImage.load(path);
    } else {
        ofSystemAlertDialog("unsupported file type");
    }
}

//--------------------------------------------------------------
void autoSelectCameraIcon(int camid, string pname) {
    ofFile file;
    string path;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    path = ICON_PATH + pname + ".png";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path);
        return;
    }
    path = ICON_PATH + pname + ".jpg";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path);
    }
}

//--------------------------------------------------------------
void changeWallImage() {
    ofFileDialogResult result = ofSystemLoadDialog("change wallpaper");
    if (result.bSuccess) {
        string path = result.getPath();
        ofFile file(path);
        string ext = ofToLower(file.getExtension());
        if (ext == "jpg" || ext == "png") {
            wallImage.clear();
            wallImage.load(path);
            wallRatio = wallImage.getWidth() / wallImage.getHeight();
            setWallParams();
        } else {
            ofSystemAlertDialog("unsupported file type");
        }
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
    int idx, i;
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
            // 2nd visible camera
            idx = getCameraIdxNthVisible(2);
            if (idx == -1) {
                break;
            }
            camView[idx].width = width / 2;
            camView[idx].height = camView[idx].width / CAMERA_RATIO;
            camView[idx].posX = (width / 2) + 1;
            camView[idx].posY = (height / 2) - (camView[idx].height / 2);
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
            // 2nd visible camera
            idx = getCameraIdxNthVisible(2);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = 0;
            camView[idx].posY = height * 0.45;
            // 3rd visible camera
            idx = getCameraIdxNthVisible(3);
            if (idx == -1) {
                break;
            }
            camView[idx].height = height * 0.55;
            camView[idx].width = camView[idx].height * CAMERA_RATIO;
            camView[idx].posX = width - camView[idx].width;
            camView[idx].posY = height * 0.45;
            break;
        default:
            // none
            break;
    }
    for (i = 1; i <= cameraNumVisible; i++) {
        idx = getCameraIdxNthVisible(i);
        if (idx == -1) {
            break;
        }
        camView[idx].basePosX = max(0, camView[idx].posX) + BASE_MARGIN_X;
        camView[idx].basePosY = max(0, camView[idx].posY) + BASE_MARGIN_Y;
        camView[idx].baseWidth = camView[idx].width - (BASE_MARGIN_X * 2);
        camView[idx].baseHeight = BASE_HEIGHT;
        camView[idx].iconPosX = max(0, camView[idx].posX) + ICON_MARGIN_X;
        camView[idx].iconPosY = max(0, camView[idx].posY) + ICON_MARGIN_Y;
        camView[idx].labelPosX = max(0, camView[idx].posX) + LABEL_MARGIN_X;
        camView[idx].labelPosY = max(0, camView[idx].posY) + LABEL_MARGIN_Y;
        camView[idx].lapPosX = max(0, camView[idx].posX) + LAP_MARGIN_X;
        camView[idx].lapPosY = max(0, camView[idx].posY) + LAP_MARGIN_Y;
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
    // camera icon, label, laptime
    for (i = 0; i < cameraNum; i++) {
        camView[i].iconImage.clear();
        camView[i].iconImage.load(ICON_FILE);
        camView[i].labelString = "Pilot" + ofToString(i + 1);
        camView[i].lap = 0;
    }
#ifdef FEATURE_SPEECH
    // speech
    speechEnabled = DFLT_SPCH_ENBLD;
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
        autoSelectCameraIcon(camid, argstr);
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
