# Tiny View Plus

Tiny View Plus is an application to display FPV video easily by using receivers compatible with UVC. Up to 4 receivers can be connected. It is implemented using openFrameworks and it operates at high speed.

![tinyviewplus](docs/overview.jpg)

## Environment

<table>
<tr>
<td>OS</td><td>macOS (High Sierra is recommended)<br/>Windows (10 64bit is recommended)</td>
</tr>
<tr>
<td>FPV Receiver</td><td>Eachine ROTG02<br/>Eachine ROTG01</td>
</tr>
</table>

To build from source code, [openFrameworks](http://openframeworks.cc/ja/) v0.10.0 is required.

## Install

Attention. We are not responsible for any problems caused by running the application.

### Case 1: Prebuilt binary

1. Download a zip file from [Release page](https://github.com/t-asano/tinyviewplus/releases).
2. Extract it.

### Case 2: Build from source code (for experts)

1. Install [openFrameworks](http://openframeworks.cc/ja/) v0.10.0.
2. Install [ofxTrueTypeFontUC addon](https://github.com/hironishihara/ofxTrueTypeFontUC)
3. Create a project with ofxOsc and ofxTrueTypeFontUC by Project Generator.
4. Overwrite src/ and bin/data/.
5. [Fix build settings for macOS](https://github.com/openframeworks/openFrameworks/commit/da6929bf6dc529a1082224a7db249e8a073b2361)
6. Build.

## Usage

### Launch

1. Connect receivers(Up to 4) to computer.
2. Launch application.

### Detection of receivers

If receivers are not successfully detected, please try following operation.

1. Exit the application.
2. Remove all receivers.
3. If using a USB hub, reconnect it or remove it.
4. Connect receivers one by one at intervals of several seconds.
5. Launch the application.

On macOS, you can check a list of cameras (receivers) with the QuickTime. If you are using a USB hub and the picture drops, please reduce the number of receivers per USB hub.

On Windows, you can check the detection status of the camera (receiver) with the device manager. In addition, if multiple receivers are connected to same USB hub, only one of them may operate.

### Keyboard shortcuts
                        
| key | Function | Default value |
|---|---|---|
| H | Display help | - |
| 1~4 | Toggle camera 1~4 on/off | on |
| Q,W,E,R | Change camera 1~4 icon | built-in image |
| L | Change camera label | Pilot1~4 |
| B | Change background image | built-in image |
| S | Toggle speech on/off (only for macOS) | off |
| I | Initialize configuration (※1) | Default value of each setting |

- Camera icon is also automatically changed in conjunction with the camera label change.
	- If {label string}.png or {label string}.jpg was found under "Tiny View Plus.app / Contents / Presents / data / pilots" folder (on macOS binary), otherwise under the "data / pilots" folder,	it is adopted as an icon image in this priority order.
	- If no image file is found, the default icon will be adopted.
	- The aspect ratio is forced to 1:1.
- The background image is scaled according to the screen, and the upper left is displayed in priority.
- Speech function works when linked with external application.
- (* 1) The following settings will be initialized.
	- Camera on / off
	- Camera icon
	- Camera label
	- Backgroud image
	- Speech on / off
- When you exit the application settings will be initialized.
- Camera audio is not supported.

## Control by OSC

Tiny View Plus can be controlled by OSC protocol. It receives OSC packet at UDP port 4000.

### Message format

#### Toggle camera on / off

/v1/camera/{id}/display {switch}

- Parameters
	- id ... Number from 1 to 4
	- switch ... "on" or "off"
- [Example] camera 1 off
	- /v1/camera/1/display "off"

#### Change camera label

/v1/camera/{id}/label {label}

- Parameters
	- id ... Number from 1 to 4
	- label ... Any string (such as pilot name)
- [Example] Set the label of camera 2 to "Whooper 2"
	- /v1/camera/2/label "Whooper 2"

Camera icon is also automatically changed in conjunction with the camera label change.

- If {label string}.png or {label string}.jpg was found under "Tiny View Plus.app / Contents / Presents / data / pilots" folder (on macOS binary), otherwise under the "data / pilots" folder,	it is adopted as an icon image in this priority order.
- If no image file is found, the default icon will be adopted.
- The aspect ratio is forced to 1:1.

#### Set camera laptime

/v1/camera/{id}/laptime {time}

- Parameters
	- id ... Number from 1 to 4
	- label ... Number indicating the lap time (seconds)
- [Example] Set the lap time of camera 3 to 128.64 seconds
	- /v1/camera/3/laptime 128.64
- [Example] Clear the lap time of camera 4
	- /v1/camera/4/laptime 0

#### Speech synthesis (only for macOS)

/v1/speech/{language}/say {text}

- Parameters
	- language ... "en" (English) or "jp" (Japanese)
	- text ... sentence to speak
- [Example] Say "We can fly!" in English
	- /v1/speeech/en/say "We can fly!"

### Test

You can check the operation with Node.js as follows.

```js
var osc = require('node-osc');
var client = new osc.Client('127.0.0.1', 4000);
client.send("/v1/camera/1/label", "Whooper1", function () {});
client.send("/v1/camera/1/laptime", 62.09, function () {});
```

## License

Tiny View Plus is distributed under the MIT License. This gives everyone the freedoms to use Tiny View Plus in any context: commercial or non-commercial, public or private, open or closed source. Please see [LICENSE](LICENSE) and [LICENSE_THIRD_PARTY](LICENSE_THIRD_PARTY) for details.