## Overview  
The purpose of this repository is to build a custom Linux embedded image with custom applications. The  <b>meta-stm32mp1-custom</b> repository provides a custom layer for the ST Microelectronics OpenSTLinux Distribution. Details can be found at: <br>
https://wiki.st.com/stm32mpu/wiki/STM32MP1_Distribution_Package
<br>
<br>
The custom layer has been configured to use Wifi connectivity, the on-board graphics display unit, and enable I2C and SPI buses for driving external peripherals .<br><br>
Details of project implementation may be found here: <br>
https://github.com/MichaelMcGetrick/meta-stm32mp1-custom/wiki
<br>
<br>
## Instructions for setting up the Custom Yocto Project

<ol>
<li>
Clone the OpenSTLinux Distro with <br>
<ul>
<li>
<b>repo init -u https://github.com/STMicroelectronics/oe-manifest.git -b refs/tags/openstlinux-6.1-yocto-mickledore-mp1-v23.06.21
</b>
</li>
<li>
<b>repo sync</b>
</li>
</ul>
</li>
<br>
<br> 
<li> 
The custom layer, meta-stm32mp1-custom, can be cloned using the commands: <br>
   
   <ul>
   	<li>
   		<b>repo init -u https://github.com/MichaelMcGetrick/manifest.git </b> <br>  
   	</li>
   	<li>
   		<b>repo sync </b><br>
   	</li>
   </ul>
   In line with ST reccommentations, locate the custom layer under folder <i>distro_par_dir</i>/layers/meta-st/ where <br>
   <i>distro_par_dir</i> is the parent directory for OpenSTLinux distribution.
</li>   
<br>
<li> 
From the OpenLSTiinux parent directory, set up the build environment using: <br>
   <ul>
   <li>
   <b>DISTRO=openstlinux-weston MACHINE=<stm32mp1> source layers/meta-st/scripts/envsetup.sh</b> 
   </li>
   <li>
   From the newly created build directory, run the command: <br>
   <b>bitbake-layers add-layer ../layers/meta-st/meta-stm32mp1-custom/</b>
   </li>
   </ul>
</li>
<br>


<br>
<li> 
Use the wpa_passphrase facility (as described in Section 3.3 of the wiki article)  <br>
https://github.com/MichaelMcGetrick/meta-stm32mp1-custom/wiki/#wpa_pw <br>
to configure WLAN with your access credentials. 
</li>
<br>
<li> 
Commence the build with <br>
  <b>bitbake st-image-weston</b>
</li>
<br>
<br>
</ol>   

