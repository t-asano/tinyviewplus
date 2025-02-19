# OSC API for Tiny View Plus

Tiny View Plus can be controlled and monitored by OSC protocol.

## Controlling(receiver)

It receives OSC packet at UDP port 4000.

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

- If {label string}.png or {label string}.jpg was found under "Tiny View Plus.app / Contents / Presents / data / pilots" folder (on macOS binary), otherwise under the "data / pilots" folder,    it is adopted as an icon image in this priority order.
- If no image file is found, the default icon will be adopted.
- The aspect ratio is forced to 1:1.

### Set laptime

/v1/camera/{id}/laptime {time}

- Parameters
  - id ... Camera ID(1~4)
  - time ... lap time(seconds)
- Example: Set the lap time of camera 3 to 128.64 seconds
  - /v1/camera/3/laptime 128.64

### Add/Delete lap

/v1/camera/{id}/lap {command}

- Parameters
  - id ... Camera ID(1～4)
  - command ... "add" or "del"
- Example: Add lap of camera 3
  - /v1/camera/3/lap "add"

### Speech synthesis

/v1/speech/{language}/say {text}

- Parameters
  - language ... "en"(English) or "jp"(Japanese)
  - text ... sentence to speak
- Example: Say "We can fly!" in English
  - /v1/speeech/en/say "We can fly!"

## Monitoring(sender) ※experimental

It sends OSC packet to the host/port specified in "settings.xml".

```xml
<oscMonitor>
    <enabled>1</enabled>
    <host>127.0.0.1</host>
    <port>4001</port>
</oscMonitor>
```

Set "enabled" to 1 to enable this feature. Set to 0 to disable.

### System information notification

/v1/system/info {info}

- Parameters
  - info ... System information
- Example: "Hello"
  - /v1/system/info "hello"

### Race event notifiacation

/v1/race/event {event}

- Parameters
  - event ... "started" or "finished"
- Example: Race has started
  - /v1/race/event "started"

### Lap notification

/v1/camera/{id}/lap {lapnum} {laptime} {label}

- Parameters
  - id ... Camera ID(1～4)
  - lapnum ... Lap number
  - laptime ... Lap time(seconds)
  - label ... Camera label
- Example: Notify lap of camera 3(lap 5, 10.2 seconds)
  - /v1/camera/3/lap 5 10.2 "Whooper 3"

### Lap delete notification

/v1/camera/{id}/lapdel {lapnum}

- Parameters
  - id ... Camera ID(1～4)
  - lapnum ... Lap number
- Example: Notify lap delete of camera 3(lap 5)
  - /v1/camera/3/lapdel 5

### Camera label notification

/v1/camera/{id}/label {label}

- Parameters
  - id ... Camera ID(1～4)
  - label ... Camera label
- Example: Notify the label of camera 2
  - /v1/camera/2/label "Whooper 2"

This works at race start and label change.