#include "tlv.h"

bool CTlvPacket::WriteInt(int data,bool bMovePtr)  
{  
    int tmp = htonl(data);  
    return Write(&tmp,sizeof(int));  
}  
bool CTlvPacket::Write(const void *pDst,unsigned int uiCount)  
{  
    memcpy(m_pWritePtr,pDst,uiCount);  
    m_pWritePtr += uiCount;  
    return m_pWritePtr < m_pEndData ? true : false;  
}  
bool CTlvPacket::ReadInt(int *data,bool bMovePtr)  
{  
    Read(data,sizeof(int));  
    *data = ntohl(*data);  
    return true;  
}  
bool CTlvPacket::Read(void *pDst,unsigned int uiCount)  
{  
    memcpy(pDst,m_pReadPtr,uiCount);  
    m_pReadPtr += uiCount;  
    return m_pReadPtr < m_pEndData ? true : false;  
}  
