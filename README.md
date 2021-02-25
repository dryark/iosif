# iOS CLI

## Build
`make`

## Show usage
`iosif`

# How does it work
iosif is unique in that it uses the MobileDevice framework to communicate with iOS devices. The MobileDevice framework is a "private Apple API", meaning that it is undocumented, but it is at least officially provided and will be maintained by Apple so it is more future proof than implementations like libimobiledevice that also re-implement the internals of the MobileDevice framework.
Currently iosif only works on MacOS because it is built directly against the framework, but in the near future it will also be made to work on Windows by linking against the MobileDevice framework DLL that exists within iTunes for Windows.

# Features

## Syslog
`iosif log`

Tails / displays the device system log.

## List Devices
`iosif list`

Lists connected devices ( both Wifi and USB )

## Device connect/disconnect Feed
`iosif detectloop`

Stay running and show devices as they are connected/disconnected.

## Take screenshot
`iosif img`

## "Gas Gauge"
`iosif gas`

Poor battery information

## Battery Information
`iosif battery`

Display detailed battery information.  

## System Configuration
`iosif syscfg`

Fetches and attempts to display iOS syscfg. In progress / isn't really usable yet.  

## Process List
`iosif ps`

## Mobile Notifications
`iosif notices`

Tails / shows "mobile notifications". These are things like "Application opened" or "Application closed" etc.

## App Memory / CPU Usage
`iosif sysmon`

Currently hardcoded to a specific PID...

## Mach Time Info
`iosif machtimeinfo`

Display uptime of device, time factor, and current mach absolute time of device.

## Install Application
`iosif install -path myapp.app`

## Mobile Gestalt
`iosif mg [key]`

## Device Info
`iosif info [key]`

## Directory Listing
`iosif ls [path]`

Shows full file listing on device. Files shown cannot be actually pulled on a non-rooted device, but you can at least see the structure.

## IO Registry
`iosif ioreg [key]`

## Port Tunnel
`iosif tunnel [from]:[to]`  

Tunnels ports to port on device. Unlike iproxy, this does not enable the reverse, for [from] to be contacted from the device at [to].

# Related Software
The following software implement some subsection of the above available features:  
* ios-deploy  
* mobiledevice
While you will note that many specific features accomplish the same thing as features of these, the implementations in iosif are full recreations based on the available knowledge of how the Apple APIs work.
libmobiledevice implements almost all of the functionality of iosif, but it is more difficult to build and it is a full reimplementation and therefore more likely to break as Apple changes things.  

# Thanks  
* To TMobile. Despite your terrible treatment of your software engineers ( including myself ), my work with you started off the interest that eventually led to this. Thanks for allowing stf_ios_support to be open source from the start.
* To my undiclosed clients. Without your financial assistance I would have nowhere to live and be unable to spend my time on this stuff.  
* To Daniel Paulus, for dtx_codec, go-ios, abd ios_simulator_dtx_dump. Your released code aided me in being able to understand how all this stuff works.
* To Alibaba, for tidevice, becoming the first to publicly release a reverse engineered method for how to run xctest without Xcode. 
* To libimobiledevice team, for creating a usable set of tools despite absurd complexity.