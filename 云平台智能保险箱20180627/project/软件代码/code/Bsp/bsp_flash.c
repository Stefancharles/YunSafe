
/******************************************************************************
******************************************************************************
* @file    bsp/bsp_flash.c 
* @author  硬件部赵纪元
* @version V1.0.0
* @date    2018.06.18
* @brief   flash板级管理包
******************************************************************************
******************************************************************************/
#include "bsp_flash.h"

#define FLASH_START_ADDRESS         (0x8000000)
#define FLASH_BASE_ADDRESS          (0*1024)    //loader size 0K
#define FLASH_LIMIT_MAX_SIZE        (256*2048)  //flash limit max
#define FLASH_BLOCK_SIZE            (2048)      //2K
#define FLASH_BLOCK_NUMBER          (256)       //256个块 数据

#define MIN_FLASH_ADDRESS = FLASH_START_ADDRESS + FLASH_BASE_ADDRESS;
#define MAX_FLASH_ADDRESS = FLASH_START_ADDRESS + FLASH_LIMIT_MAX_SIZE;

#define FLASH_PAGE_SIZE  FLASH_BLOCK_SIZE
#define FLASH_MIN_ADDR   MIN_FLASH_ADDRESS
#define FLASH_MAX_ADDR   MAX_FLASH_ADDRESS

FLASH_Status FLASHStatus = FLASH_COMPLETE;

int32_t bsp_flash_write    (uint32_t addr,uint8_t* pbuf,int32_t len)
{
    uint32_t DATA_32 = 0;
    uint32_t i = 0;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 

    while (i < len)
    {
        if(len>(i+4)) { memcpy((void*)&DATA_32,(void*)&pbuf[i],sizeof(uint32_t)); }
        else          { memcpy((void*)&DATA_32,(void*)&pbuf[i],(len-i) ); }
        
        if (FLASH_ProgramWord(addr+i, DATA_32) == FLASH_COMPLETE)
        {
          i+=4;
        }
        else
        { 
          /* Error occurred while writing data in Flash memory. User can add here some code to deal with this error */
         //   DBG_E("bsp_flash_write error:0x%x ",addr+i);
            while (1);
        }
    }

    /* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
    FLASH_Lock(); 
    return len;
}

int32_t bsp_flash_read		 (uint32_t addr,uint8_t* p_dest,int32_t len)
{
    
    memcpy((void*)p_dest, (void *)addr, len);
    return len;
}




int32_t bsp_flash_erase		 (uint32_t address,uint32_t len)
{
    uint32_t EraseCounter = 0;
    uint32_t NbrOfPage = 0;
    
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
    
    
    NbrOfPage = (len) / FLASH_PAGE_SIZE;
    if(len&(FLASH_PAGE_SIZE-1)){NbrOfPage++;}/*如果字节数大于当前块数*/
    
    for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        if (FLASH_ErasePage(address + (FLASH_PAGE_SIZE * EraseCounter))!= FLASH_COMPLETE)
        {
         /* Error occurred while sector erase. User can add here some code to deal with this error  */
       //     DBG_E("bsp_flash_erase error : address:0x%x ",address + (FLASH_PAGE_SIZE * EraseCounter));
            while (1);
        }
    }
    FLASH_Lock(); 
    return 0;
}

#define FLASH_SAVE_ADDR 0X08070000 
uint8_t test_buffer[]={"Stm32f103 bsp_flash"};

/**********************
//设置 FLASH 保存地址(必须为偶数，且其值要大于本代码所占用 FLASH 的大小+0X08000000)
//本项目 的MCU     falsh 总大小有 512k  
//address 0x08000000
//address 0x08005c00 已用到  23k
//address 0x08070000 开辟的save_addr 地址
//address 0x08080000 end save_addr 地址
***********************/
int32_t demo_flash(void)
{
//	uint8_t read_buffer[sizeof(test_buffer)];
	uint16_t i;
	uint8_t test[1024];
	uint8_t test_read[1024];
    bsp_flash_erase(FLASH_SAVE_ADDR, 1);
    bsp_flash_write(FLASH_SAVE_ADDR,test_buffer,sizeof(test_buffer));
//    bsp_flash_read(FLASH_SAVE_ADDR,read_buffer,sizeof(test_buffer));
//    if(strcmp((const char*)test_buffer,(const char*)read_buffer) == 0)
//    {
//	   printf("二者相等 %.*s\n", sizeof(test_buffer), read_buffer); 
//	   return -1;
//    }
//	else
//	{
//		printf("二者不相等%.*s\n", sizeof(test_buffer), read_buffer); 
//		return 0;
//	}
//	for (i = 1; i < 33; ++i)
//	{
//	  bsp_flash_erase(FLASH_SAVE_ADDR+(2024*i),i);
//      bsp_flash_write(FLASH_SAVE_ADDR+(2024*i),&i,1);
//	}
//	for (i = 1; i < 33; ++i)
//	{
//	  bsp_flash_read(FLASH_SAVE_ADDR+(2024*i),&test,1024);
//	  
//	  printf("%d=[%d]\r\n", i,test); 
//	}
  for (i = 0; i < 1024; ++i)
  {
	test[i]=i;
  }
	bsp_flash_erase(FLASH_SAVE_ADDR, 1);
	bsp_flash_write(FLASH_SAVE_ADDR,test,1024);
	bsp_flash_read(FLASH_SAVE_ADDR,test_read,1024);
  for (i = 0; i < 1024; ++i)
  {
  	printf("%d=[%d]\r\n", i,test_read[i]); 
  }
}

#if 0
int32_t flash_w(uint8_t argc, uint8_t *argv[])
{
    uint32_t wr_addr = 0;
    uint32_t len  = 0;
    uint32_t read_len = 0;
    uint32_t read_bytes = 0;
    uint8_t read_buf[100]={0};
    
    if(argc>=3)
    {
        sscanf((const char*)argv[1],"0x%x",&wr_addr);
        len = strlen((const char*)argv[2]); 
        
        DBG_I("flash_w address:0x%x  len:%d",wr_addr,len);
        bsp_flash_erase(wr_addr,len);
        bsp_flash_write(wr_addr,(uint8_t*)argv[2],len+1);
        
        read_len = (len<99)?(len):(99);
        read_bytes = bsp_flash_read(wr_addr,read_buf,read_len);
        DBG_I("flash read:0x%x len:%d str:%s",wr_addr,read_bytes,(char*)read_buf);
    }     
    else
    {
        DBG_W("flash_w param error! eg: flash_w addr str");
    }
	return RET_OK;
}

REG_SHELL_CMD(flash_w,0,flash_w,"flash write!!", "eg:flash_w addr str \r\n ");


int32_t flash_r(uint8_t argc, uint8_t *argv[])
{
    uint32_t rd_addr = 0;
    uint32_t len  = 0;
    uint32_t read_len = 0;
    uint32_t read_bytes = 0;
    uint8_t read_buf[100]={0};
    
    if(argc>=3)
    {
        sscanf((const char*)argv[1],"0x%x",&rd_addr);
        sscanf((const char*)argv[2],"%d"  ,&len);
        
        read_len = (len<99)?(len):(99);
        read_bytes = bsp_flash_read(rd_addr,read_buf,read_len);
        DBG_I("flash read:0x%x len:%d str:%s",rd_addr,read_bytes,(char*)read_buf);
        show_buf(read_buf,read_bytes);
    }     
    else
    {
        DBG_W("flash_r param error! eg: read addr len");
    }
	return RET_OK;
}

REG_SHELL_CMD(flash_r,0,flash_r,"flash read!!", "eg:read addr len \r\n ");
#endif
