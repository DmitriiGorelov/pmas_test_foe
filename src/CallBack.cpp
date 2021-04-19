
#include <iostream>
#include "mmcpplib.h"
#include <thread>

#include "CallBack.h"






// Callback Function once an Emergency is received.
void Emergency_Received(unsigned short usAxisRef, short sEmcyCode)
{
	printf("Emergency Message Received on Axis %d. Code: %x\n",usAxisRef,sEmcyCode) ;
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Function name	:	void callback function																		//
//	Created			:	Version 1.00																				//
//	Updated			:	3/12/2010																					//
//	Modifications	:	N/A																							//
//	Purpose			:	interuprt function 																			//
//																													//
//	Input			:	N/A																							//
//	Output			:	N/A																							//
//	Return Value	:	int																							//
//	Modifications:	:	N/A																							//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CallbackFunc(unsigned char* recvBuffer, short recvBufferSize,void* lpsock)
{
	// Whcih function ID was received ...
	switch(recvBuffer[1])
	{
	case ASYNC_REPLY_EVT:
		printf("ASYNC event Reply\r\n") ;
		break ;
	case EMCY_EVT:
		// Please note - The emergency event was registered.
		// printf("Emergency Event received\r\n") ;
		break ;
	case MOTIONENDED_EVT:
		printf("Motion Ended Event received\r\n") ;
		break ;
	case HBEAT_EVT:
		printf("H Beat Fail Event received\r\n") ;
		break ;
	case PDORCV_EVT:
		printf("PDO Received Event received - Updating Inputs\r\n") ;
		break ;
	case DRVERROR_EVT:
		printf("Drive Error Received Event received\r\n") ;
		break ;
	case HOME_ENDED_EVT:
		printf("Home Ended Event received\r\n") ;
		break ;
	case SYSTEMERROR_EVT:
		printf("System Error Event received\r\n") ;
		break ;
		/* This is commented as a specific event was written for this function. Once it occurs
		 * the ModbusWrite_Received will be called
			case MODBUS_WRITE_EVT:
			// TODO Update additional data to be read such as function parameters.
			// TODO Remove return 0 if you want to handle as part of callback.
			return 0;
			printf("Modbus Write Event received - Updating Outputs\r\n") ;

			break ;
		*/
	}
	//
	return 1 ;
}

static std::atomic<unsigned long long>fl(0);

void flag(unsigned long long value)
{
	fl = value;
}

unsigned long long flag()
{
	return fl;
}

int SyncTimerCallback()
{
	static unsigned long long counter(0);
	counter++;
	if (counter % 1000 == 0)
	{
		//std::thread::id i = std::this_thread::get_id();
		//cout << (counter / 1000) << "' from MMCTimer callback, thread ID = " << i << endl;
		flag(counter / 1000);
	}

	return 1;
}
