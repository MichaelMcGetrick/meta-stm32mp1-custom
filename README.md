## Overview  
The purpose of this repository is to build a custom Linux embedded image with required applications. The  <b>meta-stm32mp1-custom</b> repository provides a custom layer for the ST Microelectronics board support package (BSP) found at <br>
https://github.com/STMicroelectronics/meta-st-stm32mp. <br>
Specifically, meta-stm32mp1-custom is configured for the STM32MP157f-dk2 board. <br>

The custom layer provides functionality for WLAN using the onboard WiFI chip, and provides Dropbear - ssh which is useful
for file transfers during development activity using the scp facility.

The custom layer has been configured to use systemd / networkd and an I2C bus for use of the external peripheral RTC module DS3231.<br><br>
Details of project implementation may be found here: <br>


## Instructions for setting up the Custom Yocto Project

<ol>
<li>
Clone the STM32 BSP with <br>
git clone -b mickledore https://github.com/STMicroelectronics/meta-st-stm32mp.git. <br>
(NB: For the package to be compatible with the on board Murati Wifi Chip it is important to select the mickledore branch
- the Wifi interface fails to be estabished with earlier versions such as dunfell.) <br>
</li>
<br>
<li>
As indicated by STM, clone the STM32 BSP dependencies found at: <br>
 https://git.yoctoproject.org/poky <br>
 https://github.com/openembedded/meta-openembedded.git <br>
 selecting the mickledore branch. <br>
</li> 
<br> 
<li> 
Clone the custom layer using: <br>
   repo init -u https://github.com/MichaelMcGetrick/manifest.git <br> 
   repo sync <br>
</li>   
<br>
<li> 
In Yocto layers parent directory initialise build enviroment with: <br>
   source poky/oe-init-build-env build-mp1 <br>
</li>
<br>
<li> 
Commence the build with <br>
  bitbake custom-image
</li>
<br>
<br>
</ol>   

