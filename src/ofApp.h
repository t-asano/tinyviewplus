#pragma once

#include <regex>
#include "ofMain.h"
#include "ofxTrueTypeFontUC.h"
#include "ofxOsc.h"
#include "ofxAruco.h"
#include "highlyreliablemarkers.h"
#include "ofxZxing.h"

/* ---------- definitions ---------- */

// system
#define APP_INFO        "Tiny View Plus v0.9.15"
#define DEBUG_ENABLED   false
#define COLOR_YELLOW    255,215,0
#define COLOR_WHITE     255,255,255
#define COLOR_LGRAY     127,127,127
#define COLOR_BG_DARK   0,0,0,223
#define COLOR_BG_LIGHT  0,0,0,31
#define COLOR_ALERT     255,0,0
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
#define BASE_1_COLOR    201,58,64
#define BASE_2_COLOR    160,194,56
#define BASE_3_COLOR    0,116,191
#define BASE_4_COLOR    248,128,23
#define LAP_HEIGHT      20
#define LAP_MARGIN_X    20
#define LAP_MARGIN_Y    80
#define LAPHIST_HEIGHT  10
#define DFLT_CAM_TRIM   false
#define DFLT_FSCR_ENBLD false
#define DFLT_CAM_LAPHST false
#define ALIGN_LEFT      0
#define ALIGN_CENTER    1
#define ALIGN_RIGHT     2
// overlay
#define OVLMODE_NONE    0
#define OVLMODE_HELP    1
#define OVLMODE_MSG     2
#define OVLMODE_RCRSLT  3
#define OVLTXT_BLKS     12
#define OVLTXT_LINES    34
#define OVLTXT_LAPS     25
#define OVLTXT_MARG     10
#define OLVMSG_TIME     FRAME_RATE
// AR lap timer
#define ARAP_MODE_NORM  0
#define ARAP_MODE_LOOSE 1
#define ARAP_MODE_OFF   2
#define DFLT_ARAP_MODE  ARAP_MODE_NORM
#define DFLT_ARAP_RLAPS 10
#define DFLT_ARAP_RSECS 0
#define DFLT_ARAP_MNLAP 3
#define DFLT_ARAP_LCKON false
#define SND_BEEP_FILE   "system/beep.wav"
#define SND_BEEP3_FILE  "system/beep3.wav"
#define SND_COUNT_FILE  "system/count.wav"
#define SND_FINISH_FILE "system/finish.wav"
#define SND_NOTIFY_FILE "system/notify.wav"
#define ARAP_MKR_FILE   "system/marker.xml"
#define ARAP_RESULT_DIR "results/"
#define ARAP_MNUM_THR   2
#define ARAP_MAX_RLAPS  1000
#define ARAP_MAX_MNLAP  100
#define ARAP_MAX_RSECS  36000
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
// QR code reader
#define QR_CYCLE        6
// help
#define HELP_LINES      24
#define HELP_MESSAGE    "Keyboard shortcuts:\n"\
                        "[H] Display help\n"\
                        "[1~4] Camera 1~4 solo view on/off\n"\
                        "[T] Solo view trimming on/off\n"\
                        "[Shift + 1~4] Camera 1~4 on/off\n"\
                        "[Q] Start/Stop QR reader for camera label\n"\
                        "[B] Change background image\n"\
                        "[F] Fullscreen mode on/off\n"\
                        "[A] AR lap timer on/loose/off\n"\
                        "[O] Lock-on effect on/off\n"\
                        "[Space] Start/Stop race\n"\
                        "[V] Display race result\n"\
                        "[D] Set race duration (time/laps)\n"\
                        "[M] Set minimum lap time (1~100sec)\n"\
                        "[N] Change speech language\n"\
                        "[S] OSC speech on/off\n"\
                        "[I] Initialize configuration\n"\
                        "[.] Exit application"

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
    float imageScale;
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
    int foundValidMarkerNum;
    bool enoughMarkers;
    float prevElapsedSec;
    int totalLaps;
    string lapHistoryName[ARAP_MAX_RLAPS];
    float lapHistoryTime[ARAP_MAX_RLAPS];
    int flickerCount;
    int flickerValidCount;
    // QR reader
    bool qrScanned;
};

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
    void exit();
};

/* ---------- functions ---------- */

void bindCameras();
void toggleCameraSolo(int);
void enableCameraSolo(int);
void resetCameraSolo();
void toggleCameraVisibility(int);
int getCameraIdxNthVisibleAll(int);
int getCameraIdxNthVisibleSub(int);
void setupColors();
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
void speakLap(int, float, int);
void setNextSpeechRemainSecs(int);
void speakRemainTime(int);
void speakAny(string, string);
void drawCameraImage(int);
void drawCameraARMarker(int, bool);
void drawCameraPilot(int, bool);
void drawCameraLapTime(int, bool);
void drawCameraLapHistory(int);
void drawCamera(int);
string getWatchString(float);
void drawWatch();
void keyPressedOverlayHelp(int);
void keyPressedOverlayHelp(int);
void keyPressedOverlayResult(int);
void keyPressedOverlayNone(int);
void mouseReleasedOverlayNone(int, int, int);
void toggleRace();
void startRace();
void stopRace(bool);
bool isVariousPilots(int);
bool isVariousPilotsAll();
bool isRecordedLaps();
float getBestLap(int);
int getMaxLaps();
string getLapStr(float);
void toggleARLap();
void toggleLockOnEffect();
void changeMinLap();
void changeRaceDuration();
void toggleFullscreen();
void toggleSoloTrim();
// overlay - common
void setOverlayMode(int);
void loadOverlayFont();
void drawStringBlock(ofxTrueTypeFontUC*, string, int , int, int, int, int);
void drawLineBlock(int, int, int, int, int);
// overlay - race result
void generateDummyData();
void fwriteRaceResult();
int getRaceResultPages();
void processRaceResultDisplay();
void drawRaceResult(int);
// overlay - help
void drawHelp();
// overlay - message
void initOverlayMessage();
void setOverlayMessage(string);
void drawOverlayMessage();
// QR code reader
#ifdef TARGET_WIN32
string utf8ToAnsi(string);
#endif /* TARGET_WIN32 */
void toggleQrReader();
void processQrReader();
void toggleLapHistory();
