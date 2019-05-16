# OSC API for Tiny View Plus

Tiny View Plus can be controlled by OSC protocol. It receives OSC packet at UDP port 4000.

## Message format

### Set camera enhanced view on / off

/v1/camera/{id}/solo {switch}

- Parameters
	- id ... Camera ID(1~4)
	- switch ... "on" or "off"
- Example: Camera 1 enhanced view on
    - /v1/camera/1/solo "on"

### Set camera display on / off

/v1/camera/{id}/display {switch}

- Parameters
	- id ... Camera ID(1~4)
	- switch ... "on" or "off"
- Example: Camera 1 off
	- /v1/camera/1/display "off"

### Set camera label

/v1/camera/{id}/label {label}

- Parameters
	- id ... Camera ID(1~4)
	- label ... Any string (such as pilot name)
- Example: Set the label of camera 2 to "Whooper 2"
	- /v1/camera/2/label "Whooper 2"

Camera icon is also automatically changed in conjunction with the camera label change.

- If {label string}.png or {label string}.jpg was found under "Tiny View Plus.app / Contents / Presents / data / pilots" folder (on macOS binary), otherwise under the "data / pilots" folder,	it is adopted as an icon image in this priority order.
- If no image file is found, the default icon will be adopted.
- The aspect ratio is forced to 1:1.

### Set laptime (Beta)

/v1/camera/{id}/laptime {time}

- Parameters
	- id ... Camera ID(1~4)
	- time ... lap time(seconds)
- Example: Set the lap time of camera 3 to 128.64 seconds
	- /v1/camera/3/laptime 128.64

### Speech synthesis

/v1/speech/{language}/say {text}

- Parameters
	- language ... "en"(English) or "jp"(Japanese)
	- text ... sentence to speak
- Example: Say "We can fly!" in English
	- /v1/speeech/en/say "We can fly!"