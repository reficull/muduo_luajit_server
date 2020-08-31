#include <iostream>
#include <stdlib.h>
#include <string>  
#include "tlv.h"
using namespace std;
 
struct TLVNODE
{
    uint8_t tag;
    uint16_t len;
    char value[0];
}__attribute__ ((packed));
 
struct TLV
{
    int hei;
    uint8_t tag;
    uint16_t len;
    struct TLVNODE value[0];
} __attribute__ ((packed));

struct TLVComplex
{
    int hei;
    uint8_t tag;
    uint16_t len;
    struct TLV value[0];
} __attribute__ ((packed));
 
 
int main()
{
    //char *szMsg = "aaaaaaaaaaa";
    cout << sizeof(TLV) << endl;
    //uint16_t len = strlen(szMsg) + 1;
 
    char szNodeMsg[] = "bbbbbbbbbb";
    uint16_t nodelen = strlen(szNodeMsg) + 1;
    struct TLVNODE *pNode = (struct TLVNODE *) malloc(sizeof(struct TLVNODE) + sizeof(char)*nodelen);
    pNode->tag = 0x3;
    pNode->len = nodelen;
    memcpy(pNode->value, szNodeMsg, nodelen);
 
 
    struct TLV *pTlv;
    uint16_t nodeSize = sizeof(struct TLVNODE) + sizeof(char)*nodelen;
    pTlv = (struct TLV*)malloc(sizeof(struct TLV) + nodeSize);
    pTlv->tag = 0x2;
    pTlv->len = nodeSize;
//    pTlv->value[0] = (struct TLVNODE)*pNode;
    memcpy(pTlv->value, pNode, nodeSize);
    free(pNode);
    pNode = NULL;
    cout << sizeof(*pTlv) << endl;
    /*for (int i = 0; i < len; ++i)
    {
        pTlv->value[i] = szMsg[i];
    }*/
 
    /*memcpy(pTlv->value, szMsg, len);*/
    //cout << pTlv->value << endl;
    free(pTlv);
    pTlv = NULL;
    return 0;
}
