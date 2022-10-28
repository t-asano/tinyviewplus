// OSC debugger

var osc = require('node-osc');
var client = new osc.Client('127.0.0.1', 4000);

function sendOsc(addr, arg1, wait) {
  var is_str = (typeof arg1 === 'string' || arg1 instanceof String);
  var ext = is_str ? '"' : '';
  setTimeout(function () {
    console.log(addr + ' ' + ext + arg1 + ext);
    client.send(addr, arg1, function () {});
  }, wait);
}

function exit() {
  client.kill();
  process.exit;
}

function test() {
  var wait = 0;
  var step = 800;
  var step_lap = 3000;
  var step_spc = 1200;
  console.log('Sending OSC message to udp://127.0.0.1:4000');
  // camera solo
  sendOsc("/v1/camera/1/solo", "on", wait += step);
  sendOsc("/v1/camera/1/solo", "off", wait += step);
  sendOsc("/v1/camera/2/solo", "on", wait += step);
  sendOsc("/v1/camera/2/solo", "off", wait += step);
  sendOsc("/v1/camera/3/solo", "on", wait += step);
  sendOsc("/v1/camera/3/solo", "off", wait += step);
  sendOsc("/v1/camera/4/solo", "on", wait += step);
  sendOsc("/v1/camera/4/solo", "off", wait += step);
  // camera display
  sendOsc("/v1/camera/1/display", "off", wait += step);
  sendOsc("/v1/camera/2/display", "off", wait += step);
  sendOsc("/v1/camera/3/display", "off", wait += step);
  sendOsc("/v1/camera/4/display", "off", wait += step);
  sendOsc("/v1/camera/1/display", "on", wait += step);
  sendOsc("/v1/camera/2/display", "on", wait += step);
  sendOsc("/v1/camera/3/display", "on", wait += step);
  sendOsc("/v1/camera/4/display", "on", wait += step);
  // camera label
  sendOsc("/v1/camera/1/label", "パイロット1", wait += step);
  sendOsc("/v1/camera/2/label", "パイロット2", wait += step);
  sendOsc("/v1/camera/3/label", "パイロット3", wait += step);
  sendOsc("/v1/camera/4/label", "パイロット4", wait += step);
  sendOsc("/v1/camera/1/label", "sample pilot", wait += step);
  sendOsc("/v1/camera/2/label", "sample pilot", wait += step);
  sendOsc("/v1/camera/3/label", "sample pilot", wait += step);
  sendOsc("/v1/camera/4/label", "sample pilot", wait += step);
  sendOsc("/v1/camera/1/label", "Pilot1", wait += step);
  sendOsc("/v1/camera/2/label", "Pilot2", wait += step);
  sendOsc("/v1/camera/3/label", "Pilot3", wait += step);
  sendOsc("/v1/camera/4/label", "Pilot4", wait += step);
  // camera lap
  sendOsc("/v1/camera/1/lap", "add", wait += step);
  sendOsc("/v1/camera/2/lap", "add", wait += step_lap);
  sendOsc("/v1/camera/3/lap", "add", wait += step_lap);
  sendOsc("/v1/camera/4/lap", "add", wait += step_lap);
  sendOsc("/v1/camera/1/lap", "del", wait += step_lap);
  sendOsc("/v1/camera/2/lap", "del", wait += step);
  sendOsc("/v1/camera/3/lap", "del", wait += step);
  sendOsc("/v1/camera/4/lap", "del", wait += step);
  // camera laptime
  sendOsc("/v1/camera/1/laptime", 62.09, wait += step);
  sendOsc("/v1/camera/2/laptime", 56.20, wait += step_lap);
  sendOsc("/v1/camera/3/laptime", 58.83, wait += step_lap);
  sendOsc("/v1/camera/4/laptime", 45.31, wait += step_lap);
  sendOsc("/v1/camera/1/laptime", 0, wait += step_lap);
  sendOsc("/v1/camera/2/laptime", 0, wait += step);
  sendOsc("/v1/camera/3/laptime", 0, wait += step);
  sendOsc("/v1/camera/4/laptime", 0, wait += step);
  // speech
  sendOsc("/v1/speech/jp/say", "ドローンで遊ぼう", wait += step);
  sendOsc("/v1/speech/en/say", "You can fly!", wait += step_spc);
  // exit
  setTimeout(exit, wait += step);
}

test();
