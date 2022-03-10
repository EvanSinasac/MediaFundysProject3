Program runs on Visual Studio Community 2019 in both Debug and Release mode in 64 bit.
DO NOT run in 32 bit.

TO RUN THE PROGRAM:
1. Using Visual Studio Community 2019 in either Debug or Release in 64 bit mode.

TO CHANGE THE AUDIO FILES:
1. From the solution level, navigate through common->assets->audio
2. In the audio folder, add and remove any audio files
3. Reflect any changes made by copying the names of any new audio files to the audioList.txt (including the file extension)
4. Reflect any changes made by removing the names of any removed audio files from the audioList.txt
5. Currently only works with .mp3 and .wav files

!!!NEW!!!
1. There are more sound files in the audioList.txt than are used in the program.
2. There must be AT LEAST 5 sounds for the program to run
3. I should add some safety but we needed at least 5 sounds for this project so it is hard coded at 5 right now
4. The first 5 files listed in the audioList.txt will be the ones playing around the scene

TO OPERATE THE PROGRAM:
1. This is an 'interactive' environment where the user can move themselves and an upside down bunny around the scene to hear different sounds
2. WASD             - to move the camera around the scene
3. QE               - to raise and lower the camera in the scene
4. Mouse            - Move camera target
5. Arrow Keys		- Move the upside down bunny listener along the XZ-plane
6. 1/2 (if active)  - switch between listening to the camera or bunny   (lines 328, 413, 414, 429, 430, 431, 438 switch between commented and uncommented (see video))

MODELS
The models I used are from Runemark Studio's Dark Fantasy Kit (https://assetstore.unity.com/packages/3d/environments/fantasy/dark-fantasy-kit-123894),  
and Creepy Cat's 3D Scifi Kit Vol 3 (https://assetstore.unity.com/packages/3d/environments/sci-fi/3d-scifi-kit-vol-3-121447),
as well as our bunny model. 

CAMERA
Main resouces used to figure out how to make fly camera were:
1. http://www.opengl-tutorial.org/beginners-tutorials/tutorial-6-keyboard-and-mouse/
2. and https://learnopengl.com/Getting-started/Camera (mainly the learnopengl)
 
VIDEO
https://youtu.be/Y1t0DB_kCBo

CURRENT SOUNDS/MESH
1. Front Right Speaker: FF1_03_Battle (Timelapse_Remix).mp3 represented by a pinkish bunny
2. Front Left Speaker:	mudkip.mp3 represented by a red turret
3. Central Speaker:		Counterattack.mp3 represented by a brown chair
4. Rear Right Speaker:	A_Hat_in_Time_OST_You_Are_All_Bad_Guys.mp3 represented by a teal bookshelf (just looks like a box)
5. Rear Right Speaker:	boop.mp3 represented by a pink pillar
6. Second listener:		Moves with arrow keys and is represented by a green upside down bunny