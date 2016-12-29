#ifndef __PICTURE_DRIVER_H
#define __PICTURE_DRIVER_H
#include "rtthread.h"
#include <dfs_posix.h>
#include "GUI.h"
#include "DIALOG.h"


	void _ShowBMPEx(const char * sFilename, U16 usPOSX, U16 usPOSY);
 void _ShowJPGEx(const char * sFilename, U16 usPOSX, U16 usPOSY);
	void _ShowGIFEx(const char * sFilename);
	void _ShowPNGEx(const char * sFilename, U16 usPOSX, U16 usPOSY);
	
	void _ShowBMP_RAM(const char * sFilename);
	void _ShowPNG_RAM(const char * sFilename, U16 usPOSX, U16 usPOSY);
	void _ShowGIF_RAM(const char * sFilename);
	void _ShowJPG_RAM(const char * sFilename);
	
#endif
