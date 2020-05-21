# Change Log

## 1.0.0.9

### APT Unexpected Exits
Isolated and fixed cause of APT crash when camera physically unplugged.  This was due to the way spontaneous events from Windows were being handled (yay for lack of documentation!).

## 1.0.0.8
There isn't really a changelog for earlier versions, thus this version includes documentation for the current as well as a couple of prior releases.

### Download chunking
The original version allocated enough memory to retrieve the entire image download in one chunk.  This was efficient speed-wise but used more RAM.  This was noticable on SharpCap which is memory constrained in the free version.
With the addition of support for the new a7R4 that meant around 120MB of memory (vs the 24MB for my a6400).
First I tried using the Microsoft "recommended" chunk size, which turns out to be 256kB - that actually worked fine, but had a noticable performance penalty.
Next I added some logic to download in multiples of 16x the recommended size - this improved download speeds dramatically.  If Microsft ever up the default, the code is limited to a max of 8MB per chunk.
Finally, support was re-added for what I call "Maximum Memory Mode", basically allocating it all in one go.
The default is 16x the recommended size, which at this point means downloading in chunks of 4MB.

### Camera Support
A number of new cameras have been added: a7III, a7RII, a7RIV.

### Correct cleanup when device disconnected
A bug was preventing the device from actually being closed properly when the app issues a disconnect.  Not really a big issue - but in the case of APT, there is a mysterious issue where APT will just quit when a connected (Sony) camera is unplugged.  Now, as long as the camea is disconnected, it doesn't exit (don't know why this problem occurs).

### Version number logging
Added version number logging to the startup of the DLL.  This makes it much easier to diagnose issues!

### Support for auto-focus when taking images
I've noticed that if auto-focus is enabled, the camera will sometimes not respond correctly to a "take a photo" command (if exposure duration is short).  I reasoned it could be that auto-focus wasn't having time to do its thing and the camera was missing the "I press the button dummy" event.
Fix is to detect that AF is on and add a 0.5 second delay between the half and full-down steps on the shutter button.

### Special case for a7R4 not taking photos
When first testing out the a7R4 support it seemed that the command toggles to "press the shutter button" seem to default to "on" (making it look like the button is pressed).  The fix is to check that the toggles are in an "OFF" state just before taking the image.
Additionally, it also correctly checks that you don't have your finger on the shutter-release.

### SonyCameraInfo tool update
If there are multiple "portable device" class devices connected to computer when the SonyCameraInfo tool is run it could hang waiting for a Hard Drive to take a photo.  This isn't ideal, so now it should correctly detect that it isn't a camera and move on to the next device.

### Image Cropping
The raw library this code uses returns the "real" raw pixels.  It turns out that some/most cameras actually returns a bigger image than the manufacturer specs list.  There is extra data in the ARW file that specifies how much the image should be cropped.
The code now allows the auto-cropping to occur.  At the moment the option is set to "uncropped" which means image size is the full pixel array.  Supported options include "AUTO" and "MANUAL".
In a future version the default will be changed to "AUTO".
