// charcter encoding is UTF-8

#include "ofApp.h"
#ifdef TARGET_WIN32
#include <sapi.h>
#include <atlcomcli.h>
#endif /* TARGET_WIN32 */

/* ---------- variables ---------- */

// view
ofVideoGrabber grabber[CAMERA_MAXNUM];
ofColor myColorYellow, myColorWhite, myColorLGray, myColorLayer, myColorAlert;
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
bool cameraTrimEnabled;
bool fullscreenEnabled;
bool cameraLapHistEnabled;
int overlayMode;

// osc
ofxOscReceiver oscReceiver;

// speech
bool oscSpeechEnabled;
bool speechLangJpn;

// AR lap timer
ofxTrueTypeFontUC myFontWatch;
ofSoundPlayer beepSound, beep3Sound, countSound, finishSound, notifySound;
ofFile resultsFile;
int arLapMode;
bool lockOnEnabled;
bool raceStarted;
int raceDuraSecs;
int nextSpeechRemainSecs;
int raceDuraLaps;
int minLapTime;
float elapsedTime;

// overlay
ofxTrueTypeFontUC myFontOvlayP, myFontOvlayP2x, myFontOvlayM;
int raceResultPage;

// QR code reader
bool qrEnabled;
int qrUpdCount;
int qrCamIndex;

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
    loadOverlayFont();
    cameraTrimEnabled = DFLT_CAM_TRIM;
    fullscreenEnabled = DFLT_FSCR_ENBLD;
    cameraLapHistEnabled = DFLT_CAM_LAPHST;
    overlayMode = OVLAY_NONE;
    // wallpaper
    wallImage.load(WALL_FILE);
    wallRatio = wallImage.getWidth() / wallImage.getHeight();
    setWallParams();
    // camera
    bindCameras();
    // view common
    setupColors();
    setViewParams();
    // osc
    oscReceiver.setup(OSC_LISTEN_PORT);
    // AR lap timer
    arLapMode = DFLT_ARAP_MODE;
    lockOnEnabled = DFLT_ARAP_LCKON;
    minLapTime = DFLT_ARAP_MNLAP;
    raceDuraSecs = DFLT_ARAP_RSECS;
    nextSpeechRemainSecs = -1;
    raceDuraLaps = DFLT_ARAP_RLAPS;
    beepSound.load(SND_BEEP_FILE);
    beep3Sound.load(SND_BEEP3_FILE);
    countSound.load(SND_COUNT_FILE);
    finishSound.load(SND_FINISH_FILE);
    notifySound.load(SND_NOTIFY_FILE);
    myFontWatch.load(FONT_M_FILE, WATCH_HEIGHT);
    for (int i = 0; i < cameraNum; i++) {
        camView[i].aruco.setUseHighlyReliableMarker(ARAP_MKR_FILE);
        camView[i].aruco.setThreaded(true);
        camView[i].aruco.setup2d(CAMERA_WIDTH, CAMERA_HEIGHT);
    }
    raceStarted = false;
    elapsedTime = 0;
    // race result
    raceResultPage = 0;
    // QR reader
    qrEnabled = false;
    // debug
    if (DEBUG_ENABLED == true) {
        generateDummyData();
    }
}

//--------------------------------------------------------------
void ofApp::update() {
    // timer
    if (raceStarted == true) {
        elapsedTime = ofGetElapsedTimef();
        // time limited race
        if (raceDuraSecs > 0) {
            float relp = elapsedTime - WATCH_COUNT_SEC;
            // speak remaining time
            if (nextSpeechRemainSecs > 0 && raceDuraSecs - relp <= nextSpeechRemainSecs) {
                notifySound.play();
                speakRemainTime(nextSpeechRemainSecs);
                setNextSpeechRemainSecs(nextSpeechRemainSecs);
            }
            // finish race by time
            if (relp >= raceDuraSecs) {
                for (int i = 0; i < cameraNum; i++) {
                    grabber[i].update();
                    camView[i].foundMarkerNum = 0;
                    camView[i].foundValidMarkerNum = 0;
                    camView[i].enoughMarkers = false;
                    camView[i].prevElapsedSec = raceDuraSecs + WATCH_COUNT_SEC;
                }
                toggleRace();
                recvOsc();
                updateViewParams();
                return;
            }
        }
    }
    // camera
    for (int i = 0; i < cameraNum; i++) {
        grabber[i].update();
    }
    // QR reader
    if (qrEnabled == true) {
        processQrReader();
    }
    // lap
    int lapcnt = 0;
    int lockcnt = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (raceStarted == false || elapsedTime < WATCH_COUNT_SEC) {
            continue;
        }
        // AR lap timer
        float elp = elapsedTime;
        if (grabber[i].isFrameNew() && arLapMode != ARAP_MODE_OFF) {
            camView[i].aruco.detectMarkers(grabber[i].getPixels());
            // all markers
            int anum = camView[i].aruco.getNumMarkers();
            if (anum == 0 && camView[i].foundMarkerNum > 0) {
                camView[i].flickerCount++;
                if (camView[i].flickerCount <= 3) {
                    anum = camView[i].foundMarkerNum; // anti flicker
                } else {
                    camView[i].flickerCount = 0;
                }
            } else {
                camView[i].flickerCount = 0;
            }
            // vaild markers
            int vnum = camView[i].aruco.getNumMarkersValidGate();
            if (vnum == 0 && camView[i].foundValidMarkerNum > 0) {
                camView[i].flickerValidCount++;
                if (camView[i].flickerValidCount <= 3) {
                    vnum = camView[i].foundValidMarkerNum; // anti flicker
                } else {
                    camView[i].flickerValidCount = 0;
                }
            } else {
                camView[i].flickerValidCount = 0;
            }
            // passed gate
            if (anum == 0 && camView[i].enoughMarkers == true
                && ((arLapMode == ARAP_MODE_LOOSE)
                    || (arLapMode == ARAP_MODE_NORM
                        && camView[i].foundMarkerNum == camView[i].foundValidMarkerNum))) {
                float lap = elp - camView[i].prevElapsedSec;
                if (lap < minLapTime) {
                    // ignore too short lap
                    camView[i].foundMarkerNum = 0;
                    camView[i].foundValidMarkerNum = 0;
                    camView[i].enoughMarkers = false;
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
                camView[i].lapHistoryName[total - 1] = camView[i].labelString;
                camView[i].lapHistoryTime[total - 1] = lap;
                if (total == raceDuraLaps) {
                    // finish
                    camView[i].foundMarkerNum = 0;
                    camView[i].foundValidMarkerNum = 0;
                    camView[i].enoughMarkers = false;
                    finishSound.play();
                    speakLap((i + 1), lap, total);
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
                            beep3Sound.play();
                            break;
                        }
                    }
                }
                // lap without lock-on
                if (locked == false) {
                    lapcnt++;
                    if (total == (raceDuraLaps - 1)) {
                        beep3Sound.play();
                    } else {
                        beepSound.play();
                    }
                }
                speakLap((i + 1), lap, total);
            }
            camView[i].foundMarkerNum = anum;
            camView[i].foundValidMarkerNum = vnum;
            if (anum == 0) {
                camView[i].enoughMarkers = false;
            } else if ((arLapMode == ARAP_MODE_NORM && vnum >= ARAP_MNUM_THR)
                       || (arLapMode == ARAP_MODE_LOOSE && anum >= ARAP_MNUM_THR)) {
                camView[i].enoughMarkers = true;
            }
        }
    }
    if (lockOnEnabled == true && lapcnt > 0 && lockcnt == 0) {
        resetCameraSolo(); // experimental
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
void drawCameraImage(int camidx) {
    int i = camidx;
    ofSetColor(myColorWhite);
    grabber[i].draw(camView[i].posX, camView[i].posY, camView[i].width, camView[i].height);
}

//--------------------------------------------------------------
void drawCameraARMarker(int idx, bool isSub) {
    int i = idx;
    if (arLapMode != ARAP_MODE_OFF && raceStarted == true && camView[i].totalLaps < raceDuraLaps) {
        // rect
        ofPushMatrix();
        ofTranslate(camView[i].posX, camView[i].posY);
        ofScale(camView[i].imageScale, camView[i].imageScale, 1);
        ofSetLineWidth(3);
        camView[i].aruco.draw2dGate(myColorYellow, myColorAlert, false);
        ofPopMatrix();
        if (cameraLapHistEnabled == true) {
            return;
        }
        // meter
        string lv_valid = "";
        string lv_invalid = "";
        int x, y;
        int vnum = camView[i].foundValidMarkerNum;
        int ivnum = camView[i].foundMarkerNum - camView[i].foundValidMarkerNum;
        for (int j = 0; j < vnum; j++) {
            lv_valid += "|";
        }
        for (int j = 0; j < ivnum; j++) {
            lv_invalid += "|";
        }
        x = camView[i].lapPosX;
        y = isSub ? (camView[i].lapPosY + (LAP_HEIGHT / 2) + 5) : (camView[i].lapPosY + LAP_HEIGHT + 10);
        if (vnum > 0) {
            ofSetColor(myColorYellow);
            if (isSub) {
                myFontLapSub.drawString(lv_valid, x, y);
            } else {
                myFontLap.drawString(lv_valid, x, y);
            }
        }
        if (ivnum > 0) {
            ofSetColor(myColorAlert);
            if (isSub) {
                if (vnum > 0) {
                    x += 2;
                }
                x = x + myFontLapSub.stringWidth(lv_valid);
                myFontLapSub.drawString(lv_invalid, x, y);
            } else {
                if (vnum > 0) {
                    x += 5;
                }
                x = x + myFontLap.stringWidth(lv_valid);
                myFontLap.drawString(lv_invalid, x, y);
            }
        }
    }
}

//--------------------------------------------------------------
void drawCameraPilot(int camidx, bool isSub) {
    int i = camidx;
    // base
    ofSetColor(camView[i].baseColor);
    ofDrawRectangle(camView[i].basePosX, camView[i].basePosY, camView[i].baseWidth, camView[i].baseHeight);
    // number
    ofSetColor(myColorWhite);
    if (isSub) {
        myFontNumberSub.drawString(ofToString(i + 1), camView[i].numberPosX, camView[i].numberPosY);
    } else {
        myFontNumber.drawString(ofToString(i + 1), camView[i].numberPosX, camView[i].numberPosY);
    }
    // icon
    ofSetColor(myColorWhite);
    if (isSub) {
        camView[i].iconImage.draw(camView[i].iconPosX, camView[i].iconPosY, ICON_WIDTH / 2, ICON_HEIGHT / 2);
    } else {
        camView[i].iconImage.draw(camView[i].iconPosX, camView[i].iconPosY, ICON_WIDTH, ICON_HEIGHT);
    }
    // label
    if (camView[i].labelString != "") {
        ofSetColor(myColorYellow);
        if (isSub) {
            myFontLabelSub.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        } else {
            myFontLabel.drawString(camView[i].labelString, camView[i].labelPosX, camView[i].labelPosY);
        }
    }
}

//--------------------------------------------------------------
void drawCameraLapTime(int idx, bool isSub) {
    int laps = camView[idx].totalLaps;
    if (laps == 0) {
        return;
    }
    int i = idx;
    string sout;
    if (raceStarted == false || camView[i].totalLaps == raceDuraLaps) {
        // race/laps finished
        sout = "Laps: " + ofToString(laps);
        ofSetColor(myColorWhite);
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
        // not finished
        sout = "Lap" + ofToString(laps) + ": " + getLapStr(camView[i].lastLap) + "s";
        ofSetColor(myColorWhite);
        if (isSub) {
            myFontLapSub.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
        } else {
            myFontLap.drawString(sout, camView[i].lapPosX, camView[i].lapPosY);
        }
        // history
        if (cameraLapHistEnabled == false || laps < 2 || camView[idx].moveSteps > 0 || isSub == true) {
            return;
        }
        drawCameraLapHistory(idx);
    }
}

//--------------------------------------------------------------
void drawCameraLapHistory(int camidx) {
    string text;
    float lap;
    int lapidx = camView[camidx].totalLaps - 2;
    int posy = camView[camidx].posY + LAP_MARGIN_Y + (LAP_HEIGHT / 2);
    ofSetColor(myColorWhite);
    for (; lapidx >= 0; lapidx--) {
        posy += LAPHIST_HEIGHT + (LAPHIST_HEIGHT / 2);
        if (posy + (LAPHIST_HEIGHT / 2) >= camView[camidx].height) {
            break;
        }
        lap = camView[camidx].lapHistoryTime[lapidx];
        text = "Lap" + ofToString(lapidx + 1) + ": " + getLapStr(lap) + "s";
        myFontLapSub.drawString(text, camView[camidx].lapPosX, posy);
    }
}

//--------------------------------------------------------------
void drawCamera(int idx) {
    bool isSub = false;
    if (cameraIdxSolo != -1 && idx != cameraIdxSolo) {
        isSub = true;
    }
    // image
    drawCameraImage(idx);
    // AR marker
    drawCameraARMarker(idx, isSub);
    // pilot
    drawCameraPilot(idx, isSub);
    // lap time
    drawCameraLapTime(idx, isSub);
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
    ofSetColor(myColorWhite);
    int x = (ofGetWidth() / 2) - (myFontWatch.stringWidth(str) / 2);
    x = (int)(x / 5) * 5;
    myFontWatch.drawString(str, x, ofGetHeight() - 10);
}

//--------------------------------------------------------------
void ofApp::draw(){
    // wallpaper
    ofSetColor(myColorWhite);
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
    // QR reader
    if (qrEnabled == true) {
        string str = "Scanning QR code...";
        ofSetColor(myColorYellow);
        int x = (ofGetWidth() / 2) - (myFontWatch.stringWidth(str) / 2);
        myFontWatch.drawString(str, x, ofGetHeight() - 10);
    }
    // overlay
    switch (overlayMode) {
        case OVLAY_RACE_RSLT:
            drawRaceResult(raceResultPage);
            break;
        case OVLAY_NONE:
            /* fall through */
        default:
            break;
    }
    // debug
    if (DEBUG_ENABLED == true) {
        ofSetColor(myColorYellow);
        ofDrawBitmapString("FPS: " + ofToString(ofGetFrameRate()), 10, ofGetHeight() - 10);
    }
}

//--------------------------------------------------------------
void keyPressedOverlayResult(int key) {
    if (key == 'v' || key == 'V') {
        processRaceResultDisplay();
    }
}

//--------------------------------------------------------------
void keyPressedOverlayNone(int key) {
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
    } else if (key == 's' || key == 'S') {
        toggleOscSpeech();
    } else if (key == 'n' || key == 'N') {
        toggleSpeechLang();
    } else if (key == 'b' || key == 'B') {
        changeWallImage();
    } else if (key == 'q' || key == 'Q') {
        if (raceStarted == false && overlayMode == OVLAY_NONE) {
            toggleQrReader();
        }
    } else if (key == ' ') {
        toggleRace();
    } else if (key == 'v' || key == 'V') {
        if (raceStarted == false) {
            qrEnabled = false;
            processRaceResultDisplay();
        }
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
    } else if (key == 'l' || key == 'L') {
        toggleLapHistory();
    } else if (key == '.') {
        ofExit();
    }
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
    switch (overlayMode) {
        case OVLAY_RACE_RSLT:
            keyPressedOverlayResult(key);
            break;
        case OVLAY_NONE:
            /* fall through */
        default:
            keyPressedOverlayNone(key);
            break;
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
    // race result
    if (overlayMode == OVLAY_RACE_RSLT) {
        processRaceResultDisplay();
        return;
    }
    // pilot
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].visible == false || camView[i].moveSteps > 0) {
            continue;
        }
        // icon
        if (x >= camView[i].iconPosX && x <= camView[i].iconPosX + ICON_WIDTH
            && y >= camView[i].iconPosY && y <= camView[i].iconPosY + ICON_HEIGHT) {
            changeCameraIcon(i + 1);
        }
        // label
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
    // overlay
    loadOverlayFont();
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 
    
}

void ofApp::exit() {
    stopRace(true);
    for (int i = 0; i < cameraNum; i++) {
        camView[i].aruco.setThreaded(false);
    }
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
        if (DEBUG_ENABLED == false) {
            ofExit();
        }
    }
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
void setupColors() {
    // common
    myColorYellow = ofColor(COLOR_YELLOW);
    myColorWhite = ofColor(COLOR_WHITE);
    myColorLGray = ofColor(COLOR_LGRAY);
    myColorLayer = ofColor(COLOR_LAYER);
    myColorAlert = ofColor(COLOR_ALERT);
    // pilot
    for (int i = 0; i < CAMERA_MAXNUM; i++) {
        switch(i) {
            case 0:
                camView[i].baseColor = ofColor(BASE_1_COLOR);
                break;
            case 1:
                camView[i].baseColor = ofColor(BASE_2_COLOR);
                break;
            case 2:
                camView[i].baseColor = ofColor(BASE_3_COLOR);
                break;
            case 3:
                camView[i].baseColor = ofColor(BASE_4_COLOR);
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
        camView[camid - 1].labelString = file.getBaseName(); // sync
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
    float ratio = (float)width / (float)height;
    switch (cameraNumVisible) {
        case 1:
            // 1st visible camera
            idx = getCameraIdxNthVisibleAll(1);
            if (idx == -1) {
                break;
            }
            camView[idx].moveSteps = MOVE_STEPS;
            if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
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
                camView[idx].widthTarget = (width / 2) - 1;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].widthTarget = (width / 2) - 1;
                camView[idx].heightTarget = camView[idx].widthTarget / CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = (height / 2) - (camView[idx].heightTarget / 2);
            } else { // solo mode on
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                    || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
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
                if (cameraTrimEnabled == true) {
                    camView[idx].heightTarget = height * 0.55;
                } else {
                    camView[idx].heightTarget = (height / 2) - 1;
                }
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget / 2);
                camView[idx].posYTarget = 0;
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                if (cameraTrimEnabled == true) {
                    camView[idx].heightTarget = height * 0.55;
                } else {
                    camView[idx].heightTarget = (height / 2) - 1;
                }
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = 0;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 3rd camera
                idx = getCameraIdxNthVisibleAll(3);
                camView[idx].moveSteps = MOVE_STEPS;
                if (cameraTrimEnabled == true) {
                    camView[idx].heightTarget = height * 0.55;
                } else {
                    camView[idx].heightTarget = (height / 2) - 1;
                }
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = width - camView[idx].widthTarget;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            } else { // solo mode on
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                    || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
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
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget + 1);
                camView[idx].posYTarget = 0;
                // 2nd camera
                idx = getCameraIdxNthVisibleAll(2);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = 0;
                // 3rd camera
                idx = getCameraIdxNthVisibleAll(3);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) - (camView[idx].widthTarget + 1);
                camView[idx].posYTarget = height - camView[idx].heightTarget;
                // 4th camera
                idx = getCameraIdxNthVisibleAll(4);
                camView[idx].moveSteps = MOVE_STEPS;
                camView[idx].heightTarget = (height / 2) - 1;
                camView[idx].widthTarget = camView[idx].heightTarget * CAMERA_RATIO;
                camView[idx].posXTarget = (width / 2) + 1;
                camView[idx].posYTarget = height - camView[idx].heightTarget;
            } else { // solo mode off
                // main camera
                idx = cameraIdxSolo;
                camView[idx].moveSteps = MOVE_STEPS;
                if ((ratio > CAMERA_RATIO && cameraTrimEnabled == true)
                    || (ratio <= CAMERA_RATIO && cameraTrimEnabled == false)) {
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
        camView[idx].imageScale = (float)(camView[idx].width) / (float)CAMERA_WIDTH;
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
        camView[idx].imageScale = (float)(camView[idx].width) / (float)CAMERA_WIDTH;
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
    cameraTrimEnabled = DFLT_CAM_TRIM;
    fullscreenEnabled = DFLT_FSCR_ENBLD;
    cameraLapHistEnabled = DFLT_CAM_LAPHST;
    // speech
    oscSpeechEnabled = DFLT_SPCH_ENBLD;
    speechLangJpn = DFLT_SPCH_JPN;
    // AR lap timer
    overlayMode = OVLAY_NONE;
    arLapMode = DFLT_ARAP_MODE;
    lockOnEnabled = DFLT_ARAP_LCKON;
    minLapTime = DFLT_ARAP_MNLAP;
    raceDuraLaps = DFLT_ARAP_RLAPS;
    raceDuraSecs = DFLT_ARAP_RSECS;
    nextSpeechRemainSecs = -1;
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
        speakLap(camid, argfloat, 0);
    }
    int idx = camid - 1;
    int total = camView[idx].totalLaps + 1;
    // camView[idx].prevElapsedSec = elp;
    camView[idx].totalLaps = total;
    camView[idx].lastLap = argfloat;
    camView[idx].lapHistoryName[total - 1] = camView[idx].labelString;
    camView[idx].lapHistoryTime[total - 1] = argfloat;
}

//--------------------------------------------------------------
void toggleOscSpeech() {
    oscSpeechEnabled = !oscSpeechEnabled;
    if (oscSpeechEnabled == true) {
        ofSystemAlertDialog("OSC speech: On");
    } else {
        ofSystemAlertDialog("OSC speech: Off");
    }
}

//--------------------------------------------------------------
void toggleSpeechLang() {
    speechLangJpn = !speechLangJpn;
    if (speechLangJpn == true) {
        ofSystemAlertDialog("Speech language: Japanese");
    }
    else {
        ofSystemAlertDialog("Speech language: English");
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

#ifdef TARGET_WIN32
HRESULT cpVicehr = E_NOINTERFACE;
CComPtr<ISpVoice> cpVoice;

class sayWin : public ofThread {
public:
    void exec(string lang, string text) {
        if (FAILED(cpVicehr)) {
            cpVicehr = cpVoice.CoCreateInstance(CLSID_SpVoice);
        }
        if (SUCCEEDED(cpVicehr)) {
            if (lang == "en") {
                this->text = "<xml><lang langid=\"409\">" + text + "</lang></xml>"; // 409:English
            }
            else if (lang == "jp") {
                this->text = "<xml><lang langid=\"411\">" + text + "</lang></xml>"; // 411:Japanese
            }
            startThread();
        }
    }
private:
    void threadedFunction() {
        int iBufferSize = ::MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, (wchar_t *)NULL, 0);
        wchar_t* wpBufWString = (wchar_t*)new wchar_t[iBufferSize];
        ::MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wpBufWString, iBufferSize);
        cpVoice->Speak(wpBufWString, SPF_DEFAULT, NULL);
        delete[] wpBufWString;
    }
    string text;
};

sayWin mySayWin[SPCH_SLOT_NUM];
#endif /* TARAGET_WIN32 */

//--------------------------------------------------------------
void speakLap(int camid, float sec, int num) {
    if (camid < 1 || camid > cameraNum || sec == 0.0) {
        return;
    }
    string ssec, sout;
    ssec = getLapStr(sec);
    sout = camView[camid - 1].labelString + ", ";
    if (speechLangJpn == true) {
        sout = regex_replace(sout, regex("(Pilot)(\\d)"), "パイロット $2");
    } else {
        sout = regex_replace(sout, regex("(Pilot)(\\d)"), "$1 $2");
    }
    if (num > 0) {
        sout += (speechLangJpn == true) ? "ラップ" : "lap";
        sout += " " + ofToString(num) + ", ";
    }
    if (speechLangJpn == true) {
        sout += ofToString(int(sec)) + "秒";
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
void setNextSpeechRemainSecs(int curr) {
    int next;
    // ...180,120,60,30
    if (curr > 60) {
        if (curr % 60 == 0) {
            next = curr - 60;
        } else {
            next = (curr / 60) * 60;
        }
    } else if (curr > 30) {
        next = 30;
    } else {
        next = -1;
    }
    nextSpeechRemainSecs = next;
}

//--------------------------------------------------------------
void speakRemainTime(int sec) {
    bool jp = speechLangJpn;
    string str = "";
    if (jp == true) {
        str += "残り";
    }
    if (sec >= 60 && sec % 60 == 0) {
        // minute
        int min = sec / 60;
        str += ofToString(min);
        if (jp == true) {
            str += "分";
        } else {
            str += " minute";
            if (min != 1) {
                str += "s";
            }
        }
    } else {
        // second
        str += ofToString(sec);
        if (jp == true) {
            str += "秒";
        } else {
            str += " second";
            if (sec != 1) {
                str += "s";
            }
        }
    }
    if (jp == false) {
        str += " to go";
    }
    speakAny(jp == true ? "jp" : "en", str);
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
        startRace();
    }
    else {
        stopRace(false);
    }
}

void startRace() {
    if (raceStarted == true) {
        return;
    }
    // stop/init -> start
    finishSound.stop();
    for (int i = 0; i < cameraNum; i++) {
        camView[i].foundMarkerNum = 0;
        camView[i].foundValidMarkerNum = 0;
        camView[i].enoughMarkers = false;
        camView[i].flickerCount = 0;
        camView[i].flickerValidCount = 0;
        camView[i].prevElapsedSec = WATCH_COUNT_SEC; // countdown
        camView[i].totalLaps = 0;
        camView[i].lastLap = 0;
        for (int h = 0; h < ARAP_MAX_RLAPS; h++) {
            camView[i].lapHistoryName[h] = "";
            camView[i].lapHistoryTime[h] = 0;
        }
    }
    if (raceDuraSecs > 0) {
        setNextSpeechRemainSecs(raceDuraSecs);
    }
    raceStarted = true;
    qrEnabled = false;
    ofResetElapsedTimeCounter();
    countSound.play();
}

void stopRace(bool exit) {
    if (raceStarted == false) {
        return;
    }
    // start -> stop
    if (exit == false) {
        raceStarted = false;
        countSound.stop();
        finishSound.play();
        if (speechLangJpn == true) {
            speakAny("jp", "レース終了");
        } else {
            speakAny("en", "race finished.");
        }
        processRaceResultDisplay();
    }
    fwriteRaceResult();
}

//--------------------------------------------------------------
bool isVariousPilots(int camidx) {
    if (camidx < 0 || camidx >= cameraNum || camView[camidx].totalLaps <= 1) {
        return false;
    }
    string name = camView[camidx].lapHistoryName[0];
    for (int i = 1; i < camView[camidx].totalLaps; i++) {
        if (name != camView[camidx].lapHistoryName[i]) {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------
bool isVariousPilotsAll() {
    for (int i = 0; i < cameraNum; i++) {
        if (isVariousPilots(i) == true) {
            return true;
        }
    }
    return false;
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
        float t = camView[camidx].lapHistoryTime[i];
        if (blap == 0) {
            blap = t;
        } else if (t < blap) {
            blap = t;
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
    if (isRecordedLaps() == false) {
        return;
    }
    string newline;
#ifdef TARGET_WIN32
    newline = "\r\n";
#else /* TARGET_WIN32 */
    newline = "\n";
#endif /* TARGET_WIN32 */
    string strsumm = "Race Result" + newline + newline;
    string strlaph = "";
    string strlapb = "";
    string sep = "  ";
    int maxlap = 0;

    // SUMMARY: NAME LAPS BESTLAP TIME
    // - head
    strsumm += "- Summary -" + newline;
    strsumm += "NAME" + sep  + "LAPS" + sep + "BESTLAP" + sep + "TIME" + newline;
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
                strlapb += getLapStr(camView[i].lapHistoryTime[lap - 1]);
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
    switch (arLapMode) {
        case ARAP_MODE_NORM:
            arLapMode = ARAP_MODE_LOOSE;
            ofSystemAlertDialog("AR lap timer mode: loose");
            break;
        case ARAP_MODE_LOOSE:
            arLapMode = ARAP_MODE_OFF;
            ofSystemAlertDialog("AR lap timer mode: off");
            break;
        case ARAP_MODE_OFF:
            arLapMode = ARAP_MODE_NORM;
            ofSystemAlertDialog("AR lap timer mode: normal");
            break;
    }
}

//--------------------------------------------------------------
void toggleLockOnEffect() {
    lockOnEnabled = !lockOnEnabled;
    resetCameraSolo();
    if (lockOnEnabled == true) {
        ofSystemAlertDialog("Lock-on effect: On");
    } else {
        ofSystemAlertDialog("Lock-on effect: Off");
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
        str = ofSystemTextBoxDialog("Race time (0~" + ofToString(ARAP_MAX_RSECS) + " secs):", str);
        sec = (str == "") ? 0 : ofToInt(str);
        if (sec <= 0) {
            // no limit
            raceDuraSecs = 0;
            nextSpeechRemainSecs = -1;
            break;
        } else if (sec <= ARAP_MAX_RSECS) {
            raceDuraSecs = sec;
            int remain;
            if (raceStarted == true) {
                remain = raceDuraSecs - (elapsedTime - WATCH_COUNT_SEC);
            } else {
                remain = raceDuraSecs;
            }
            setNextSpeechRemainSecs(remain);
            break;
        } else {
            ofSystemAlertDialog("Please enter 0~" + ofToString(ARAP_MAX_RSECS)
                                + " (0/empty means no limit)");
            // retry
        }
    }
    // laps
    while (true) {
        int laps;
        str = (raceDuraLaps == 0) ? "" : ofToString(raceDuraLaps);
        str = ofSystemTextBoxDialog("Race laps (1~"  + ofToString(ARAP_MAX_RLAPS) + "):", str);
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
    cameraTrimEnabled = !cameraTrimEnabled;
    setViewParams();
}

//--------------------------------------------------------------
void loadOverlayFont() {
    int h = (ofGetHeight() - (OVLTXT_MARG * 2)) / OVLTXT_LINES * 0.7;
    if (myFontOvlayP.isLoaded()) {
        myFontOvlayP.unloadFont();
    }
    if (myFontOvlayP2x.isLoaded()) {
        myFontOvlayP2x.unloadFont();
    }
    if (myFontOvlayM.isLoaded()) {
        myFontOvlayM.unloadFont();
    }
    myFontOvlayP.load(FONT_P_FILE, h);
    myFontOvlayP2x.load(FONT_P_FILE, h * 2);
    myFontOvlayM.load(FONT_M_FILE, h);
}

//--------------------------------------------------------------
void drawStringBlock(ofxTrueTypeFontUC *font, string text,
                     int xblock, int yline, int align, int blocks, int lines) {
    int bw, bh, x, y;
    int margin = OVLTXT_MARG;
    bw = (ofGetWidth() - (margin * 2)) / blocks;
    bh = (ofGetHeight() - (margin * 2)) / lines;
    // pos-x
    switch (align) {
        case ALIGN_LEFT:
            x = bw * xblock;
            break;
        case ALIGN_CENTER:
            x = (bw * xblock) + (bw / 2) - (font->stringWidth(text) / 2);
            break;
        case ALIGN_RIGHT:
            x = (bw * xblock) + bw - font->stringWidth(text);
            break;
        default:
            return;
    }
    x += margin;
    // pos-y
    y = bh * (yline + 1) + margin;
    // draw
    font->setLineHeight(bh);
    font->drawString(text, x, y);
}

//--------------------------------------------------------------
void drawLineBlock(int xblock1, int xblock2, int yline, int blocks, int lines) {
    int bw, bh, x, y, w, h;
    int margin = OVLTXT_MARG;

    bw = (ofGetWidth() - (margin * 2)) / blocks;
    x = (bw * xblock1) + margin;
    w = bw * (xblock2 - xblock1 + 1);

    bh = (ofGetHeight() - (margin * 2)) / lines;
    y = (bh * yline) + (bh * 0.5) + margin - 1;
    h = 2;

    ofDrawRectangle(x, y, w, h);
}

//--------------------------------------------------------------
void generateDummyData() {
    // camera
    cameraNum = 4;
    camView[0].labelString = "Pilot1";
    camView[1].labelString = "Pilot2";
    camView[2].labelString = "Pilot3";
    camView[3].labelString = "Pilot4";
    // race result
    camView[0].prevElapsedSec = 180.1;
    camView[1].prevElapsedSec = 180.2;
    camView[2].prevElapsedSec = 180.3;
    camView[3].prevElapsedSec = 180.4;
    camView[0].totalLaps = ARAP_MAX_RLAPS;
    camView[1].totalLaps = ARAP_MAX_RLAPS / 2;
    camView[2].totalLaps = ARAP_MAX_RLAPS / 3;
    camView[3].totalLaps = ARAP_MAX_RLAPS / 4;
    for (int i = 0; i < ARAP_MAX_RLAPS; i++) {
        for (int j = 0; j < 4; j++) {
            camView[j].lapHistoryName[i] = camView[j].labelString + "_L" + ofToString(i);
            camView[j].lapHistoryTime[i] = 60 + (j * 0.1) + (i * 0.01);
        }
    }
}

//--------------------------------------------------------------
int getRaceResultPages() {
    int lnum = getMaxLaps();
    int pages;
    if (lnum <= 0) {
        pages = 0;
    } else {
        pages = (lnum / OVLTXT_LAPS) + ((lnum % OVLTXT_LAPS == 0) ? 0 : 1);
    }
    return pages;
}

//--------------------------------------------------------------
void processRaceResultDisplay() {
    if (isRecordedLaps() == false) { // no result
        return;
    }
    if (overlayMode != OVLAY_RACE_RSLT) {
        overlayMode = OVLAY_RACE_RSLT;
        raceResultPage = 0;
    } else {
        if ((raceResultPage + 1) >= getRaceResultPages()) {
            overlayMode = OVLAY_NONE;
            raceResultPage = 0;
        } else {
            raceResultPage++;
        }
    }
}

//--------------------------------------------------------------
void drawRaceResult(int pageidx) {
    int szb = OVLTXT_BLKS - (CAMERA_MAXNUM - cameraNum);
    int szl = OVLTXT_LINES;
    int pages, line;

    pages = getRaceResultPages();
    if (pageidx < 0 || (pageidx + 1) > pages) {
        return;
    }

    // background
    ofSetColor(myColorLayer);
    ofDrawRectangle(0, 0, ofGetWidth(), ofGetHeight());

    // title
    line = 2;
    ofSetColor(myColorYellow);
    drawStringBlock(&myFontOvlayP2x, "Race Result", 0, line, ALIGN_CENTER, 1, szl);

    // summary : name laps bestlap time
    // _header
    line = 4;
    ofSetColor(myColorWhite);
    drawStringBlock(&myFontOvlayP, "Name", 1, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Laps", 2, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "BestLap", 3, line, ALIGN_CENTER, szb, szl);
    drawStringBlock(&myFontOvlayP, "Time", 4, line, ALIGN_CENTER, szb, szl);
    line += 1;
    ofSetColor(myColorYellow);
    drawLineBlock(1, 4, line, szb, szl);
    // _body
    ofSetColor(myColorWhite);
    for (int i = 0; i < cameraNum; i++) {
        string str;
        float fval;
        // _newline
        line += 1;
        // pilot
        str = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        drawStringBlock(&myFontOvlayP, str, 1, line, ALIGN_CENTER, szb, szl);
        // laps
        str = ofToString(camView[i].totalLaps);
        drawStringBlock(&myFontOvlayM, str, 2, line, ALIGN_CENTER, szb, szl);
        // bestlap
        fval = getBestLap(i);
        str = (fval == 0) ? "-.-" : getLapStr(fval);
        drawStringBlock(&myFontOvlayM, str, 3, line, ALIGN_CENTER, szb, szl);
        // time
        fval = camView[i].prevElapsedSec - WATCH_COUNT_SEC;
        str = (fval <= 0) ? "-:-.-" : getWatchString(fval);
        drawStringBlock(&myFontOvlayM, str, 4, line, ALIGN_CENTER, szb, szl);
    }

    // laptimes : lap p1 p2 p3 p4
    // _header
    int xoff = 6;
    line = 4;
    drawStringBlock(&myFontOvlayP, "Lap", xoff, line, ALIGN_CENTER, szb, szl);
    for (int i = 0; i < cameraNum; i++) {
        string pilot = (camView[i].labelString == "") ? ("Pilot" + ofToString(i + 1)) : camView[i].labelString;
        drawStringBlock(&myFontOvlayP, pilot, xoff + i + 1, line, ALIGN_CENTER, szb, szl);
    }
    line += 1;
    ofSetColor(myColorYellow);
    drawLineBlock(xoff, xoff + cameraNum, line, szb, szl);
    // _body
    ofSetColor(myColorWhite);
    int lapidx = pageidx * OVLTXT_LAPS;
    int lnum = getMaxLaps();
    for (int cnt = 0; cnt < OVLTXT_LAPS; cnt++) {
        if ((lapidx + 1) > lnum) {
            break;
        }
        line += 1;
        drawStringBlock(&myFontOvlayM, ofToString(lapidx + 1), xoff, line, ALIGN_CENTER, szb, szl);
        for (int i = 0; i < cameraNum; i++) {
            string str;
            if ((lapidx + 1) > camView[i].totalLaps) {
                str = "-.-";
            } else {
                str = getLapStr(camView[i].lapHistoryTime[lapidx]);
            }
            drawStringBlock(&myFontOvlayM, str,
                            xoff + i + 1, line, ALIGN_CENTER, szb, szl);
        }
        lapidx += 1;
    }
    // _page
    if (pages > 1) {
        line += 1;
        ofSetColor(myColorLGray);
        drawStringBlock(&myFontOvlayP,
                        "(Page " + ofToString(pageidx + 1) + " of " + ofToString(pages) + ")",
                        xoff + 2, line, ALIGN_CENTER, szb, szl);
    }

    // message
    line = OVLTXT_LINES - 1;
    ofSetColor(myColorLGray);
    drawStringBlock(&myFontOvlayP, "Press V key to continue...", 0, line, ALIGN_CENTER, 1, szl);
}

#ifdef TARGET_WIN32
//--------------------------------------------------------------
string utf8ToAnsi(string utf8) {
    int ulen, alen;
    wchar_t* ubuf;
    char* abuf;
    string ansi;

    // utf8 -> wchar
    ulen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size() + 1, NULL, 0);
    ubuf = new wchar_t[ulen];
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size() + 1, ubuf, ulen);
    // wchar -> ansi
    alen = WideCharToMultiByte(CP_ACP, 0, ubuf, -1, NULL, 0, NULL, NULL);
    abuf = new char[alen];
    WideCharToMultiByte(CP_ACP, 0, ubuf, ulen, abuf, alen, NULL, NULL);
    ansi = abuf;

    delete[] ubuf;
    delete[] abuf;
    return ansi;
}
#endif /* TARGET_WIN32 */

//--------------------------------------------------------------
void toggleQrReader() {
    if (qrEnabled == false) {
        // start
        qrUpdCount = 1;
        qrCamIndex = 0;
        for (int i = 0; i < cameraNum; i++) {
            camView[i].qrScanned = false;
        }
        qrEnabled = true;
    }
    else {
        // stop
        qrEnabled = false;
    }
    elapsedTime = 0;
}

//--------------------------------------------------------------
void processQrReader() {
    if (qrUpdCount == QR_CYCLE) {
        bool scanned = false;
        if (camView[qrCamIndex].qrScanned == false) {
            ofxZxing::Result zxres;
            zxres = ofxZxing::decode(grabber[qrCamIndex].getPixels(), true);
            if (zxres.getFound()) {
                scanned = true;
                camView[qrCamIndex].qrScanned = true;
                beepSound.play();
                string label = zxres.getText();
#ifdef TARGET_WIN32
                label = utf8ToAnsi(label);
#endif /* TARGET_WIN32 */
                camView[qrCamIndex].labelString = label;
                autoSelectCameraIcon(qrCamIndex + 1, label);
            }
        }
        qrCamIndex++;
        if (qrCamIndex == cameraNum) {
            qrCamIndex = 0;
        }
        qrUpdCount = 1;
        if (scanned == false) {
            return;
        }
    }
    else {
        qrUpdCount++;
        return;
    }
    // scanned
    int count = 0;
    for (int i = 0; i < cameraNum; i++) {
        if (camView[i].qrScanned == true) {
            count++;
        }
    }
    if (count == cameraNum) {
        toggleQrReader(); // finished
    }
}

//--------------------------------------------------------------
void toggleLapHistory() {
    cameraLapHistEnabled = !cameraLapHistEnabled;
}
