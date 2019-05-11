# Tiny View Plus

Tiny View Plus is an application to display FPV video easily with USB receivers. Up to 4 receivers(\*) can be connected.

![tinyviewplus](docs/img/overview.jpg)

**(\*)On Windows, it is impossible to connect multiple receivers to the USB hub. Please connect only one receiver to each USB port built in PC.**

## Environment

<table>
<tr>
<td>OS</td><td>macOS (Mojave is recommended)<br/>Windows (10 64bit is recommended)</td>
</tr>
<tr>
<td>FPV Receiver</td><td>Eachine ROTG02<br/>Eachine ROTG01</td>
</tr>
</table>

To build from source code, [openFrameworks](http://openframeworks.cc/ja/) v0.10.1 is required.

## Install

Attention: We are not responsible for any problems caused by running the application.

If you want to build from source code, please refer to [this document](docs/HowToBuild_en.md).

### macOS

1. Download a zip file from [Release page](https://github.com/t-asano/tinyviewplus/releases).
2. Extract it and place its contents in the proper place.
    - Be sure to put the application under the "Application" folder. If you place it anywhere else, functions that writing to file will not work correctly.
    ![install_mac.png](docs/img/install_mac.png)

### Windows

1. Download a zip file from [Release page](https://github.com/t-asano/tinyviewplus/releases).
2. Extract it and place its contents in the proper place.
3. Install [Microsoft Visual C++ 2015 Redistributable(vc_redist.x64.exe)
](https://www.microsoft.com/ja-jp/download/details.aspx?id=53587)(Only if VCOMP140.DLL error occurs at startup.)

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
| 1~4 | Camera 1~4 solo view on/off | off |
| T | Camera triming on/off (\*1) | off |
| Shift + 1~4 | Camera 1~4 display on/off | on |
| Q | Start/Stop QR code reader (\*2) | - |
| B | Change background image | built-in image |
| F | Fullscreen mode on/off | off |
| L | Lap history display during race on/off | off |
| A | Switch AR lap timer mode (normal/loose/off) (\*3) | normal |
| O | Lock-on effect on/off (\*4) | off |
| Space | Start/Stop race | - |
| V | Display race results (\*5) | - |
| D | Set race duration time (0~36,000sec) and laps (1\~10,000) | 0(no limit), 10 |
| M | Set minimum lap time (1~100sec) | 3 |
| N | Change speech language (Japanese/English) | Japanese |
| S | OSC speech on/off | off |
| I | Initialize configuration | - |
| . | Exit application | - |

- (\*1) When 1 or 3 cameras are connected, the image will be cropped and displayed as large as possible.
- (\*2) Sets the characters scanned from the QR code to the camera label.
- The background image is scaled according to the screen, and the upper left is displayed in priority.
- (\*3) In loose mode, lap time will be measured even if the drone passes outside the gate.
- (\*4) If the difference in gate passing time is less than 1 second, tracking side camera display will be enlarged.
- (\*5) Race results will be saved under predefined folder when race finish.
	- macOS binary: Tiny View Plus.app/Contents/ Resources/data/pilots
	- others: data/pilots
- When you exit the application settings will be initialized.
- Camera audio is not supported.

### Mouse operation

Some functions can also be operated with the mouse.

| Operation | Function |
| --- | --- |
| Click at camera 1~4 icon | Change icon (\*1) |
| Click at camera 1~4 label | Change label (\*2) |
| Shift key + Click at camaera 1~4 | Cancel previous lap time |

- (\*1) Camera label will be automatically changed according to changing camera icon.
	- The file name of the icon (excluding the extension) will be adopted.
- (\*2) Camera icon will be automatically changed according to changing camera label.
	- If {label string}.png or {label string}.jpg was found under predefined folder, it will be adopted as an icon image in this priority order.
		- macOS binary: Tiny View Plus.app/Contents/ Resources/data/pilots
		- others: data/pilots
	- If no image file is found, the default icon will be adopted.
	- The aspect ratio is forced to 1:1.

### QR Code for label setting

You can set the camera label by the QR Code.

![qr_screen.png](docs/img/qr_screen.png)

You can use Google Charts API to create QR Code. An example of URL is as follows.

[https://chart.apis.google.com/chart?cht=qr&chs=500x500&chl=TinyViewPlus](https://chart.apis.google.com/chart?cht=qr&chs=500x500&chl=TinyViewPlus)

It is convenient to embed the QR Code in the OSD. An example of an image for the Betaflight is as follows.

![qr_betaflight.png](docs/img/qr_betaflight.png)

\* "QR Code" is a registered trademark of DENSO WAVE INCORPORATED.

### AR lap timer

You can measure the lap time by using AR marker.

![argate_single.png](docs/img/argate_single.png)
![argate_multi.png](docs/img/argate_multi.png)

Please place 4 to 8 markers around the gate. Also make sure that the top of the marker faces the center of the gate. Following four types of markers are supported. It does not matter if only one type is placed as shown above.

- [marker_00_main_a.png](docs/img/marker_00_main_a.png)
- [marker_01_main_b.png](docs/img/marker_01_main_b.png)
- [marker_02_main_c.png](docs/img/marker_02_main_c.png)
- [marker_03_main_d.png](docs/img/marker_03_main_d.png)

The size of the marker should be 150mm per side. Larger marker will be suitable for high speed race. If marker recognition is not successful, please try increasing the size of the marker, using multiple types, or placing them in a brighter place.

Lap time will be measured only during the race. After detecting two or more correctly oriented markers simultaneously, when the marker disappears from the screen, lap time will be confirmed. However, if the direction of the last displayed marker is incorrect, the measurement will be canceled.

Depending on the environment, recognition processing of markers might be heavy. In that case, please disable the function.

## Control by OSC

Tiny View Plus can be controlled by OSC protocol. It receives OSC packet at UDP port 4000.

### Message format

#### Toggle camera solo mode on / off

/v1/camera/{id}/solo {switch}

- Parameters
	- id ... Number from 1 to 4
	- switch ... "on" or "off"
- [Example] camera 1 solo mode on
    - /v1/camera/1/solo "on"

#### Toggle camera visibility on / off

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
	- time ... Number indicating the lap time (seconds)
- [Example] Set the lap time of camera 3 to 128.64 seconds
	- /v1/camera/3/laptime 128.64
- [Example] Clear the lap time of camera 4
	- /v1/camera/4/laptime 0

#### Speech synthesis

/v1/speech/{language}/say {text}

- Parameters
	- language ... "en" (English) or "jp" (Japanese)
	- text ... sentence to speak
- [Example] Say "We can fly!" in English
	- /v1/speeech/en/say "We can fly!"

## License

Tiny View Plus is distributed under the MIT License. This gives everyone the freedoms to use Tiny View Plus in any context: commercial or non-commercial, public or private, open or closed source. Please see [LICENSE.md](LICENSE.md) and [LICENSE\_THIRD\_PARTY.md](LICENSE_THIRD_PARTY.md) for details.