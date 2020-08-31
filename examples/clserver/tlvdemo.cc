#include <stdio.h>  
#include <string.h>
#include <string>  
#include <arpa/inet.h>  
#include "tlv.h"  
  
enum emTLVNodeType  
{  
    emTlvNNone = 0,  
    emTlvNRoot,         //根节点  
    emTlvName,          //名字  
    emTlvAge,           //年龄  
    emTlvColor          //颜色 1 白色 2 黑色  
};  
  
  
typedef struct _CAT_INFO  
{  
    char szName[12];  
    int iAge;  
    int iColor;  
}CAT_INFO,*LPCAT_INFO;  
  
  
/* 
 
格式： 
    root L1 V 
        T L V T L V T L V 
 
    L1 的长度即为“T L V T L V T L V”的长度 
 
*/  
  
int TLV_EncodeCat(LPCAT_INFO pCatInfo, char *pBuf, int &iLen)  
{  
    if (!pCatInfo || !pBuf)  
    {  
        return -1;  
    }  
  
    CTlvPacket enc(pBuf,iLen);  
    enc.WriteInt(emTlvNRoot);  
    enc.WriteInt(20+12+12); //根节点emTlvNRoot中的L，20=4+4+12，12=4+4+4，12=4+4+4  
  
    enc.WriteInt(emTlvName);  
    enc.WriteInt(12);  
    enc.Write(pCatInfo->szName,12);  
  
    enc.WriteInt(emTlvAge);  
    enc.WriteInt(4);  
    enc.WriteInt(pCatInfo->iAge);  
  
    enc.WriteInt(emTlvColor);  
    enc.WriteInt(4);  
    enc.WriteInt(pCatInfo->iColor);  
  
    iLen = 8+20+12+12; //总长度再加上emTLVNRoot的T和L，8=4+4  
  
    return 0;  
}  
  
int TLV_DecodeCat(char *pBuf, int iLen, LPCAT_INFO pCatInfo)  
{  
    if (!pCatInfo || !pBuf)  
    {  
        return -1;  
    }  
  
    CTlvPacket encDec(pBuf,iLen);  
    int iType;  
    int iSum,iLength;  
  
    encDec.ReadInt(&iType);  
    if (emTlvNRoot != iType)  
    {  
        return -2;  
    }  
    encDec.ReadInt(&iSum);  
  
    while (iSum > 0)  
    {  
        encDec.ReadInt(&iType);  
        encDec.ReadInt(&iLength);  
        switch(iType)  
        {  
        case emTlvName:  
            encDec.Read(pCatInfo->szName,12);  
            iSum -= 20;  
            break;  
        case emTlvAge:  
            encDec.ReadInt(&pCatInfo->iAge);  
            iSum -= 12;  
            break;  
        case emTlvColor:  
            encDec.ReadInt(&pCatInfo->iColor);  
            iSum -= 12;  
            break;  
        default:  
            printf("TLV_DecodeCat unkonwn error. \n");  
            break;  
        }  
    }  
  
    return 0;  
}  
  
int main(int argc, char* argv[])  
{  
  
    int iRet, iLen;  
    char buf[256] = {0};  
  
    CAT_INFO cat;  
    memset(&cat,0,sizeof(cat));  
    strcpy(cat.szName,"Tom");  
    cat.iAge = 5;  
    cat.iColor = 2;  
  
    iRet = TLV_EncodeCat(&cat,buf,iLen);  
    if ( 0 == iRet )  
    {  
        printf("TLV_EncodeCat ok, iLen = %d. \n",iLen);  
    }  
    else  
    {  
        printf("TLV_EncodeCat error \n");  
    }  
  
    memset(&cat,0,sizeof(cat));  
    iRet = TLV_DecodeCat(buf,iLen,&cat);  
    if ( 0 == iRet )  
    {  
        printf("TLV_DecodeCat ok, cat name = %s, age = %d, color = %d. \n",cat.szName,cat.iAge,cat.iColor);  
    }  
    else  
    {  
        printf("TLV_DecodeCat error, code = %d. \n", iRet);  
    }  
  
    int iWait = getchar();  
    return 0;  
}  
