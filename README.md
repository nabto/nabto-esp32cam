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

Enter into menu "Custom configuration"

Edit/adjust the following menu items:


* WIFI access point name (SSID) : The wifi access point you want the module to attach to
* WIFI password : The password used by the access point
* Nabto ID : The Nabto device id you get from your AppMyProduct account
* Nabto key - 32 hex chars : The key for the specific device id you entered in the before mentioned item


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


```
```

## Download the AMP video app

Download the Android or iPhone app from app store
https://play.google.com/store/apps/details?id=com.appmyproduct.video
https://itunes.apple.com/lc/app/appmyproduct-video-client/id1276975254


## Pair the video app with the wifi module


## Connect and view video
