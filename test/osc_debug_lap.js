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
  var step_lap = 2000;
  var step_spc = 1000;
  console.log('Sending OSC message to udp://127.0.0.1:4000');
  // camera lap
  sendOsc("/v1/camera/1/lap", "add", wait += step);
  sendOsc("/v1/camera/2/lap", "add", wait += step_lap);
  sendOsc("/v1/camera/3/lap", "add", wait += step_lap);
  sendOsc("/v1/camera/4/lap", "add", wait += step_lap);
  sendOsc("/v1/camera/1/lap", "del", wait += step_lap);
  sendOsc("/v1/camera/2/lap", "del", wait += step);
  sendOsc("/v1/camera/3/lap", "del", wait += step);
  sendOsc("/v1/camera/4/lap", "del", wait += step);
  // exit
  setTimeout(exit, wait += step);
}

test();
