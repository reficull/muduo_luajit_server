#ifndef _CTLVPACKET
#define _CTLVPACKET 

#include <stdio.h>  
#include <string.h>
#include <string>  
#include <arpa/inet.h>  

class CTlvPacket  
{  
public:  
    CTlvPacket(char *pBuf,unsigned int len):m_pData(pBuf),m_uiLength(len),m_pEndData(m_pData+len),m_pWritePtr(m_pData),m_pReadPtr(m_pData) { }  
    ~CTlvPacket() { }  
  
    bool WriteInt(int data,bool bMovePtr = true);  
  
    bool Write(const void *pDst,unsigned int uiCount);  
  
    bool ReadInt(int *data,bool bMovePtr = true);  
  
    bool Read(void *pDst,unsigned int uiCount);  
  
private:  
    char *m_pData;  
    unsigned int m_uiLength;  
    char *m_pEndData;  
    char *m_pWritePtr;  
    char *m_pReadPtr;  
};  
#endif
