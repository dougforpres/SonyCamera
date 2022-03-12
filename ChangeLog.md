# Change Log
## 1.0.0.32
* Updated GetDeviceInfo method to use device handle like all other methods (as opposed to deviceId).
* ICLE-7R (A7R): Updated to include Auto-Crop resolution, Exposure, and ISO info.
* ICLE-7M2 (A7 Mk II): Updated to include Exposure and ISO info.
* ICLE-5100 (a5100): Updated to include Exposure and ISO info.
* ICLE-6300 (a6300): Updated to include ISO info.
* Updated SonyCameraInfo.exe to use new GetDeviceInfo method.  The older implementation was causing issues when non-cameras (like USB Drives) were connected with lower device ID's than the camera.
* Removed live status info from ASCOM Setup Dialog - started crashing after install of NINA 2.0 beta (no idea why).
* Updated auto learn of Exposure Time and ISO so they hopefully work a little more reliably.  They will attempt to nudge the settings up/down by one setting before starting the detection.  It is possible the virtual button might be a little sticky :).

## 1.0.0.31
* ILCE-A7S (A7S): Updated to include Cropped Image Size, Exposure, and ISO.
* ILCE-A7C (A7C): Added.
* SLT-A99V (Alpha 99): Added.
* ILCE-6000 (a6000): Updated to include ISO.

## 1.0.0.30
* SonyCameraInfo.exe can now be run (in basic scan mode) from Windows Explorer (it will wait for \<enter> key to be pressed before exiting).
* Finally figured out how libraw deals with crop info.  Thanks to a user with an A58 who sent me a sample image.
* (ASCOM Driver) Deal with APT not supporting get/set of Gain using index mode.  Driver will now try to find a matching gain and use it.

## 1.0.0.29
* More fixes to support NINA.
* Changed the lock when taking an image to only lock direct control/update of camera settings.  Tested a number of longer images using NINA with no issue.
* Removed a little old code that had previously been commented out.

## 1.0.0.28
* Fixed problem with NINA support that caused driver to fail when connecting to camera.
* Fixed problem with NINA support where the wrong error would be sent to NINA if ISO/Gain was disabled/unavailable.
* (ASCOM Driver) Added warning popups to a couple of settings, requesting user visit Wiki.
* (ASCOM Driver) Added links to setup dialog pages providing direct access to Wiki.
* Updated SonyCameraInfo.exe to not dump all loaded DLL's at start unless requested.

## 1.0.0.27
* SonyCameraInfo.exe will now try to load each of the required DLL's separately and connect to all the needed methods.  Info about the success/failure is displayed at startup.  Additionally, a new command-line option (/v) has been added that will dump info on ALL DLL's loaded directly or indirectly by the driver.
* Added ISO info for A7S III.
* Added a bypass for RefreshPropertyList method when a photo is being taken.  NINA will call this method (which is locked) during exposures, and generate an error if it takes too long to respond when a long exposure is occurring.
* Temporarily removed loading of libusbK library

## 1.0.0.26
* SonyCameraInfo.exe updated to detect ISO values.
* Added support for changing ISO setting.  Note that the available ISO values reported by the camera (at least for my a6400) are not valid.  The list includes many ISO values that the camera does not support.  As a result the driver utility now scans the available ISO values when the camera is detected and these are stored.
* Added fake camera property (0xfffe) that can be queried to retrieve supported ISO values.

## 1.0.0.25
* SonyCameraInfo.exe refactored to detect supported exposure times.
* Driver support for changing exposure time.  This is useful for taking very short exposures (i.e. Bias/Flats) as the native timing of the camera becomes critical for this type of shot.
* Added fake camera property (0xffff) that can be queried to retrieve supported exposure times.

## 1.0.0.24
* Various tests trying to get libusbK devices to be recogni(s|z)ed.

## 1.0.0.23
* Enhanced "SonyCameraInfo" with extra abilities.  See "SonyCameraInfo.exe /h" for details.
* Updated camera detection to re-acquire test shots if crop info is not in registry.
* Added a couple of pieces of extra info in DEVICEINFO structure.  This will help other apps.

## 1.0.0.22
* Added code to check that logfile is working correctly in the "SonyCameraInfo.exe" utility.
* Added code to the logging system so that it will tack the process-id to the end of the logfile name.  This should avoid situations where some other app has locked the file.
* Added camera definition for DSC-RX10M4.
* Added camera definition for ILCA-99M2.
* Added camera definition for A7R IV (as libusbK device, already had MTP defintion).
* Added camera definition for A7S III (as libusbK device, already had MTP defintion).
* Updated SonyCameraInfo.exe to write additional info to output.  It will also now enable live-view feature in APT (if camera supported) without requiring a driver update.
* Updated Logging system to respect environment variables.  Now user can just set "Logfile Name" to be "%HOMEDRIVE%%HOMEPATH%\mylogfile.log" and it'll put it in their home directory.'

## 1.0.0.21
* Large reshuffle of code to allow inclusion of native LibUSBK devices (which appears to be the way Sony are going with newer camera models.)  This will allow newer cameras to be supported out of the box without having to mess around with device driver fun.
* Fixed a couple of minor issues with older code.

## 1.0.0.20
* Added some new logging for command success/fail.
* Added an ignore for the success/fail of the initial "GetStorageIDs" query.
* Added camera definition for a7siii.
* Modified code to ignore error respose to the "GetStorageIDs" request.  Seems the a7siii returns an error, but otherwise works.
* Include camera response code for all requests.

## 1.0.0.19
* Added flag for enabling the Auto Save of images.  This works in conjunction with the Auto Save Path.  The flag setting is managed by the ASCOM driver.
* Fixed a couple of logging issues.
* Added camera defintion for a6100.
* Added autocrop image size data for a few cameras - need to get the rest before default crop mode can be set to "AutoCrop" to have RGGB size match ARW.

## 1.0.0.18
* Fix to handle the difference between 32 and 64-bit handles being passed back and forth between this code and ASCOM driver (which is expecting 32-bit values).

## 1.0.0.17
* Added 64 bit support to the project, plus indicator of 32/64 in log startup.

## 1.0.0.16
Found and fixed the issue with a6300 (and a6500) not being able to use LiveView mode.

## 1.0.0.15
* Added camera definition for SLT a58.
* Fixed issue where if ASCOM closed a device (camera) it could not reopen it.  Caused by passing the object pointer instead of the handle to the cleanup method.

## 1.0.0.14
* Added camera definition for a6600.
* Added camera definition for a5100.

## 1.0.0.13
* Added camera definition for a7 II.

## 1.0.0.12
* Added camera definition for a5000.

## 1.0.0.11
* Added a7R IV
* Added camera definition for a7R IV.

## 1.0.0.10
* The Dummy camera device is used for testing, it needed to have support for the newer properties that are used when taking images.

## 1.0.0.9
### APT Unexpected Exits
* Isolated and fixed cause of APT crash when camera physically unplugged.  This was due to the way spontaneous events from Windows were being handled (yay for lack of documentation!).

## 1.0.0.8
* There isn't really a changelog for earlier versions, thus this version includes documentation for the current as well as a couple of prior releases.

### Download chunking
* The original version allocated enough memory to retrieve the entire image download in one chunk.  This was efficient speed-wise but used more RAM.  This was noticable on SharpCap which is memory constrained in the free version.
* With the addition of support for the new a7R4 that meant around 120MB of memory (vs the 24MB for my a6400).
* First I tried using the Microsoft "recommended" chunk size, which turns out to be 256kB - that actually worked fine, but had a noticable performance penalty.
* Next I added some logic to download in multiples of 16x the recommended size - this improved download speeds dramatically.  If Microsft ever up the default, the code is limited to a max of 8MB per chunk.
* Finally, support was re-added for what I call "Maximum Memory Mode", basically allocating it all in one go.
* The default is 16x the recommended size, which at this point means downloading in chunks of 4MB.

### Camera Support
* A number of new cameras have been added: a7III, a7RII, a7RIV.

### Correct cleanup when device disconnected
* A bug was preventing the device from actually being closed properly when the app issues a disconnect.  Not really a big issue - but in the case of APT, there is a mysterious issue where APT will just quit when a connected (Sony) camera is unplugged.  Now, as long as the camea is disconnected, it doesn't exit (don't know why this problem occurs).

### Version number logging
* Added version number logging to the startup of the DLL.  This makes it much easier to diagnose issues!

### Support for auto-focus when taking images
* I've noticed that if auto-focus is enabled, the camera will sometimes not respond correctly to a "take a photo" command (if exposure duration is short).  I reasoned it could be that auto-focus wasn't having time to do its thing and the camera was missing the "I press the button dummy" event.
* Fix is to detect that AF is on and add a 0.5 second delay between the half and full-down steps on the shutter button.

### Special case for a7R4 not taking photos
* When first testing out the a7R4 support it seemed that the command toggles to "press the shutter button" seem to default to "on" (making it look like the button is pressed).  The fix is to check that the toggles are in an "OFF" state just before taking the image.
* Additionally, it also correctly checks that you don't have your finger on the shutter-release.

### SonyCameraInfo tool update
* If there are multiple "portable device" class devices connected to computer when the SonyCameraInfo tool is run it could hang waiting for a Hard Drive to take a photo.  This isn't ideal, so now it should correctly detect that it isn't a camera and move on to the next device.

### Image Cropping
* The raw library this code uses returns the "real" raw pixels.  It turns out that some/most cameras actually returns a bigger image than the manufacturer specs list.  There is extra data in the ARW file that specifies how much the image should be cropped.
* The code now allows the auto-cropping to occur.  At the moment the option is set to "uncropped" which means image size is the full pixel array.  Supported options include "AUTO" and "MANUAL".
* In a future version the default will be changed to "AUTO".
