All of my work done in this folder was primarily changing names from sonicnext-dev/MarathonRecomp and the work done by the community reverse engineering the GTAIV RAGE Engine.

However there were a bunch of additions I made using both Xenia and other recompilations including my own. 


My changes: Changing references to Fernando rather than Marathon. I dont want to use GTA4 branding as I dont want rockstar looking for me. As will for most code, instead of naming it GTA4 or something, I will be using F1 Driver names instead. 

## cpu/guest_thread.h/cpp

Replaced the offsets and sizes from marathon and created new structs for PCR, TLS and TEB. 
Changed the raw byte-swap writes to struct field initialisation and g_memory

## api/RAGE folder
Added the reverse engineered structures from the RAGE engine. Research was done by the Nizhniy Novgorod's .black project and OpenIV.

## gpu/video.cpp
Disabled A LOT of the original sonic06 hooks as they will just confict with GTA4 since theyre not used. I have kept them but disabled for reference.

## hid/mouse_camera.h/cpp
Added mouse camera functionality for mouse camera movement. This maps the mouse to the Right Stick as there is no native mkb support on the 360 version
TODO: add weapon wheel functionality with scroll wheel

## hid/driver/sdl_hid.cpp
Not much, just added some translations from SDL scancode unicode to Xbox vkey

## install/hashes
The hashes for the game taken from the xbox 360 game disc

## install/installer
Removed DLC checks and parsing. Changed checkfiles to GTA4

## kernel/io/file_system.cpp
Redirect shader paths to the extracted shaders location created by installer.cpp

## kernel/heap.cpp
Changed heap allocation sizes

## kernel/memory.cpp/h
reworked because GTA4 can reach very very low memory.
