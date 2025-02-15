Accelerated Linux Distribution
==============================

Contents
--------

1. Introduction
2. Instructions for compiling
3. Changing the applications/kernel-options/libraries
4. Documentation


1 Introduction
------------

This source package is an "all-in-one" build framework for generating a
complete embedded system. It has been developed with embedded devices
in mind, but it can just as equally be used for normal computing devices
(like a PC for example). It is ideal for building small, light weight
systems.

The framework is targeted at building Linux based firmware for small
embedded systems. It is capable of building for non-MMU and MMU targets
of Linux.

It supports a wide varity of hardware, many CPUs and a large number of
target boards. It is easy to extend the build for new vendors or boards
and to add new applications.


2 Instructions for Compiling
--------------------------

  1. You will need a cross-compiler package for your target. Many binary
     tool packages exist already. Your development host system may have
     cross cimpiler packages available for it (for example ubuntu does).
     There are third party packages available from groups such as
     CodeSourcery. Pre built toolchains are available from:
	 
	 https://sourceforge.net/projects/uclinux/files/Tools/
	 
     Install that first in the usual way.
 
  2. If you have not un-archived the source package then do that now.
     It is a bzipped tar image, so do:
 
       tar xzf accelerated-XXXXXXXX.tar.bz2
 
     This will dump the source into a "accelerated" directory.
     You can do this into any directory, typically use your own user
     login. (I don't recommend devloping as root, it is bad practice,
     and it will bite you one day!)
 
  3. Cd into the source tree:
 
          cd accelerated
 
  4. Configure the build target:
 
          make menuconfig
 
     You can also use "make config" or "make xconfig" if you prefer.
 
     The top level selection is straight forward if you know the vendor of
     the board you want to compile for. You can choose also to modify the
     underlying default kernel and application configuration if you want.
 
     At first it is suggested that you use the default configuration for
     your target board. It will almost certainly work "as is".

     You can choose to enter configuration for the kernel or libraries/
     applications at this step. Saying yes to those will then run the
     configuration on those components after 'Save and Exit' of this step.
     (Section 2 below contains more information on doing this.)

     Based on what platform you choose in this step the build will generate
     an appropriate default application set.

     Sometimes a number of questions will appear after you 'Save and Exit'.
     Do not be concerned, it just means that some new config options have
     been added to the source tree that do not have defaults for the
     configuration you have chosen.  If this happens the safest option is
     to answer 'N' to each question as they appear.

  5. Build the image:
 
          make
 
 
  Thats it!
 
  The make will generate appropriate binary images for the target hardware
  specified. The final generated files will be placed under the "images"
  directory. The exact files vary from target to target, typically you end
  up with something like an "image.bin" file.

  How to load and run the generated image will depend on your target system
  hardware. There are a number of HOWTO documents under the Documentation
  directy that describe how to load and run the image on specific boards.
  Look for a file named after your target board.


3 Changing the Applications/Kernel/Libraries
------------------------------------------

  You can modify the kernel configuration and application set generated for
  your target using the config system. You can configure by running one of
  the following three commands:

        make xconfig       - graphical X11 based config
        make menuconfig    - text menu based config
        make config        - plain text shell script based config

  Menuconfig and xconfig are the simplest, I would recommend using one of
  them.

  The key options under the "Target Platform Selection" menu are the
  following:

        Customize Kernel Settings
            Selecting this option run the standard Linux kernel config.

        Customize Vendor/User Settings
            Selecting this option will run a configure process allowing
            you to enable or disable individual applications and libraries.

   Use the online "Help" if unsure of what a configuration option means.

   When you 'Save and Exit' the build system will run you through the
   configs you have selected to customise.


4 Documention
-----------

  There is an assortment of documentaion files under the Documentaion
  directory. The more interresting ones are:

       SOURCE  -- file at the top level gives a brief run down of the
                  structure of this source distribution package.

       Documentation/Adding-User-Apps-HOWTO
               -- description of how to add a new application into the
                  config and build setup of the distribution.

       Documentation/Adding-Platforms-HOWTO
               -- description of how to add a new vendor board config to
                  the distribution.

       Documentation/<BOARD>-HOWTO
               -- describes building and loading for a particular board.

