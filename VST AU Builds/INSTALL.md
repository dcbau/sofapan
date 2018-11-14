Only 64bit precompiled Plugins. 32bit Plugins have to be compiled by yourself, if you really need some. 

## WINDOWS

1) Copy the VST3 and VST2 to the desired VST Folder, e.g.
	- `C:/Program Files/Common Files/VST3` (for VST3)
	- `C:/Program Files/Steinberg/VSTPlugins` (for VST2)

2) Download the newest NetCDF PreBuilt Release and install it (www.unidata.ucar.edu/downloads/netcdf/index.jsp). Alternatively, copy the files `hdf5.lib`, `hdf5_hl.lib`, `zlib.lib` and `netcdf.lib` from `Dependencies/netCDF/lib` to your `C:/Windows/System32` . This should be sufficient. 

3) Copy the Default HRTF `mit_kemar_normal_pinna.sofa` from the SOFAFiles Folder to `C:/Program Files/Steinberg/VSTPlugins/SOFAFiles/`

## MAC

1) Copy the VST3, VST2 and AU to the desired VST Folder, e.g.
	- `/Library/Audio/Plug-Ins/VST`
	- `/Library/Audio/Plug-Ins/VST3`
 	- `/Library/Audio/Plug-Ins/Components`

2) Copy the Default HRTF `mit_kemar_normal_pinna.sofa` from the SOFAFiles Folder to `/Library/Audio/Plug-Ins/VST/SOFAFiles/` 
