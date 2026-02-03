# EAWebKit
EAWebKit is the browser engine used by SimCity and many other EA titles. it has been used for UI since the SimCity/sims 2 era

## how the engine loads parts of the game 
The UI is stored in the .package files. it is  standard HTML/CSS/js, except it makes requests to special URLs to receive and send information to the engine. known index files:
- Updater.html: the SimCity launcher shown after the splash screen 
- WebKitPanel.html: blank panel
- Editor.html: SCRUI editor (missing scripts in the package shipped with the game)
- Launcher3D_planar.html: The main menu after the updater and the intro movie

## how the game communicates with EAWebKit
### Client Callback
this system uses a js object in window.Client that contains callbacks which link to c++ code. this appears to be an older implementation and a remnant. The code paths it makes are impossible to progress with.
### game protocol
this system expects the engine to respond to specific special URLs.
- example URL: game://host/dbpf/file.js asks for game://host/gamecommand, this is a special URL 
#### special URLs 
all of these URLs have client Callback versions btw, it's just that game events don't work in client Callback.
- gamecommand
- gameevents
- gamedata 



