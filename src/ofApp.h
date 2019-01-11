#pragma once

#include <regex>
#include "ofMain.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOsc.h"
#include "ofxAruco.h"
#include "highlyreliablemarkers.h"

/* ---------- definitions ---------- */

// view
#define FRAME_RATE      60
#define MOVE_STEPS      10
#define VERTICAL_SYNC   true
#define WALL_FILE       "system/background.png"
#define CAMERA_MAXNUM   4
#define CAMERA_WIDTH    640
#define CAMERA_HEIGHT   480
#define CAMERA_RATIO    1.3333
#define FONT_P_FILE     "system/mplus-1p-bold.ttf"
#define FONT_M_FILE     "system/mplus-1m-bold.ttf"
#define ICON_FILE       "system/pilot_icon.png"
#define ICON_DIR        "pilots/"
#define ICON_WIDTH      50
#define ICON_HEIGHT     50
#define ICON_MARGIN_X   20
#define ICON_MARGIN_Y   0
#define NUMBER_HEIGHT   20
#define NUMBER_MARGIN_X 1
#define NUMBER_MARGIN_Y 35
#define LABEL_HEIGHT    30
#define LABEL_MARGIN_X  75
#define LABEL_MARGIN_Y  40
#define BASE_MARGIN_X   0
#define BASE_MARGIN_Y   0
#define BASE_WIDTH      20
#define BASE_HEIGHT     50
#define BASE_1_RED      201
#define BASE_1_GREEN    58
#define BASE_1_BLUE     64
#define BASE_2_RED      160
#define BASE_2_GREEN    194
#define BASE_2_BLUE     56
#define BASE_3_RED      0
#define BASE_3_GREEN    116
#define BASE_3_BLUE     191
#define BASE_4_RED      248
#define BASE_4_GREEN    128
#define BASE_4_BLUE     23
#define LAP_HEIGHT      20
#define LAP_MARGIN_X    20
#define LAP_MARGIN_Y    80
// AR lap timer
#define DFLT_ARAP_ENBLD true
#define DFLT_ARAP_RLAPS 10
#define DFLT_ARAP_RSECS 0
#define DFLT_ARAP_MNLAP 3
#define DFLT_ARAP_LCKON false
#define SND_BEEP_FILE   "system/beep.wav"
#define SND_COUNT_FILE  "system/count.wav"
#define SND_FINISH_FILE "system/finish.wav"
#define SND_LOCKON_FILE "system/lockon.wav"
#define ARAP_MKR_FILE   "system/marker.xml"
#define ARAP_RESULT_DIR "results/"
#define ARAP_MNUM_THR   2
#define ARAP_MAX_RLAPS  100
#define ARAP_MAX_MNLAP  100
#define ARAP_MAX_RSECS  3600
#define ARAP_LOCKON_SEC 1
#define ARAP_RSLT_SCRN  0
#define ARAP_RSLT_FILE  1
#define WATCH_COUNT_SEC 5
#define WATCH_HEIGHT    15
// osc
#define OSC_LISTEN_PORT 4000
// speech
#define DFLT_SPCH_ENBLD false
#define DFLT_SPCH_JPN   true
#define SPCH_SLOT_NUM	8
// help
#define HELP_MESSAGE    "Keyboard shortcuts:\n"\
                        "[H] Display help\n"\
                        "[1~4] Camera 1~4 solo mode on/off\n"\
                        "[Shift + 1~4] Camera 1~4 on/off\n"\
                        "[Q,W,E,R] Change camera 1~4 icon\n"\
                        "[L] Change camera label\n"\
                        "[B] Change background image\n"\
                        "[A] AR lap timer on/off\n"\
                        "[O] Lock-on effect on/off\n"\
                        "[Space] Start/Stop race\n"\
                        "[V] Display race results\n"\
                        "[D] Set race duration (time/laps)\n"\
                        "[M] Set minimum lap time (1~100sec)\n"\
                        "[N] Change speech language\n"\
                        "[S] OSC speech on/off\n"\
                        "[I] Initialize configuration\n"\
                        "[.] Exit application\n"

/* ---------- classes ---------- */

class tvpCamView {
public:
    // camera
    bool visible;
    int moveSteps;
    int width;
    int height;
    int widthTarget;
    int heightTarget;
    int posX;
    int posY;
    int posXTarget;
    int posYTarget;
    // base
    ofColor baseColor;
    int basePosX;
    int basePosY;
    int basePosXTarget;
    int basePosYTarget;
    int baseWidth;
    int baseHeight;
    // number
    int numberPosX;
    int numberPosY;
    int numberPosXTarget;
    int numberPosYTarget;
    // icon
    ofImage iconImage;
    int iconPosX;
    int iconPosY;
    int iconPosXTarget;
    int iconPosYTarget;
    // label
    string labelString;
    int labelPosX;
    int labelPosY;
    int labelPosXTarget;
    int labelPosYTarget;
    // lap
    int lapPosX;
    int lapPosY;
    int lapPosXTarget;
    int lapPosYTarget;
    float lastLap;
    // AR lap timer
    ofxAruco aruco;
    int foundMarkerNum;
    float prevElapsedSec;
    int totalLaps;
    float lapHistory[ARAP_MAX_RLAPS];
};

#ifdef TARGET_WIN32
// speech
class sayWin : public ofThread {
public:
    void exec(string lang, string text) {
        this->text = text;
        this->lang = lang;
        startThread();
    }
private:
    void threadedFunction() {
        ofSystem("cscript data\\system\\saywin.js " + lang + " \"" + text + "\"");
    }
    string text;
    string lang;
};
#endif /* TARAGET_WIN32 */

class ofApp : public ofBaseApp {
public:
    void setup();
    void update();
    void draw();
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void mouseEntered(int x, int y);
    void mouseExited(int x, int y);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
};

/* ---------- functions ---------- */

void bindCameras();
void toggleCameraSolo(int);
void enableCameraSolo(int);
void resetCameraSolo();
void toggleCameraVisibility(int);
int getCameraIdxNthVisibleAll(int);
int getCameraIdxNthVisibleSub(int);
void setupBaseColors();
void changeCameraLabel(int);
void changeCameraLabelAll();
void changeCameraIcon(int);
void changeCameraIconPath(int, string);
void autoSelectCameraIcon(int, string);
void changeWallImage();
void setWallParams();
void setViewParams();
int calcViewParam(int, int, int);
void updateViewParams();
void initConfig();
void recvOsc();
void recvOscCameraString(int, string, string);
void recvOscCameraFloat(int, string, float);
void toggleOscSpeech();
void toggleSpeechLang();
void recvOscSpeech(string, string);
void speakLap(int, float);
void speakAny(string, string);
void drawCamera(int);
string getWatchString(float);
void drawWatch();
void toggleRace();
bool isRecordedLaps();
float getBestLap(int);
int getMaxLaps();
string getLapStr(float);
void printRaceResults(int);
void toggleARLap();
void toggleLockOnEffect();
void changeMinLap();
void changeRaceDuration();
