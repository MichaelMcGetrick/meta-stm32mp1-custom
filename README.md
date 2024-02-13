## Overview  
The purpose of this repository is to build a custom Linux embedded image with custom applications. The  <b>meta-stm32mp1-custom</b> repository provides a custom layer for the ST Microelectronics OpenSTLinux Distribution.

The custom layer has been configured to use Wifi connectivity, and enable I2C and SPI buses for driving external peripherals .<br><br>
Details of project implementation may be found here: <br>
https://github.com/MichaelMcGetrick/meta-stm32mp1-custom/wiki

## Instructions for setting up the Custom Yocto Project

<ol>
<li>
Clone the OpenSTLinux Distro with <br>
<b>repo init -u https://github.com/STMicroelectronics/oe-manifest.git -b refs/tags/openstlinux-6.1-yocto-mickledore-mp1-v23.06.21
</b>
<br>
<b>repo sync</b>
</li>
<br>
<br> 
<li> 
Clone the custom layer using: <br>
   <b>repo init -u https://github.com/MichaelMcGetrick/manifest.git </b> <br>  
   <b>repo sync </b><br>
</li>   
<br>
<li> 
In Yocto layers parent directory initialise build enviroment with: <br>
   <b>source poky/oe-init-build-env build-mp1</b> <br>
</li>
<br>
<li> 
Navigate to build-mp1/conf folder and ensure the following layers are defined in the BBLAYERS variable in bblayers.conf: <br>
   BBLAYERS ?= " \ <br>
  /<i>layer-parent-dir</i>/poky/meta \ <br>
  /<i>layer-parent-dir</i>/poky/meta-poky \<br>
  /<i>layer-parent-dir</i>/poky/meta-yocto-bsp \<br>
  /<i>layer-parent-dir</i>/meta-openembedded/meta-oe \<br>
  /<i>layer-parent-dir</i>/meta-openembedded/meta-python \<br>
  /<i>layer-parent-dir</i>/meta-st-stm32mp \<br>
  /<i>layer-parent-dir</i>/meta-stm32mp1-custom \<br>
  "<br>
  where <i>layer-parent-dir</i> is the parent directory for Yocto layers.<br> 
</li>
<br>
<li> 
Use the wpa_passphrase facility (as described in Section 3.3 of the wiki article)  <br>
https://github.com/MichaelMcGetrick/meta-stm32mp1-custom/wiki/#wpa_pw <br>
to configure WLAN with your access credentials. 
</li>
<br>
<li> 
Commence the build with <br>
  <b>bitbake custom-image</b>
</li>
<br>
<br>
</ol>   

