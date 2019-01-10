var stdo = WScript.StdOut;
var args = WScript.Arguments;
if (args.Count() != 2) {
  stdo.WriteLine("invalid arguments.");
  WScript.Quit();
}
var lang = args.Unnamed(0);
var text = args.Unnamed(1);
var spvc = WScript.CreateObject("SAPI.SpVoice");
if (lang != "jp" && lang != "en") {
  stdo.WriteLine("invalid language.");
  WScript.Quit();
}
var voices = spvc.GetVoices();
var vce;
var found = false;
for (var i = 0; i < voices.Count; i++) {
    vce = voices.Item(i);
    var desc = vce.GetDescription().toLowerCase();
    if (lang == "jp" && desc.indexOf("japanese") != -1) {
      found = true;
      spvc.Voice = vce;
      break;
    } else if (lang == "en" && desc.indexOf("english") != -1) {
      found = true;
      spvc.Voice = vce;
      break;
    }
}
if (found == false) {
  stdo.WriteLine("language:" + lang + " not found.");
}
spvc.Speak(text);