# uNabto ESP32 cam demo

This project turns the ESP32-CAM into a remote P2P accesible surveillance camera.
It uses the esp-idf FreeRTOS based core (vs. some other demos which uses the arduino core).
Also it uses the espressif esp32-cam library with slight modification (we could not get the reset to work).

The video is MJPEG and the module can cope to remotely stream (encode to mjpeg and encrypt and integrity check) a VGA feed at around 10 frames per second.

# The structure

The structure of the project is as follows
<pre>

    +
    +-->+-------------+
    |   |    Main     |        This folder contains the initial setup
    |   +-------------+        and the application_event() function defining
    |                          the application
    +-->+-------------+
        |  Components |
        +-------------+
                      |
                      +------->+----------------+
                      |        | unabto         |    Don't edit here (link to other github rep)
                      |        +----------------+
                      |
                      +------->+----------------+
                      |        | unabto-esp-idf |    The platform integration
                      |        +----------------+
                      |
                      +------->+----------------+
                      |        | nabtotunnel    |    Nabto tunnel application
                      |        +----------------+
                      |
                      +------->+----------------+
                      |        | unabto-esp-    |    Client fingerprint database
                      |        | fingerprint    |    storage in NVS
                      |        +----------------+
                      |
                      +------->+----------------+
                               | esp32-camera   |    From the espressif github (not a link, since we adjusted)
                               +----------------+
</pre>


ESP-IDF project has a speciel structure. You can read more about that here:
https://esp-idf.readthedocs.io/en/v1.0/build_system.html
The unabto source and the integration code is done as components and therefore resides in the IDF component folder.
The unabto sdk is a submodule link to the unabto generic source.

The initial setup/commisioning and the application is located in the main folder.
unabto_application.c contains the application (inside the application_event function), the main.c contains the setup and configuration of the WiFi module.


# How to set it up

## Step 1: Setup the ESP-IDF build environment

Follow the setup of the ESP-IDF toolchain setup

http://esp-idf.readthedocs.io/en/latest/get-started/index.html

## Step 2: Clone the repository


```
git clone --recursive https://github.com/nabto/unabto-esp32.git
```

## Step 3: Make menuconfig

Enter into menu "Camera configuration"

Edit/adjust the following menu items:


* WIFI access point name (SSID) : The wifi access point you want the module to attach to
* WIFI password : The password used by the access point
* Nabto ID : The Nabto device id you get from your AppMyProduct account
* Nabto key - 32 hex chars : The key for the specific device id you entered in the before mentioned item

Camera wiring! If you have an ESP-EYE board nothing else needs to be set up if you have an “ESP32 Cam” from Ai Tinker you need to configure this too (also in the “Camera configuration” menu).


## Step 3: Build the project

```
make
```

## Step 4: Flash the Image

Possible you need to adjust the serial deivce to use for flashing which is setup in the menuconfig part, but mostly the standard setup will match your platform.

```
make flash
```


# How to test the application


## Monitor the output from the board

Using the monitor command you should see a printout similar to the following every time the ESP32-EVB starts up:

Look for the "connected!" telling that the device knows how to connect to your WIFI. Also look for the Nabto state change to "ATTACHED" meaning that the device succesfully attached to Nabto infrastructure (registered and is online, waiting for connect requests from clients). The cloud service (controller) address can be varying dependent on geographic region (we have 4 datacenters) and availabillity.

```
00:00:01:457 main.c(382) connected!
00:00:01:460 main.c(388) IP Address:  192.168.2.147
00:00:01:465 main.c(389) Subnet mask: 255.255.255.0
00:00:01:470 main.c(390) Gateway:     192.168.2.1
00:00:01:475 unabto_application.c(59) In demo_init
00:00:01:479 unabto_application.c(78) Before fp_mem_init
00:00:01:495 unabto_application.c(81) Before acl_ae_init
00:00:01:495 unabto_common_main.c(110) Device id: 'jicnkjqs.ev9dbf.appmyproduct.com'
00:00:01:498 unabto_common_main.c(111) Program Release 4.4.0-alpha.0
00:00:01:505 network_adapter.c(140) Socket opened: port=5570
00:00:01:510 network_adapter.c(140) Socket opened: port=49153
00:00:01:515 unabto_stream_event.c(235) sizeof(stream__)=328
00:00:01:521 unabto_context.c(55) SECURE ATTACH: 1, DATA: 1
00:00:01:526 unabto_context.c(63) NONCE_SIZE: 32, CLEAR_TEXT: 0
00:00:01:532 unabto_common_main.c(183) Nabto was successfully initialized
00:00:01:539 unabto_context.c(55) SECURE ATTACH: 1, DATA: 1
00:00:01:543 unabto_context.c(63) NONCE_SIZE: 32, CLEAR_TEXT: 0
00:00:01:550 network_adapter.c(140) Socket opened: port=49154
00:00:01:555 unabto_attach.c(770) State change from IDLE to WAIT_DNS
00:00:01:561 unabto_attach.c(771) Resolving DNS for jicnkjqs.ev9dbf.appmyproduct.com
00:00:01:675 unabto_attach.c(790) Resolved DNS for jicnkjqs.ev9dbf.appmyproduct.com to:
00:00:01:675 unabto_attach.c(796)   Controller ip: 34.232.129.33
00:00:01:678 unabto_attach.c(802) State change from WAIT_DNS to WAIT_BS
00:00:01:887 unabto_attach.c(480) State change from WAIT_BS to WAIT_GSP
00:00:01:888 unabto_attach.c(481) GSP address: 34.194.195.231:5565
00:00:01:895 unabto_attach.c(270) ########    U_INVITE with LARGE nonce sent, version: - URL: -
00:00:02:089 unabto_attach.c(563) State change from WAIT_GSP to ATTACHED
```

To test the camera you can try to access:
http://<adress of camera>:8081/

Which will 


## Download the AMP video app

Download the Android or iPhone app from app store
https://play.google.com/store/apps/details?id=com.appmyproduct.video
https://itunes.apple.com/lc/app/appmyproduct-video-client/id1276975254


## Pair the video app with the wifi module


## Connect and view video
