// This file is intended to be included in the middle of a function after
// defining the macro to do what you want.

// NOTE: once we stop targeting anything older than Windows 8, we can drop this file
// and just call FormatMessage.

#if defined(__DINPUT_INCLUDED__)
// -------------------------------------------------------------
// dinput.h error codes
// -------------------------------------------------------------
DXERRMSG(DI_OK, "DI_OK")
DXERRMSG(DI_NOTATTACHED, "DI_NOTATTACHED")
//DXERRMSG(DI_BUFFEROVERFLOW, "DI_BUFFEROVERFLOW")
//DXERRMSG(DI_PROPNOEFFECT, "DI_PROPNOEFFECT")
//DXERRMSG(DI_NOEFFECT, "DI_NOEFFECT")
DXERRMSG(DI_POLLEDDEVICE, "DI_POLLEDDEVICE")
DXERRMSG(DI_DOWNLOADSKIPPED, "DI_DOWNLOADSKIPPED")
DXERRMSG(DI_EFFECTRESTARTED, "DI_EFFECTRESTARTED")
//DXERRMSG(DI_SETTINGSNOTSAVED_ACCESSDENIED, "DI_SETTINGSNOTSAVED_ACCESSDENIED")
//DXERRMSG(DI_SETTINGSNOTSAVED_DISKFULL, "DI_SETTINGSNOTSAVED_DISKFULL")
DXERRMSG(DI_TRUNCATED, "DI_TRUNCATED")
DXERRMSG(DI_TRUNCATEDANDRESTARTED, "DI_TRUNCATEDANDRESTARTED")
DXERRMSG(DI_WRITEPROTECT, "DI_WRITEPROTECT")
DXERRMSG(DIERR_OLDDIRECTINPUTVERSION, "The application requires a newer version of DirectInput.")
DXERRMSG(DIERR_GENERIC, "DIERR_GENERIC")
//DXERRMSG(DIERR_OLDDIRECTINPUTVERSION, "DIERR_OLDDIRECTINPUTVERSION")
DXERRMSG(DIERR_BETADIRECTINPUTVERSION, "The application was written for an unsupported prerelease version of DirectInput.")
DXERRMSG(DIERR_BADDRIVERVER, "The object could not be created due to an incompatible driver version or mismatched or incomplete driver components.")
DXERRMSG(DIERR_DEVICENOTREG, "DIERR_DEVICENOTREG")
DXERRMSG(DIERR_NOTFOUND, "The requested object does not exist.")
//DXERRMSG(DIERR_OBJECTNOTFOUND, "DIERR_OBJECTNOTFOUND")
DXERRMSG(DIERR_INVALIDPARAM, "DIERR_INVALIDPARAM")
DXERRMSG(DIERR_NOINTERFACE, "DIERR_NOINTERFACE")
//DXERRMSG(DIERR_GENERIC, "DIERR_GENERIC")
DXERRMSG(DIERR_OUTOFMEMORY, "DIERR_OUTOFMEMORY")
DXERRMSG(DIERR_UNSUPPORTED, "DIERR_UNSUPPORTED")
DXERRMSG(DIERR_NOTINITIALIZED, "This object has not been initialized")
DXERRMSG(DIERR_ALREADYINITIALIZED, "This object is already initialized")
DXERRMSG(DIERR_NOAGGREGATION, "DIERR_NOAGGREGATION")
DXERRMSG(DIERR_OTHERAPPHASPRIO, "DIERR_OTHERAPPHASPRIO")
DXERRMSG(DIERR_INPUTLOST, "Access to the device has been lost.  It must be re-acquired.")
DXERRMSG(DIERR_ACQUIRED, "The operation cannot be performed while the device is acquired.")
DXERRMSG(DIERR_NOTACQUIRED, "The operation cannot be performed unless the device is acquired.")
//DXERRMSG(DIERR_READONLY, "DIERR_READONLY")
//DXERRMSG(DIERR_HANDLEEXISTS, "DIERR_HANDLEEXISTS")
DXERRMSG(DIERR_INSUFFICIENTPRIVS, "Unable to IDirectInputJoyConfig_Acquire because the user does not have sufficient privileges to change the joystick configuration. & An invalid media type was specified")
DXERRMSG(DIERR_DEVICEFULL, "The device is full. & An invalid media subtype was specified.")
DXERRMSG(DIERR_MOREDATA, "Not all the requested information fit into the buffer. & This object can only be created as an aggregated object.")
DXERRMSG(DIERR_NOTDOWNLOADED, "The effect is not downloaded. & The enumerator has become invalid.")
DXERRMSG(DIERR_HASEFFECTS, "The device cannot be reinitialized because there are still effects attached to it. & At least one of the pins involved in the operation is already connected.")
DXERRMSG(DIERR_NOTEXCLUSIVEACQUIRED, "The operation cannot be performed unless the device is acquired in DISCL_EXCLUSIVE mode. & This operation cannot be performed because the filter is active.")
DXERRMSG(DIERR_INCOMPLETEEFFECT, "The effect could not be downloaded because essential information is missing.  For example, no axes have been associated with the effect, or no type-specific information has been created. & One of the specified pins supports no media types.")
DXERRMSG(DIERR_NOTBUFFERED, "Attempted to read buffered device data from a device that is not buffered. & There is no common media type between these pins.")
DXERRMSG(DIERR_EFFECTPLAYING, "An attempt was made to modify parameters of an effect while it is playing.  Not all hardware devices support altering the parameters of an effect while it is playing. & Two pins of the same direction cannot be connected together.")
DXERRMSG(DIERR_UNPLUGGED, "The operation could not be completed because the device is not plugged in. & The operation cannot be performed because the pins are not connected.")
DXERRMSG(DIERR_REPORTFULL, "SendDeviceData failed because more information was requested to be sent than can be sent to the device.  Some devices have restrictions on how much data can be sent to them.  (For example, there might be a limit on the number of buttons that can be pressed at once.) & No sample buffer allocator is available.")
DXERRMSG(DIERR_MAPFILEFAIL, "A mapper file function failed because reading or writing the user or IHV settings file failed. & A run-time error occurred.")
#endif

#if defined(__DINPUTD_INCLUDED__)
// -------------------------------------------------------------
// dinputd.h error codes
// -------------------------------------------------------------
DXERRMSG(DIERR_NOMOREITEMS, "No more items.")
DXERRMSG(DIERR_DRIVERFIRST, "Device driver-specific codes. Unless the specific driver has been precisely identified, no meaning should be attributed to these values other than that the driver originated the error.")
DXERRMSG(DIERR_DRIVERFIRST + 1, "DIERR_DRIVERFIRST+1")
DXERRMSG(DIERR_DRIVERFIRST + 2, "DIERR_DRIVERFIRST+2")
DXERRMSG(DIERR_DRIVERFIRST + 3, "DIERR_DRIVERFIRST+3")
DXERRMSG(DIERR_DRIVERFIRST + 4, "DIERR_DRIVERFIRST+4")
DXERRMSG(DIERR_DRIVERFIRST + 5, "DIERR_DRIVERFIRST+5")
DXERRMSG(DIERR_DRIVERLAST, "Device installer errors.")
DXERRMSG(DIERR_INVALIDCLASSINSTALLER, "Registry entry or DLL for class installer invalid or class installer not found.")
DXERRMSG(DIERR_CANCELLED, "The user cancelled the install operation. & The stream already has allocated samples and the surface doesn't match the sample format.")
DXERRMSG(DIERR_BADINF, "The INF file for the selected device could not be found or is invalid or is damaged. & The specified purpose ID can't be used for the call.")
#endif

#if defined(_D3D9_H_)
// -------------------------------------------------------------
// d3d9.h error codes
// -------------------------------------------------------------
//DXERRMSG(D3D_OK, "Ok")
DXERRMSG(D3DERR_WRONGTEXTUREFORMAT, "Wrong texture format")
DXERRMSG(D3DERR_UNSUPPORTEDCOLOROPERATION, "Unsupported color operation")
DXERRMSG(D3DERR_UNSUPPORTEDCOLORARG, "Unsupported color arg")
DXERRMSG(D3DERR_UNSUPPORTEDALPHAOPERATION, "Unsupported alpha operation")
DXERRMSG(D3DERR_UNSUPPORTEDALPHAARG, "Unsupported alpha arg")
DXERRMSG(D3DERR_TOOMANYOPERATIONS, "Too many operations")
DXERRMSG(D3DERR_CONFLICTINGTEXTUREFILTER, "Conflicting texture filter")
DXERRMSG(D3DERR_UNSUPPORTEDFACTORVALUE, "Unsupported factor value")
DXERRMSG(D3DERR_CONFLICTINGRENDERSTATE, "Conflicting render state")
DXERRMSG(D3DERR_UNSUPPORTEDTEXTUREFILTER, "Unsupported texture filter")
DXERRMSG(D3DERR_CONFLICTINGTEXTUREPALETTE, "Conflicting texture palette")
DXERRMSG(D3DERR_DRIVERINTERNALERROR, "Driver internal error")
DXERRMSG(D3DERR_NOTFOUND, "Not found")
DXERRMSG(D3DERR_MOREDATA, "More data")
DXERRMSG(D3DERR_DEVICELOST, "Device lost")
DXERRMSG(D3DERR_DEVICENOTRESET, "Device not reset")
DXERRMSG(D3DERR_NOTAVAILABLE, "Not available")
DXERRMSG(D3DERR_OUTOFVIDEOMEMORY, "Out of video memory")
DXERRMSG(D3DERR_INVALIDDEVICE, "Invalid device")
DXERRMSG(D3DERR_INVALIDCALL, "Invalid call")
DXERRMSG(D3DERR_DRIVERINVALIDCALL, "Driver invalid call")
DXERRMSG(D3DERR_WASSTILLDRAWING, "Was Still Drawing")
DXERRMSG(D3DOK_NOAUTOGEN, "The call succeeded but there won't be any mipmaps generated")

// Extended for Windows Vista
DXERRMSG(D3DERR_DEVICEREMOVED, "Hardware device was removed")
DXERRMSG(S_NOT_RESIDENT, "Resource not resident in memory")
DXERRMSG(S_RESIDENT_IN_SHARED_MEMORY, "Resource resident in shared memory")
DXERRMSG(S_PRESENT_MODE_CHANGED, "Desktop display mode has changed")
DXERRMSG(S_PRESENT_OCCLUDED, "Client window is occluded (minimized or other fullscreen)")
DXERRMSG(D3DERR_DEVICEHUNG, "Hardware adapter reset by OS")

// Extended for Windows 7
DXERRMSG(D3DERR_UNSUPPORTEDOVERLAY, "Overlay is not supported")
DXERRMSG(D3DERR_UNSUPPORTEDOVERLAYFORMAT, "Overlay format is not supported")
DXERRMSG(D3DERR_CANNOTPROTECTCONTENT, "Contect protection not available")
DXERRMSG(D3DERR_UNSUPPORTEDCRYPTO, "Unsupported cryptographic system")
DXERRMSG(D3DERR_PRESENT_STATISTICS_DISJOINT, "Presentation statistics are disjoint")
#endif


#if defined(__DSOUND_INCLUDED__)
// -------------------------------------------------------------
// dsound.h error codes
// -------------------------------------------------------------
//DXERRMSG(DS_OK, "")
DXERRMSG(DS_NO_VIRTUALIZATION, "The call succeeded, but we had to substitute the 3D algorithm")
DXERRMSG(DSERR_ALLOCATED, "The call failed because resources (such as a priority level) were already being used by another caller")
DXERRMSG(DSERR_CONTROLUNAVAIL, "The control (vol, pan, etc.) requested by the caller is not available")
//DXERRMSG(DSERR_INVALIDPARAM, "DSERR_INVALIDPARAM")
DXERRMSG(DSERR_INVALIDCALL, "This call is not valid for the current state of this object")
//DXERRMSG(DSERR_GENERIC, "DSERR_GENERIC")
DXERRMSG(DSERR_PRIOLEVELNEEDED, "The caller does not have the priority level required for the function to succeed")
//DXERRMSG(DSERR_OUTOFMEMORY, "Not enough free memory is available to complete the operation")
DXERRMSG(DSERR_BADFORMAT, "The specified WAVE format is not supported")
//DXERRMSG(DSERR_UNSUPPORTED, "DSERR_UNSUPPORTED")
DXERRMSG(DSERR_NODRIVER, "No sound driver is available for use")
DXERRMSG(DSERR_ALREADYINITIALIZED, "This object is already initialized")
//DXERRMSG(DSERR_NOAGGREGATION, "DSERR_NOAGGREGATION")
DXERRMSG(DSERR_BUFFERLOST, "The buffer memory has been lost, and must be restored")
DXERRMSG(DSERR_OTHERAPPHASPRIO, "Another app has a higher priority level, preventing this call from succeeding")
DXERRMSG(DSERR_UNINITIALIZED, "This object has not been initialized")
//DXERRMSG(DSERR_NOINTERFACE, "DSERR_NOINTERFACE")
//DXERRMSG(DSERR_ACCESSDENIED, "DSERR_ACCESSDENIED")
DXERRMSG(DSERR_BUFFERTOOSMALL, "Tried to create a DSBCAPS_CTRLFX buffer shorter than DSBSIZE_FX_MIN milliseconds")
DXERRMSG(DSERR_DS8_REQUIRED, "Attempt to use DirectSound 8 functionality on an older DirectSound object")
DXERRMSG(DSERR_SENDLOOP, "A circular loop of send effects was detected")
DXERRMSG(DSERR_BADSENDBUFFERGUID, "The GUID specified in an audiopath file does not match a valid MIXIN buffer")
DXERRMSG(DSERR_OBJECTNOTFOUND, "The object requested was not found (numerically equal to DMUS_E_NOT_FOUND)")

DXERRMSG(DSERR_FXUNAVAILABLE, "Requested effects are not available")
#endif

#if defined(__d3d10_h__)
// -------------------------------------------------------------
// d3d10.h error codes
// -------------------------------------------------------------
DXERRMSG(D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS, "There are too many unique state objects.")
DXERRMSG(D3D10_ERROR_FILE_NOT_FOUND, "File not found")
#endif

#if defined(__dxgitype_h__)
// -------------------------------------------------------------
// dxgi.h error codes
// -------------------------------------------------------------
DXERRMSG(DXGI_STATUS_OCCLUDED, "The target window or output has been occluded. The application should suspend rendering operations if possible.")
DXERRMSG(DXGI_STATUS_CLIPPED, "Target window is clipped.")
DXERRMSG(DXGI_STATUS_NO_REDIRECTION, "")
DXERRMSG(DXGI_STATUS_NO_DESKTOP_ACCESS, "No access to desktop.")
DXERRMSG(DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE, "")
DXERRMSG(DXGI_STATUS_MODE_CHANGED, "Display mode has changed")
DXERRMSG(DXGI_STATUS_MODE_CHANGE_IN_PROGRESS, "Display mode is changing")
DXERRMSG(DXGI_ERROR_INVALID_CALL, "The application has made an erroneous API call that it had enough information to avoid. This error is intended to denote that the application should be altered to avoid the error. Use of the debug version of the DXGI.DLL will provide run-time debug output with further information.")
DXERRMSG(DXGI_ERROR_NOT_FOUND, "The item requested was not found. For GetPrivateData calls, this means that the specified GUID had not been previously associated with the object.")
DXERRMSG(DXGI_ERROR_MORE_DATA, "The specified size of the destination buffer is too small to hold the requested data.")
DXERRMSG(DXGI_ERROR_UNSUPPORTED, "Unsupported.")
DXERRMSG(DXGI_ERROR_DEVICE_REMOVED, "Hardware device removed.")
DXERRMSG(DXGI_ERROR_DEVICE_HUNG, "Device hung due to badly formed commands.")
DXERRMSG(DXGI_ERROR_DEVICE_RESET, "Device reset due to a badly formed commant.")
DXERRMSG(DXGI_ERROR_WAS_STILL_DRAWING, "Was still drawing.")
DXERRMSG(DXGI_ERROR_FRAME_STATISTICS_DISJOINT, "The requested functionality is not supported by the device or the driver.")
DXERRMSG(DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE, "The requested functionality is not supported by the device or the driver.")
DXERRMSG(DXGI_ERROR_DRIVER_INTERNAL_ERROR, "An internal driver error occurred.")
DXERRMSG(DXGI_ERROR_NONEXCLUSIVE, "The application attempted to perform an operation on an DXGI output that is only legal after the output has been claimed for exclusive owenership.")
DXERRMSG(DXGI_ERROR_NOT_CURRENTLY_AVAILABLE, "The requested functionality is not supported by the device or the driver.")
DXERRMSG(DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED, "Remote desktop client disconnected.")
DXERRMSG(DXGI_ERROR_REMOTE_OUTOFMEMORY, "Remote desktop client is out of memory.")
#endif

#if defined(__d3d11_h__)
// -------------------------------------------------------------
// d3d11.h error codes
// -------------------------------------------------------------
DXERRMSG(D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS, "There are too many unique state objects.")
DXERRMSG(D3D11_ERROR_FILE_NOT_FOUND, "File not found")
DXERRMSG(D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS, "Therea are too many unique view objects.")
DXERRMSG(D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD, "Deferred context requires Map-Discard usage pattern")
#endif