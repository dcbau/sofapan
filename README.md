# sofapan (under development)
Binaural Renderer Plugin. Can load HRTFs in the SOFA file format and customize it. Audio Plugin based on JUCE. 

This is a university project from the University of Applied Sciences Duesseldorf, Germany. 

# Features:
- VST//VST3/AU
- Mac/PC
- 360 degree (3D Full Sphere: Azimuth, Elevation & Distance(when provided)) Sourround Panning of a Mono Input Source
- Load SOFA Files from disc during runtime 
- Binaural Rendering via Fast Convolution
- Access to SOFA Metadata (view only)
- Crossfading between Convolution Products when the HRTF is exchanged (Angle changed) 
- Graphical Representation (Plot) of current HRTF and HRIR 
- Room/Distance Simulation Mode: Direct placing of the source in a virtual room with two 2DPlanes
	- Room is simulated with semistatic early reflections, based on mirror source model
	- Nearfield simulation by using acoustic parallax effect
- ITD customization
        - Modification of HRTF data to fit individual head diameter of the listener

# Planned:
- Documentation
- Better Name
- Dynamic Early Reflections n
- Tuning of Reverberation (still sounds not that pleasant)
- … and of course bugfixes and improved audio quality.

# Additional Dependencies (for building):
- JUCE (Guts of Plugin & Audio Engine)
- FFTW (Fourier Transform/Fast Convolution)
- NetCDF (Access to Data in SOFA Files)
- VST2/3 SDK (For Building the VST)

NetCDF & FFTW are included in the repo. The VST SDK and the JUCE-Modules need to be added in the Jucer-Config. 

# Additional Dependencies (for running the plugins):
- On Windows, NetCDF has to be installed as a shared library to run the plugin (see Install notes in the VST AU Builds folder). On Mac, it is directly compiled into the plugin.

The standart HRTF („mit_kemar_normal_pinna.sofa“, included in the repo) needs to be located at "/Library/Audio/Plug-Ins/VST/SOFAFiles/mit_kemar_normal_pinna.sofa" (Mac) or "C:\\Program Files\\Steinberg\\VstPlugins\\SOFAFiles\\mit_kemar_normal_pinna.sofa" (Win64)
