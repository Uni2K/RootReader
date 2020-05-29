# RootReader

 
This helps you to read in WaveCatcher binary files and convert them to .root files.
For more informations see the Github Wiki.
  
![Demo](demo.PNG)

  
 

### How to run it?

Firstly, it needs to run in a C environment (better C++) with pre-installed ROOT framework (https://root.cern.ch/). If it is used on Windows, the best way would be to use a WSL (Windows subsystem for linux). This can be installed directly from Microsoft Store (e.g. search for Ubuntu). This works only on Windows 10, here is a tutorial:

https://medium.com/@blake.leverington/installing-cern-root-under-windows-10-with-subsystem-for-linux-beta-75295defc6d4
(I recommend using a pre-build version of ROOT)

If everything is installed correctly just start the software like you would start any other script:

> ./RootReader.sh

  

### Purpose & Customisation

This tool is designed to read binary data produced by the WaveCatcher software into a ROOT tree. This works by calculating pre-defined observables (integral, amplitude, time,...) for each signal. The ROOT file finally combines all the results into histograms. Which observables are calculated depends on the purpose of the measurement. I personally did efficiency measurements that mainly require integrals to be calculated. However, when dealing with other purposes such as timing resolution studies etc. the pre-defined observables need to be changed.
This can be done by adjusting the section inside the read.C file. This file provides the main read in algorithm and is the heart of the software kit. Unfortunately, it is not possible to describe every part of this script, it is therefore necessary to analyze the read.C file and get familiar with the parts. I tried my best to make it as clear as possible.

  
For instant help just open the menu item and view some helpful informations
Support: *jan4995@gmail.com*
