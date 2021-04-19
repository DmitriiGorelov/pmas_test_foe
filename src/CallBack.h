#include <atomic>

typedef unsigned long long timerFlag_t;

void Emergency_Received(unsigned short usAxisRef, short sEmcyCode) ;
int  CallbackFunc(unsigned char* recvBuffer, short recvBufferSize,void* lpsock);
int SyncTimerCallback();
void flag(timerFlag_t value);
timerFlag_t flag();
