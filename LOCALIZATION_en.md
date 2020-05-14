# Localization structure

The program used simple XML language files in /data/lang/ directory. Files with names like "lang_XX.xml", where "XX" is the short name of the language. To add support for a new language, you need to perform a series of actions. First of all, generate your own language file. Copy simple bash script "langgen" from /docs/locales directory in source root and run like "langgen >./lang_XX.xml". Or copy and rename "lang_en.xml" file from /docs/locales. You need to translate all lines in the file as you like. Save file in UTF-8 locale. Copy that file to /data/lang directory.

Modify header attributes in XML-file:
- id - short name of language.
- label - localized name of language.
- name - name of language in english.
- locale - locale name.
- wincodepage - windows code page. See [here](https://en.wikipedia.org/wiki/Windows_code_page).
- winvoice - id of windows voice. See [here](https://support.microsoft.com/en-ca/help/324097/list-of-language-packs-and-their-codes-for-windows-2000-domain-control).
- osxvoice - name of MacOSX voice. See [here](https://gist.github.com/mculp/4b95752e25c456d425c6).

Next, check two procedures: "speakRemainTime" and "speakLap". After "// internationalization" comment add your own regex modifications to correctly pronounce sentences.
    
    void speakLap(int camid, float sec, int num) {
    ....
    // Put your language definition here
    // Czech
    if (currentlang == "cs") sout = regex_replace(sout, regex("(\\d).(\\d{2})"), "$1 $2");
    	speakAny(currentlang, sout);
    }
    
    void speakRemainTime(int sec) {
    ....
    	// Put your language definition here
    	if (currentlang == "cs") {
    		// something here
    	}
    	speakAny(currentlang, str);
    }
All done! Recompile TinyViewPlus, delete or backup "settings.xml" file, run program and check the working of autoselect language procedure, localization interface and speech engine.
