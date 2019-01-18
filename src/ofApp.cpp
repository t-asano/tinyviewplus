#include "ofApp.h"

/* ---------- variables ---------- */

// view
ofVideoGrabber grabber[CAMERA_MAXNUM];
ofxTrueTypeFontUC myFontNumber, myFontLabel, myFontLap;
ofxTrueTypeFontUC myFontNumberSub, myFontLabelSub, myFontLapSub;
ofImage wallImage;
float wallRatio;
int wallDrawWidth;
int wallDrawHeight;
tvpCamView camView[CAMERA_MAXNUM];
int cameraNum;
int cameraNumVisible;
int cameraIdxSolo;
string helpMessage;
bool soloViewTrim;
bool fullscreenEnabled;

// osc
ofxOscReceiver oscReceiver;

// speech
bool oscSpeechEnabled;
bool speechLangJpn;
#ifdef TARGET_WIN32
sayWin mySayWin[SPCH_SLOT_NUM];
#endif /* TARGET_WIN32 */

// AR lap timer
ofxTrueTypeFontUC myFontWatch;
ofSoundPlayer beepSound, countSound, finishSound, lockonSound;
ofFile resultsFile;
bool arLapEnabled;
bool lockOnEnabled;
bool raceStarted;
int raceDuraSecs;
int raceDuraLaps;
int minLapTime;
float elapsedTime;

//--------------------------------------------------------------
void ofApp::setup() {
    // system
    ofSetEscapeQuitsApp(false);
    ofDirectory dir;
    if (dir.doesDirectoryExist("../data") == false) {
        // macOS binary release
        ofSetDataPathRoot("../Resources/data");
    }
#ifdef TARGET_WIN32
    HWND handleWindow;
    AllocConsole();
    handleWindow = FindWindowA("ConsoleWindowClass", NULL);
    ShowWindow(handleWindow, 0);
#endif /* TARGET_WIN32 */
    // speech
    oscSpeechEnabled = DFLT_SPCH_ENBLD;
    speechLangJpn = DFLT_SPCH_JPN;
#ifdef TARGET_WIN32
    speakAny("en", ""); // warmup
#endif /* TARGET_WIN32 */
    // help
    helpMessage = ofToString(HELP_MESSAGE);
    // screen
    ofSetWindowTitle("Tiny View Plus");
    ofBackground(0, 0, 0);
    ofSetVerticalSync(VERTICAL_SYNC);
    ofSetFrameRate(FRAME_RATE);
    myFontNumber.load(FONT_P_FILE, NUMBER_HEIGHT);
    myFontLabel.load(FONT_P_FILE, LABEL_HEIGHT);
    myFontLap.load(FONT_P_FILE, LAP_HEIGHT);
    myFontNumberSub.load(FONT_P_FILE, NUMBER_HEIGHT / 2);
    myFontLabelSub.load(FONT_P_FILE, LABEL_HEIGHT / 2);
    myFontLapSub.load(FONT_P_FILE, LAP_HEIGHT / 2);
    soloViewTrim = DFLT_SOLO_TRIM;
    fullscreenEnabled = DFLT_FSCR_ENBLD;
    // wallpaper
    wallImage.load(WALL_FILE);
    wallRatio = wallImage.getWidth() / wallImage.getHeight();
    setWallParams();
    // camera
    bindCameras();
    setViewParams();
    // osc
    oscReceiver.setup(OSC_LISTEN_PORT);
    // AR lap timer
    arLapEnabled = DFLT_ARAP_ENBLD;
    lockOnEnabled = DFLT_ARAP_LCKON;
    minLapTime = DFLT_ARAP_MNLAP;
    raceDuraSecs = DFLT_ARAP_RSECS;
    raceDuraLaps = DFLT_ARAP_RLAPS;
    beepSound.load(SND_BEEP_FILE);
    countSound.load(SND_COUNT_FILE);
    finishSound.load(SND_FINISH_FILE);
    lockonSound.load(SND_LOCKON_FILE);
    myFontWatch.load(FONT_M_FILE, WATCH_HEIGHT);
    for (int i = 0; i < cameraNum; i++) {
        camView[i].aruco.setUseHighlyReliableMarker(ARAP_MKR_FILE);
        camView[i].aruco.setThreaded(true);
        camView[i].aruco.setup2d(CAMERA_WIDTH, CAMERA_HEIGHT);
    }
    raceStarted = false;
    elapsedTime = 0;
}

//--------------------------------------------------------------
void ofApp::update() {
    // timer
    if (raceStarted == true) {
        elapsedTime = ofGetElapsedTimef();
        // finish race by time
        if (raceDuraSecs > 0 && (elapsedTime - WATCH_COUNT_SEC) > raceDuraSecs) {
            for (int i = 0; i < cameraNum; i++) {
                grabber[i].update();
                camView[i].foundMarkerNum = 0;
                camView[i].prevElapsedSec = raceDuraSecs + WATCH_COUNT_SEC;
            }
            toggleRace();
            recvOsc();
            updateViewParams();
            return;
        }
    }
    // camera, lap
    int lapcnt = 0;
    int lockcnt = 0;
    for (int i = 0; i < cameraNum; i++) {
        grabber[i].update();
        if (raceStarted == false || elapsedTime < WATCH_COUNT_SEC) {
            continue;
        }
        // AR lap timer
        float elp = elapsedTime;
        if (grabber[i].isFrameNew() && arLapEnabled == true) {
            camView[i].aruco.detectMarkers(grabber[i].getPixels());
            int num = camView[i].aruco.getNumMarkers();
            if (num == 0 && camView[i].foundMarkerNum >= ARAP_MNUM_THR) {
                float lap = elp - camView[i].prevElapsedSec;
                if (lap < minLapTime) {
                    // ignore too short lap
                    camView[i].foundMarkerNum = 0;
                    continue;
                }
                if (camView[i].totalLaps >= raceDuraLaps) {
                    // already finished
                    continue;
                }
                int total = camView[i].totalLaps + 1;
                camView[i].prevElapsedSec = elp;
                camView[i].totalLaps = total;
                camView[i].lastLap = lap;
                camView[i].lapHistory[total - 1] = lap;
                if (total == raceDuraLaps) {
                    // finish
                    camView[i].foundMarkerNum = 0;
                    finishSound.play();
                    speakLap((i + 1), lap);
                    continue;
                }
                // lap with lock-on
                bool locked = false;
                if (lockOnEnabled == true) {
                    for (int j = 0; j < cameraNum; j++) {
                        if (j == i) {
                            continue;
                        }
                        float diff = elp - camView[j].prevElapsedSec;
                        if (diff > 0 && diff < ARAP_LOCKON_SEC) {
                            // lock on!
                            locked = true;
                            lockcnt++;
                            enableCameraSolo(i + 1);
                            lockonSound.play();
                            break;
                        }
                    }
                }
                // lap without lock-on
                if (locked == false) {
                    lapcnt++;
                    beepSound.play();
                }
                speakLap((i + 1), lap);
            }
            if (num == 0) {
                camView[i].foundMarkerNum = 0;
            } else if (num > camView[i].foundMarkerNum) {
                camView[i].foundMarkerNum = num;
            }
        }
    }
    if (lockOnEnabled == true && lapcnt > 0 && lockcnt == 0) {
        // xxx experimental
        resetCameraSolo();
    }
    // finish race by laps
    if (raceStarted == true) {
        int count = 0;
        for (int i = 0; i < cameraNum; i++) {
            if (camView[i].totalLaps >= raceDuraLaps) {
                count++;
            }
        }
        if (count == cameraNum) {
            toggleRace();
        }
    }
    // osc
    recvOsc();
    // view layout
    updateViewParams();
}

//--------------------------------------------------------------
void drawCamera(int idx) {
    int i = idx;
    bool isSub = false;
    if (cameraIdxSolo != -1 && i != cameraIdxSolo) {
        isSub = true;
    }
    // image
    ofSetColor(255, 255, 255);
    grabber[i].draw(camView[i].posX, camView[i].posY, camView[i].width, camView[i].height);
    // base
    ofSetColor(camView[i].baseColor);
    ofDrawRectangle(camView[i].basePosX, camView[i].basePosY, camView[i].baseWidth, camView[i].baseHeight);
    // number
    ofSetColor(255, 255, 255);
    if (isSub) {
        myFontNumberSub.drawString(ofToString(i + 1), camView[i].numberPosX, camView[i].numberPosY);
    } else {
        myFontNumber.drawString(ofToString(i + 1), camView[i].numberPosX, camView[i].numberPosY);
    }
    // icon
    ofSetColor(255, 255, 255);
    if (isSub) {
        camView[i].iconImage.draw(camView[i].iconPosX, camView[i].iconPosY, ICON_WIDTH / 2, ICON_HEIGHT / 2);
    } else {
        camView[i].iconImage.draw(camView[i].iconPosX, camView[i].iconPosY, ICON_WIDTH, ICON_HEIGHT);
    }
    // label
    if (camView[i].labelString != "") {
        ofSetColor(255, 215, 0);
        if (isSub) {
            myFontLabelSub.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        } else {
            myFontLabel.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        }
    }
    // lap time
    if (camView[i].lastLap != 0) {
        string sout;
        int laps = camView[i].totalLaps;
        if (isRecordedLaps() == true && raceStarted == false) {
            // AR lap timer && race stopped
            sout = "Laps: " + ofToString(laps);
            ofSetColor(255, 255, 255);
            if (isSub) {
                myFontLapSub.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
            } else {
                myFontLap.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
            }
            float blap = getBestLap(i);
            if (blap != 0) {
                sout = "BestLap: " + getLapStr(blap) + "s";
                if (isSub) {
                    myFontLapSub.drawString(sout, camView[i].lapPosX, camView[i].lapPosY + (LAP_HEIGHT / 2) + 5);
                } else {
                    myFontLap.drawString(sout, camView[i].lapPosX, camView[i].lapPosY + LAP_HEIGHT + 10);
                }
                sout = "Time: " + getWatchString(camView[i].prevElapsedSec - WATCH_COUNT_SEC);
                if (isSub) {
                    myFontLapSub.drawString(sout, camView[i].lapPosX, camView[i].lapPosY + LAP_HEIGHT + 10);
                } else {
                    myFontLap.drawString(sout, camView[i].lapPosX, camView[i].lapPosY + (LAP_HEIGHT * 2) + 20);
                }
            }
        } else {
            // others
            sout = "Lap" + ((laps > 0) ? ofToString(laps) : "") + ": " + getLapStr(camView[i].lastLap) + "s";
            ofSetColor(255, 255, 255);
            if (isSub) {
                myFontLapSub.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
            } else {
                myFontLap.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
            }
        }
    }
    // AR marker
    if (arLapEnabled == true && raceStarted == true && camView[i].totalLaps < raceDuraLaps) {
        ofSetColor(255, 215, 0);
        string lv = "";
        for (int j = 0; j < camView[i].foundMarkerNum; j++) {
            lv += "|";
        }
        if (isSub) {
            myFontLapSub.drawString(lv, camView[i].lapPosX, camView[i].lapPosY + (LAP_HEIGHT / 2) + 5);
        } else {
            myFontLap.drawString(lv, camView[i].lapPosX, camView[i].lapPosY + LAP_HEIGHT + 10);
        }
    }
}

//--------------------------------------------------------------
string getWatchString(float sec) {
    char buf[16];
    int m = (int)(sec) / 60;
    int s = (int)(sec) % 60;
    int ss = (int)(sec * 100) % 100;
    snprintf(buf, sizeof(buf), "%02d:%02d.%02d", m, s, ss);
    return ofToString(buf);
}

//--------------------------------------------------------------
void drawWatch() {
    string str;
    if (raceStarted == false) {
        str = "Finished";
    } else if (elapsedTime < 5) {
        str = ofToString(5 - (int)elapsedTime);
    } else if (elapsedTime < 7) {
        str = "Go!";
    } else {
        float sec;
        if (raceDuraSecs <= 0) {
            sec = elapsedTime - WATCH_COUNT_SEC;
        } else {
            sec = raceDuraSecs - (elapsedTime - WATCH_COUNT_SEC);
        }
        str = getWatchString(sec);
    }
    ofSetColor(255, 255, 255);
    int x = (ofGetWidth() / 2) - (myFontWatch.stringWidth(str) / 2);
    x = (int)(x / 5) * 5;
    myFontWatch.drawString(str, x, ofGetHeight() - 10);
}

//--------------------------------------------------------------
void drawRaceResult() {
    // xxx under development
}

//--------------------------------------------------------------
void ofApp::draw(){
    // wallpaper
    ofSetColor(255, 255, 255);
    wallImage.draw(0, 0, wallDrawWidth, wallDrawHeight);
    // camera (solo main)
    if (cameraIdxSolo != -1) {
        drawCamera(cameraIdxSolo);
    }
    // camera (solo sub / solo off)
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false || i == cameraIdxSolo) {
            continue;
        }
        drawCamera(i);
    }
    // total time
    if (raceStarted == true || elapsedTime != 0) {
        drawWatch();
    }
#if 0
    // xxx for debug
    ofSetColor(255, 215, 0);
    myFontLabel.drawString("FPS: " + ofToString(ofGetFrameRate()), 10, 360);
#endif /* 0 */
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == '1') {
        toggleCameraSolo(1);
    } else if (key == '2') {
        toggleCameraSolo(2);
    } else if (key == '3') {
        toggleCameraSolo(3);
    } else if (key == '4') {
        toggleCameraSolo(4);
    } else if (key == '!') {
        toggleCameraVisibility(1);
    } else if (key == '"' || key == '@') {
        toggleCameraVisibility(2);
    } else if (key == '#') {
        toggleCameraVisibility(3);
    } else if (key == '$') {
        toggleCameraVisibility(4);
    } else if (key == 'h' || key == 'H') {
        ofSystemAlertDialog(helpMessage);
    } else if (key == 'i' || key == 'I') {
        initConfig();
    } else if (key == 'l' || key == 'L') {
        changeCameraLabelAll();
    } else if (key == 's' || key == 'S') {
        toggleOscSpeech();
    } else if (key == 'n' || key == 'N') {
        toggleSpeechLang();
    } else if (key == 'b' || key == 'B') {
        changeWallImage();
    } else if (key == 'q' || key == 'Q') {
        changeCameraIcon(1);
    } else if (key == 'w' || key == 'W') {
        changeCameraIcon(2);
    } else if (key == 'e' || key == 'E') {
        changeCameraIcon(3);
    } else if (key == 'r' || key == 'R') {
        changeCameraIcon(4);
    } else if (key == ' ') {
        toggleRace();
    } else if (key == 'v' || key == 'V') {
        drawRaceResult(); // xxx
    } else if (key == 'a' || key == 'A') {
        toggleARLap();
    } else if (key == 'm' || key == 'M') {
        changeMinLap();
    } else if (key == 'd' || key == 'D') {
        changeRaceDuration();
    } else if (key == 'o' || key == 'O') {
        toggleLockOnEffect();
    } else if (key == 'f' || key == 'F') {
        toggleFullscreen();
    } else if (key == 't' || key == 'T') {
        toggleSoloTrim();
    } else if (key == '.') {
        ofSystemAlertDialog("Exit application");
        ofExit();
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
    // camera icon
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false) {
            continue;
        }
        if (x >= camView[i].iconPosX && x <= camView[i].iconPosX + ICON_WIDTH
            && y >= camView[i].iconPosY && y <= camView[i].iconPosY + ICON_HEIGHT) {
            changeCameraIcon(i + 1);
        }
    }
    // camera label
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false) {
            continue;
        }
        if (x >= camView[i].labelPosX && x <= camView[i].posX + camView[i].width
            && y >= camView[i].posY && y <= camView[i].iconPosY + ICON_HEIGHT) {
            changeCameraLabel(i + 1);
        }
    }
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
    cameraIdxSolo = -1;
    vector<ofVideoDevice> devices = grabber[0].listDevices();
    for (vector<ofVideoDevice>::iterator it = devices.begin(); it != devices.end(); ++it) {
        if (regex_search(it->deviceName, regex("USB2.0 PC CAMERA")) == true
            || regex_search(it->deviceName, regex("GV-USB2")) == true) {
            if (cameraNum < CAMERA_MAXNUM) {
                int idx = cameraNum;
                grabber[idx].setDeviceID(it->id);
                if (grabber[idx].initGrabber(CAMERA_WIDTH, CAMERA_HEIGHT) == true) {
                    ofLogNotice() << "Pilot" << (cameraNum + 1) << ": " << it->deviceName;
                    camView[idx].visible = true;
                    camView[idx].iconImage.load(ICON_FILE);
                    camView[idx].labelString = "Pilot" + ofToString(idx + 1);
                    camView[idx].lastLap = 0;
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
        ofSystemAlertDialog("No FPV receiver");
        ofExit();
    }
    setupBaseColors();
}

//--------------------------------------------------------------
void toggleCameraSolo(int camid) {
    int idx = camid - 1;
    if (cameraNum == 1 || camid < 1 || camid > cameraNum) {
        return;
    }
    if (idx == cameraIdxSolo) {
        cameraIdxSolo = -1;
    } else {
        if (camView[idx].visible == false) {
            toggleCameraVisibility(camid);
        }
        cameraIdxSolo = idx;
    }
    setViewParams();
}

//--------------------------------------------------------------
void enableCameraSolo(int camid) {
    int idx = camid - 1;
    if (cameraNum == 1 || camid < 1 || camid > cameraNum) {
        return;
    }
    if (idx != cameraIdxSolo) {
        if (camView[idx].visible == false) {
            toggleCameraVisibility(camid);
        }
        cameraIdxSolo = idx;
    }
    setViewParams();
}

//--------------------------------------------------------------
void resetCameraSolo() {
    cameraIdxSolo = -1;
    setViewParams();
}

//--------------------------------------------------------------
void toggleCameraVisibility(int camid) {
    int idx = camid - 1;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (camView[idx].visible == true) {
        if (cameraIdxSolo == idx) {
            toggleCameraSolo(camid);
        }
        camView[idx].visible = false;
        cameraNumVisible -= 1;
    } else {
        camView[idx].visible = true;
        cameraNumVisible += 1;
    }
    setViewParams();
}

//--------------------------------------------------------------
int getCameraIdxNthVisibleAll(int nth) {
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
int getCameraIdxNthVisibleSub(int nth) {
    int cnt = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == true && i != cameraIdxSolo) {
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
            case 3:
                camView[i].baseColor.r = BASE_4_RED;
                camView[i].baseColor.g = BASE_4_GREEN;
                camView[i].baseColor.b = BASE_4_BLUE;
                break;
            default:
                break;
        }
    }
}

//--------------------------------------------------------------
void changeCameraLabel(int camid) {
    string str;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    str = camView[camid - 1].labelString;
    str = ofSystemTextBoxDialog("Camera" + ofToString(camid) + " label:", str);
    camView[camid - 1].labelString = str;
    autoSelectCameraIcon(camid, str);
}

//--------------------------------------------------------------
void changeCameraLabelAll() {
    for (int i = 0; i < cameraNum; i++) {
        changeCameraLabel(i + 1);
    }
}

//--------------------------------------------------------------
void changeCameraIcon(int camid) {
    string str;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    str = "Camera" + ofToString(camid) + " icon";
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
        ofSystemAlertDialog("Unsupported file type");
    }
}

//--------------------------------------------------------------
void autoSelectCameraIcon(int camid, string pname) {
    ofFile file;
    string path;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    path = ICON_DIR + pname + ".png";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path);
        return;
    }
    path = ICON_DIR + pname + ".jpg";
    if (file.doesFileExist(path)) {
        changeCameraIconPath(camid, path);
    } else {
        changeCameraIconPath(camid, ICON_FILE);
    }
}

//--------------------------------------------------------------
void changeWallImage() {
    ofFileDialogResult result = ofSystemLoadDialog("Wallpaper");
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
            ofSystemAlertDialog("Unsupported file type");
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
            idx = getCameraIdxNthVisibleAll(1);
            if (idx == -1) {
                break;
            }
            camView[idx].moveSteps = MOVE_STEPS;
            if ((ratio > CAMERA_RATIO && soloViewTrim == true)
                || (ratio <= CAMERA_RATIO && soloViewTrim == false)) {
                // wide-fill, tall-fit
                camView[idx].widthTarget = width;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
            } else {
                // wide-fit, tall-fill
                camView[idx].heightTarget = height;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
            }
            camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
            camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
            break;
        case 2:
            if (cameraIdxSolo == -1) { // solo mode off
                // 1st camera
                idx = getCameraIdxNthVisibleAll(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 2;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = -1;
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 2;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
            } else { // solo mode on
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && soloViewTrim == true)
                    || (ratio <= CAMERA_RATIO && soloViewTrim == false)) {
                    // wide-fill, tall-fit
                    camView[idx].widthTarget = width;
                    camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                } else {
                    // wide-fit, tall-fill
                    camView[idx].heightTarget = height;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                }
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 1st sub camera
                idx = getCameraIdxNthVisibleSub(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            }
            break;
        case 3:
            if (cameraIdxSolo == -1) { // solo mode off
                // 1st camera
                idx = getCameraIdxNthVisibleAll(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.55;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = 0;
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.55;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = height * 0.45;
                // 3rd camera
                idx = getCameraIdxNthVisibleAll(3);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.55;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height * 0.45;
            } else { // solo mode on
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && soloViewTrim == true)
                    || (ratio <= CAMERA_RATIO && soloViewTrim == false)) {
                    // wide-fill, tall-fit
                    camView[idx].widthTarget = width;
                    camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                } else {
                    // wide-fit, tall-fill
                    camView[idx].heightTarget = height;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                }
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 1st sub camera
                idx = getCameraIdxNthVisibleSub(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 2nd sub camera
                idx = getCameraIdxNthVisibleSub(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            }
            break;
        case 4:
            if (cameraIdxSolo == -1) { // solo mode off
                // 1st camera
                idx = getCameraIdxNthVisibleAll(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.5;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget + 1);
                camView[idx].posYTarget = -1;
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.5;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = -1;
                // 3rd camera
                idx = getCameraIdxNthVisibleAll(3);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.5;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget + 1);
                camView[idx].posYTarget = (height / 2) + 1;
                // 4th camera
                idx = getCameraIdxNthVisibleAll(4);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = height * 0.5;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = (height / 2) + 1;
            } else { // solo mode off
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && soloViewTrim == true)
                    || (ratio <= CAMERA_RATIO && soloViewTrim == false)) {
                    // wide-fill, tall-fit
                    camView[idx].widthTarget = width;
                    camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                } else {
                    // wide-fit, tall-fill
                    camView[idx].heightTarget = height;
                    camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                }
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 1st sub camera
                idx = getCameraIdxNthVisibleSub(1);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 2nd sub camera
                idx = getCameraIdxNthVisibleSub(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 3rd sub camera
                idx = getCameraIdxNthVisibleSub(3);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = width / 5;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            }
            break;
        default:
            // none
            break;
    }
    for (i = 1; i <= cameraNumVisible; i++) {
        idx = getCameraIdxNthVisibleAll(i);
        if (idx == -1) {
            break;
        }
        camView[idx].basePosXTarget = max(0, camView[idx].posXTarget) + BASE_MARGIN_X;
        camView[idx].basePosYTarget = max(0, camView[idx].posYTarget) + BASE_MARGIN_Y;
        camView[idx].baseWidth = BASE_WIDTH;
        camView[idx].baseHeight = BASE_HEIGHT;
        camView[idx].numberPosXTarget = max(0, camView[idx].posXTarget) + NUMBER_MARGIN_X;
        camView[idx].numberPosYTarget = max(0, camView[idx].posYTarget) + NUMBER_MARGIN_Y;
        camView[idx].iconPosXTarget = max(0, camView[idx].posXTarget) + ICON_MARGIN_X;
        camView[idx].iconPosYTarget = max(0, camView[idx].posYTarget) + ICON_MARGIN_Y;
        camView[idx].labelPosXTarget = max(0, camView[idx].posXTarget) + LABEL_MARGIN_X;
        camView[idx].labelPosYTarget = max(0, camView[idx].posYTarget) + LABEL_MARGIN_Y;
        camView[idx].lapPosXTarget = max(0, camView[idx].posXTarget) + LAP_MARGIN_X;
        camView[idx].lapPosYTarget = max(0, camView[idx].posYTarget) + LAP_MARGIN_Y;
        if (cameraIdxSolo != -1 && idx != cameraIdxSolo) { // sub
            camView[idx].baseWidth = camView[idx].baseWidth / 2;
            camView[idx].baseHeight = camView[idx].baseHeight / 2;
            camView[idx].numberPosXTarget = camView[idx].numberPosXTarget - (NUMBER_MARGIN_X / 2);
            camView[idx].numberPosYTarget = camView[idx].numberPosYTarget - (NUMBER_MARGIN_Y / 2);
            camView[idx].iconPosXTarget = camView[idx].iconPosXTarget - (ICON_MARGIN_X / 2);
            camView[idx].labelPosXTarget = camView[idx].labelPosXTarget - (LABEL_MARGIN_X / 2);
            camView[idx].labelPosYTarget = camView[idx].labelPosYTarget - (LABEL_MARGIN_Y / 2);
            camView[idx].lapPosXTarget = camView[idx].lapPosXTarget - (LAP_MARGIN_X / 2);
            camView[idx].lapPosYTarget = camView[idx].lapPosYTarget - (LAP_MARGIN_Y / 2);
        }
    }
}

//--------------------------------------------------------------
int calcViewParam(int target, int current, int steps) {
    int val, diff;
    if (steps == 0 || target == current) {
        return target;
    }
    if (target > current) {
        diff = (target - current) / steps;
        val = current + diff;
    } else {
        diff = (current - target) / steps;
        val = current - diff;
    }
    return val;
}

//--------------------------------------------------------------
void updateViewParams() {
    int i, idx, steps;
    for (i = 1; i <= cameraNumVisible; i++) {
        // normal view
        idx = getCameraIdxNthVisibleAll(i);
        if (idx == -1) {
            break;
        }
        steps = camView[idx].moveSteps;
        if (steps == 0) {
            continue;
        }
        // camera
        camView[idx].width = calcViewParam(camView[idx].widthTarget, camView[idx].width, steps);
        camView[idx].height = calcViewParam(camView[idx].heightTarget, camView[idx].height, steps);
        camView[idx].posX = calcViewParam(camView[idx].posXTarget, camView[idx].posX, steps);
        camView[idx].posY = calcViewParam(camView[idx].posYTarget, camView[idx].posY, steps);
        // base
        camView[idx].basePosX = calcViewParam(camView[idx].basePosXTarget, camView[idx].basePosX, steps);
        camView[idx].basePosY = calcViewParam(camView[idx].basePosYTarget, camView[idx].basePosY, steps);
        // number
        camView[idx].numberPosX = calcViewParam(camView[idx].numberPosXTarget, camView[idx].numberPosX, steps);
        camView[idx].numberPosY = calcViewParam(camView[idx].numberPosYTarget, camView[idx].numberPosY, steps);
        // icon
        camView[idx].iconPosX = calcViewParam(camView[idx].iconPosXTarget, camView[idx].iconPosX, steps);
        camView[idx].iconPosY = calcViewParam(camView[idx].iconPosYTarget, camView[idx].iconPosY, steps);
        // label
        camView[idx].labelPosX = calcViewParam(camView[idx].labelPosXTarget, camView[idx].labelPosX, steps);
        camView[idx].labelPosY = calcViewParam(camView[idx].labelPosYTarget, camView[idx].labelPosY, steps);
        // lap
        camView[idx].lapPosX = calcViewParam(camView[idx].lapPosXTarget, camView[idx].lapPosX, steps);
        camView[idx].lapPosY = calcViewParam(camView[idx].lapPosYTarget, camView[idx].lapPosY, steps);
        // finished
        camView[idx].moveSteps--;
    }
}

//--------------------------------------------------------------
void initConfig() {
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
    cameraIdxSolo = -1;
    setViewParams();
    // camera icon, label, laptime
    for (i = 0; i < cameraNum; i++) {
        camView[i].iconImage.clear();
        camView[i].iconImage.load(ICON_FILE);
        camView[i].labelString = "Pilot" + ofToString(i + 1);
        camView[i].lastLap = 0;
    }
    // view mode
    soloViewTrim = DFLT_SOLO_TRIM;
    fullscreenEnabled = DFLT_FSCR_ENBLD;
    // speech
    oscSpeechEnabled = DFLT_SPCH_ENBLD;
    speechLangJpn = DFLT_SPCH_JPN;
    // AR lap timer
    arLapEnabled = DFLT_ARAP_ENBLD;
    lockOnEnabled = DFLT_ARAP_LCKON;
    minLapTime = DFLT_ARAP_MNLAP;
    raceDuraLaps = DFLT_ARAP_RLAPS;
    raceDuraSecs = DFLT_ARAP_RSECS;
    raceStarted = false;
    ofSystemAlertDialog("Configuration initialized");
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
    }
}

//--------------------------------------------------------------
void recvOscCameraString(int camid, string method, string argstr) {
    ofLogNotice() << "osc cam(s): " << method << "," << camid << "," << argstr;
    if (camid < 1 || camid > cameraNum) {
        return;
    }
    if (method == "solo") {
        if (cameraIdxSolo != (camid - 1) && argstr == "on") {
            toggleCameraSolo(camid);
        }
        if (cameraIdxSolo == (camid - 1) && argstr == "off") {
            toggleCameraSolo(camid);
        }
    } else if (method == "display") {
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
    beepSound.play();
    if (oscSpeechEnabled == true){
        speakLap(camid, argfloat);
    }
    camView[camid - 1].lastLap = argfloat;
}

//--------------------------------------------------------------
void toggleOscSpeech() {
    oscSpeechEnabled = !oscSpeechEnabled;
    if (oscSpeechEnabled == true) {
        ofSystemAlertDialog("Speech on");
    } else {
        ofSystemAlertDialog("Speech off");
    }
}

//--------------------------------------------------------------
void toggleSpeechLang() {
    speechLangJpn = !speechLangJpn;
    if (speechLangJpn == true) {
        ofSystemAlertDialog("Speech language japanese");
    }
    else {
        ofSystemAlertDialog("Speech language english");
    }
}

//--------------------------------------------------------------
void recvOscSpeech(string lang, string text) {
    ofLogNotice() << "osc spc(s): " << lang << "," << text;
    if (oscSpeechEnabled == false) {
        return;
    }
    speakAny(lang, text);
}

//--------------------------------------------------------------
void speakLap(int camid, float sec) {
    if (camid < 1 || camid > cameraNum || sec == 0.0) {
        return;
    }
    string ssec, sout;
    ssec = getLapStr(sec);
    sout = camView[camid - 1].labelString + ", ";
#ifdef TARGET_WIN32
    sout = regex_replace(sout, regex("(Pilot)(\\d)"), "$1 $2");
#endif /* TARGET_WIN32 */
    if (speechLangJpn == true) {
        sout += ofToString(int(sec)) + "秒"; // UTF-8
        sout += ssec.substr(ssec.length() - 2, 1) + " ";
        sout += ssec.substr(ssec.length() - 1, 1);
    }
    else {
        sout += ofToString(int(sec)) + ".";
        sout += ssec.substr(ssec.length() - 2, 1);
        sout += ssec.substr(ssec.length() - 1, 1) + " seconds";
    }
    speakAny(speechLangJpn ? "jp" : "en", sout);
}

//--------------------------------------------------------------
void speakAny(string lang, string text) {
#ifdef TARGET_OSX
    int pid = fork();
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
#endif /* TARGET_OSX */
#ifdef TARGET_WIN32
    for (int i = 0; i < SPCH_SLOT_NUM; i++) {
        if (mySayWin[i].isThreadRunning() == false) {
            mySayWin[i].exec(lang, text);
            break;
        }
    }
#endif /* TARGET_WIN32 */
}

//--------------------------------------------------------------
void toggleRace() {
    if (lockOnEnabled == true) {
        resetCameraSolo();
    }
    if (raceStarted == false) {
        // init -> start
        finishSound.stop();
        countSound.play();
        ofResetElapsedTimeCounter();
        for (int i = 0; i < cameraNum; i++) {
            camView[i].foundMarkerNum = 0;
            camView[i].prevElapsedSec = WATCH_COUNT_SEC; // countdown
            camView[i].totalLaps = 0;
            camView[i].lastLap = 0;
            for (int h = 0; h < ARAP_MAX_RLAPS; h++) {
                camView[i].lapHistory[h] = 0;
            }
        }
        raceStarted = true;
    }
    else {
        // start -> stop
        raceStarted = false;
        countSound.stop();
        finishSound.play();
        fwriteRaceResult();
    }
}

//--------------------------------------------------------------
bool isRecordedLaps() {
    bool ret = false;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].totalLaps > 0) {
            ret = true;
            break;
        }
    }
    return ret;
}

//--------------------------------------------------------------
float getBestLap(int camidx) {
    float blap = 0;
    if (camidx < 0 || camidx >= cameraNum) {
        return blap;
    }
    for (int i = 0; i < camView[camidx].totalLaps; i++) {
        float h = camView[camidx].lapHistory[i];
        if (blap == 0) {
            blap = h;
        } else if (h < blap) {
            blap = h;
        }
    }
    return blap;
}

//--------------------------------------------------------------
int getMaxLaps() {
    int laps = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].totalLaps > laps) {
            laps = camView[i].totalLaps;
        }
    }
    return laps;
}

//--------------------------------------------------------------
string getLapStr(float lap) {
    stringstream stream;
    stream << fixed << setprecision(2) << lap; // 2 digits
    return stream.str();
}

//--------------------------------------------------------------
void fwriteRaceResult() {
    string newline;
#ifdef TARGET_WIN32
    newline = "\r\n";
#else /* TARGET_WIN32 */
    newline = "\n";
#endif /* TARGET_WIN32 */
    string strsumm = "Race Result:" + newline + newline;
    string strlaph = "";
    string strlapb = "";
    string sep = "  ";
    int maxlap = 0;

    if (raceStarted == true || isRecordedLaps() == false) {
        return;
    }

    // SUMMARY: PILOT LAPS BESTLAP TIME
    // - head
    strsumm += "- Summary -" + newline;
    strsumm += "PILOT" + sep  + "LAPS" + sep + "BESTLAP" + sep + "TIME" + newline;
    // - body
    for (int i = 0; i < cameraNum; i++) {
        float blap = getBestLap(i);
        float total = camView[i].prevElapsedSec - WATCH_COUNT_SEC;
        string pilot = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        strsumm += pilot + sep; // PILOT
        strsumm += ofToString(camView[i].totalLaps) + sep; // LAPS
        strsumm += ((blap == 0) ? "-.-" : getLapStr(blap)) + sep; // BESTLAP
        strsumm += ((total <= 0) ? "-:-.-" : getWatchString(total)) + sep; // TIME
        strsumm += newline;
    }
    strsumm += newline;

    // LAP TIMES: LAP P1 P2 P3 P4
    // - head
    strlaph += "- Lap Times -" + newline;
    strlaph += "LAP" + sep;
    for (int i = 0; i < cameraNum; i++) {
        string pilot = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        strlaph += pilot + sep;
    }
    strlaph += newline;
    maxlap = getMaxLaps();
    // - body
    for (int lap = 1; lap <= maxlap; lap++) {
        strlapb += ofToString(lap) + sep;
        for (int i = 0; i < cameraNum; i++) {
            // LAPTIME
            if (lap > camView[i].totalLaps) {
                strlapb += "-.-";
            } else {
                strlapb += getLapStr(camView[i].lapHistory[lap - 1]);
            }
            if (i < (cameraNum - 1)) {
                strlapb += sep;
            }
        }
        strlapb += newline;
    }

    // write to file
    resultsFile.open(ARAP_RESULT_DIR + ofGetTimestampString() + ".txt" , ofFile::WriteOnly);
    resultsFile << (strsumm + strlaph + strlapb);
    resultsFile.close();
}

//--------------------------------------------------------------
void toggleARLap() {
    arLapEnabled = !arLapEnabled;
    if (arLapEnabled == true) {
        ofSystemAlertDialog("AR lap timer on");
    } else {
        ofSystemAlertDialog("AR lap timer off");
    }
}

//--------------------------------------------------------------
void toggleLockOnEffect() {
    lockOnEnabled = !lockOnEnabled;
    resetCameraSolo();
    if (lockOnEnabled == true) {
        ofSystemAlertDialog("Lock-on effect on");
    } else {
        ofSystemAlertDialog("Lock-on effect off");
    }
}

//--------------------------------------------------------------
void changeMinLap() {
    string str;
    int lap;
    str = ofToString(minLapTime);
    str = ofSystemTextBoxDialog("Min. lap time (1~" + ofToString(ARAP_MAX_MNLAP) + "sec):", str);
    lap = (str == "") ? 0 : ofToInt(str);
    if (lap > 0 && lap <= ARAP_MAX_MNLAP) {
        minLapTime = lap;
    } else {
        ofSystemAlertDialog("Please enter 1~" + ofToString(ARAP_MAX_MNLAP));
        changeMinLap();
    }
}

//--------------------------------------------------------------
void changeRaceDuration() {
    string str;
    // time (seconds)
    while (true) {
        int sec;
        str = (raceDuraSecs == 0) ? "" : ofToString(raceDuraSecs);
        str = ofSystemTextBoxDialog("Race time (sec):", str);
        sec = (str == "") ? 0 : ofToInt(str);
        if (sec <= 0) {
            // no limit
            raceDuraSecs = 0;
            break;
        } else if (sec <= ARAP_MAX_RSECS) {
            raceDuraSecs = sec;
            break;
        } else {
            ofSystemAlertDialog("Please enter 0~" + ofToString(ARAP_MAX_RSECS) + " (0/empty means no limit)");
            // retry
        }
    }
    // laps
    while (true) {
        int laps;
        str = (raceDuraLaps == 0) ? "" : ofToString(raceDuraLaps);
        str = ofSystemTextBoxDialog("Race laps:", str);
        laps = (str == "") ? 0 : ofToInt(str);
        if (laps > 0 && laps <= ARAP_MAX_RLAPS) {
            raceDuraLaps = laps;
            break;
        } else {
            ofSystemAlertDialog("Please enter 1~" + ofToString(ARAP_MAX_RLAPS));
            // retry
        }
    }
}

//--------------------------------------------------------------
void toggleFullscreen() {
    fullscreenEnabled = !fullscreenEnabled;
    ofSetFullscreen(fullscreenEnabled);
}

//--------------------------------------------------------------
void toggleSoloTrim() {
    soloViewTrim = !soloViewTrim;
    setViewParams();
}
