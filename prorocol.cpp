#include"protocol.h"

//给协议申请动态空间
PDU *mkPDU(uint uiMsgLen){
    uint uiPDULen=sizeof(PDU)+uiMsgLen;//总大小=原始大小+弹性大小
    PDU *pdu=(PDU *)malloc(uiPDULen);//申请空间
    if(NULL==pdu){
        exit(EXIT_FAILURE);//申请空间失败则结束程序
    }
    memset(pdu,0,uiPDULen);//清空(把pdu的内存区域uiPDULen大小空间 的值全部设置为0)
    pdu->uiPDULen=uiPDULen;
    pdu->uiMsgLen=uiMsgLen;
    return pdu;
}



