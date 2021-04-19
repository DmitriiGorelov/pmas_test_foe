/*
============================================================================
 Name : PIVarExample.h
 Author :
 Version :
 Description : GMAS C/C++ project header file
============================================================================
*/


/*
============================================================================
 Project general functions prototypes
============================================================================
*/

bool MainInit();
void MachineSequences();
void MainClose();

/*
============================================================================
 Global structures for Elmo's Function Blocks
============================================================================
*/

MMC_CONNECT_HNDL gConnHndl ;
CMMCConnection cConn ;
CMMCSingleAxis 	cAxes[3];
MMC_AXIS_REF_HNDL aref[3];
