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
#include <list>
#include <malloc.h>

#ifdef WIN32
//#include <windows.h>
#include <chrono>
#include <thread>
#else
#include <time.h>
#endif

#include "ISSUE.h"
#include "CallBack.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */

#ifdef WIN32
#define linfo std::cout 
#define lerr std::cout  
#else
#define linfo std::cout << RESET << GREEN
#define lerr std::cout << RESET << RED
#endif

int mode(1);
std::string filename("Hello1");
std::string slave("0");
std::string password("20000000");
std::string hostaddress("192.168.35.5");
std::string pmasaddress("192.168.35.10");

void uswait(int us)
{
	#ifdef WIN32
		/*HANDLE timer;
		LARGE_INTEGER ft;

		ft.QuadPart = -(10 * us); // Convert to 100 nanosecond interval, negative value indicates relative time

		timer = CreateWaitableTimer(NULL, TRUE, NULL);
		SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
		WaitForSingleObject(timer, INFINITE);
		CloseHandle(timer);*/

		std::this_thread::sleep_for(std::chrono::microseconds(us));
	#else
		usleep(us);
	#endif
}

#ifdef WIN32
const int ac = 6;
#else
const int ac = 4;
#endif

int main(int argc, char* argv[])
{
	if (argc>ac)
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

		i++; // mode
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				args=std::string(args.c_str()+1);
				if (args=="to")
					mode=2;
				if (args=="from")
					mode=1;
			}
		}

		i++; // file name
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				filename=std::string(args.c_str()+1);
			}
		}

		i++; // password
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				password=std::string(args.c_str()+1);
			}
		}

#ifdef WIN32
		i++; // host IP
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				hostaddress = std::string(args.c_str() + 1);
			}
		}

		i++; // pmas IP
		{
			linfo << "Argument #" << i << " = " << argv[i] << std::endl;
			std::string args(argv[i]);

			if (args.find("-") == 0)
			{
				pmasaddress = std::string(args.c_str() + 1);
			}
		}
#else
#endif

	}
	else
	{
		lerr << "ERROR! run with parameters: \n" <<
			"-slave_name (f.e. \"g01\") \n" <<
			"-to/from (to means to slave from PMAS mnt/jffs/usr/, from means from slave to PMAS) \n" <<
			"-file_name \n" <<
			"-password (in hex format, f.e. \"-20000000\" to read/write SD card)\n";
#ifdef WIN32
		lerr <<
			"-host_ip (f.e. \"-192.168.35.5)\" \n" <<
			"-pmas_ip (f.e. \"-192.168.35.10)\" \n";
#else
#endif

			
		lerr << endl;
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
	strcpy_s(in.cAxisName, name);
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
#if WIN32
	linfo << "host: " << hostaddress << " PMAS: " << pmasaddress << std::endl;
	gConnHndl = cConn.ConnectRPCEx(const_cast<char*>(hostaddress.c_str()), const_cast<char*>(pmasaddress.c_str()),
		0x7fffffff, reinterpret_cast<MMC_MB_CLBK>(CallbackFunc));
#else
	gConnHndl = cConn.ConnectIPCEx(0x7fffffff, (MMC_MB_CLBK)CallbackFunc);
#endif


	//gConnHndl = cConn.ConnectIPCEx(0x7fffffff, NULL) ;

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
		uswait(100000);
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

	uswait(100000);// sleep 100ms

	return true;
}

