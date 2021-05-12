/*
============================================================================
 Name : PIVarExample.cpp
 Author :
 Version :
 Description : GMAS C++ project source file
============================================================================
*/
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include "mmcpplib.h"
#include "mmc_definitions.h"
#include <signal.h>
#include <chrono>
#include <cmath>
#include <sys/time.h>
#include <list>
#include <malloc.h>

#include "ISSUE.h"
#include "CallBack.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */

#define linfo std::cout << RESET << GREEN
#define lerr std::cout << RESET << RED

int mode(1);
std::string filename("Hello1");
std::string slave("0");
std::string password("20000000");

int main(int argc, char* argv[])
{
	if (argc>4)
	{
		int i=1; // slave name
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				slave=std::string(args.c_str()+1);
			}
		}

		i=2; // mode
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				args=std::string(args.c_str()+1);
				linfo << args << endl;
				if (args=="to")
					mode=2;
				if (args=="from")
					mode=1;
			}
		}

		i=3; // file name
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				filename=std::string(args.c_str()+1);
			}
		}

		i=4; // password
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				password=std::string(args.c_str()+1);
			}
		}
	}
	else
	{
		lerr << "ERROR! run with parameters: \n-slave_name (f.e. g01) \n-to/from (to means to slave from PMAS mnt/jffs/usr/, from means from slave to PMAS) \n-file_name \n-password (in hex format, f.e. 20000000 to read/write SD card)" << endl;
		return 1;
	}
	else
	{
		lerr << "ERROR! run with parameters: -slave_name -to/from -file_name" << endl;
		return 1;
	}

	//	Initialize system, axes and all needed initializations
	//
	if (MainInit())
	{
		while(MachineSequences())
		{

		}
	}
	//
	//	Close what needs to be closed before program termination
	//
	MainClose();
	//
	//
	return 0;
}

int getAxisRef(const char* name)
{
	MMC_AXISBYNAME_IN in;
	MMC_AXISBYNAME_OUT out;
#ifdef WIN32
	strcpy_s(in.cAxisName, name.c_str());
#else
	strcpy(in.cAxisName, name);
#endif
	out.usAxisIdx = -1;//DeviceID(); // can be -1 !!!;
	if (MMC_GetAxisByNameCmd(gConnHndl, &in, &out) != 0)
	{
		lerr << "AXIS REF ERROR: " << out.usErrorID << endl;
		return -1;
	}
	linfo << "MMC_GetAxisByNameCmd for " << in.cAxisName << ": out.usAxisIdx = " << out.usAxisIdx << endl;
	return out.usAxisIdx;
}

unsigned int HexStringToUInt(const char* s)
{
	unsigned int v = 0;
	while (char c = *s++)
	{
		if (c < '0') return 0; //invalid character

		if (c > '9') //shift alphabetic characters down
		{
			if (c >= 'a') c -= 'a' - 'A'; //upper-case 'a' or higher
			if (c > 'Z') return 0; //invalid character

			if (c > '9') c -= 'A'-1-'9'; //make consecutive with digits
			if (c < '9' + 1) return 0; //invalid character
		}
		c -= '0'; //convert char to hex digit value
		v = v << 4; //shift left by a hex digit
		v += c; //add the current digit
	}

	return v;
}

//	INIT
bool MainInit()
{
	gConnHndl = cConn.ConnectIPCEx(0x7fffffff, NULL) ;

	// Register the callback function for Modbus and Emergency:
	cConn.RegisterEventCallback(MMCPP_EMCY,(void*)Emergency_Received) ;

	int ref=std::stoi(slave);//getAxisRef(slave.c_str());

	MMC_DOWNLOADFOEEX_IN in;
	memset(&in.pcFileName,0,256);
	memcpy(&in.pcFileName, filename.c_str(), filename.length()); // file name to upload/download.
	linfo << "FileName: " << in.pcFileName << endl;
	in.pwSlaveId[0] = ref; // list of slaves IDs to be executed FoE on.
	in.ucSlavesNum=1; // number of slaves to be executed FoE on.
	in.ucOperation=mode; // 1 = FROM DEVICE , 2 = TO DEVICE
	linfo << "mode: " << mode << endl;
	in.ucInitialState = 8; // The Ecat state to move to before the upload/download starts.
	in.ucFinalState = 8; // The Ecat state to move to after the upload/download ends.
	in.ucFileSavedInFlash=1; // 0 = Saved in RAM (/tmp), 1 = Saved in Flash (/mnt/jffs/usr)
	in.ucDeleteFileAfterDownload=0;
	in.ulPassword=HexStringToUInt(password.c_str()); // password to define an action
	memset(&in.ucReservedBytes,0,32);
	//memcpy(&(in.ucReservedBytes[0]), filename.c_str(), filename.length()); // file name to upload/download.

	MMC_DOWNLOADFOEEX_OUT out;

	if (0!=MMC_DownloadFoEEx(gConnHndl, &in, &out))
	{
		lerr << "ERROR UPLOAD #" << " " << out.usErrorID-65536 << endl;
		MMC_RESET_IN inr;
		MMC_RESET_OUT outr;
		inr.ucExecute=1;
		MMC_ResetAsync(gConnHndl,ref,&inr,&outr);
		usleep(100000);
		return false;
	}

	return true;
}

void MainClose()
{
	MMC_CloseConnection(gConnHndl) ;
}

//	Maschine Sequence
bool MachineSequences()
{
	MMC_GETFOESTATUS_OUT outs;

	if (0!=MMC_GetFoEStatus(gConnHndl,&outs))
	{
		lerr << "FOE STATUS: ERROR " << outs.usErrorID << endl;
		return false;
	}
	else
	{
		linfo << "FOE STATUS: " << outs.sFOEStatus << " FOE PROGRESS : " << int(outs.ucProgress) << " FOE STARTED: " << int(outs.ucFOEStarted)
				<< " FOE SLAVE <" << int(outs.pstSlavesErrorID[0].usSlaveID) << "> ERROR: <" << int(outs.pstSlavesErrorID[0].sErrorID) << ">" << std::endl;

	}

	usleep(100000);// sleep 100ms

	return true;
}

