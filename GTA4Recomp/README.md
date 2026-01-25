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

## kernel/vfs.cpp/h
Added the VFS system to redirect all calls to specific xbox360ied paths to the host so like game:/common.rpf gets redirected to gamefiles\common\

## kernel/xam.h/cpp
Added translations for keyboard to controller mapping.

## kernel/game_init
Replace sub_82120000 before game load

## kernel/xenon_memory
init xb360 memory regions because the game assumes that they exist

## locale/config_locale.cpp
Added menu language for english

## locale/locale.h/cpp
Added menu language for Japanese, English, Spanish, Italian, German and French. These dont work, only english does as of now.

## patches/audio_patches.cpp
Updated the patch for the GTA4 audio engine

## gta4_aspect_ratio_patches.cpp
added patches for different aspect ratio monitors. If its bigger than 16:9, it adds pillarboxes and if its lower than 16:9 then add letterboxes

## gta4_input_patches.cpp/h
Added the register for the xbox controller

## gta4_patches.cpp/h
No functionality as of now, just a structure which I will add to in debugging

## res/win32
changed labels for unleashed recomp to GTA4Recomp

## ui/button_window.cpp
removed the sonic style button window, handle it differently

## ui/common_menu.cpp
Changed menu, not too much but it's just the functionality that changed

## ui/game_window
Added native resolution detection

## ui/options_menu
Updated options menu to gta4

## user/config
setup config, based on MarathonRecomp

## user/config_def.h
config definitions

