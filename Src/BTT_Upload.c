/***
 * Implement BigTreeTEch uploader here or at least implement readt back of icons for display demo purpose 
 * 
 * 
 */ 
#include "BTT_Upload.h"
#include "main.h"
#include "stdint.h"
#define PADDING 10
#include "main.h"
#include "ff.h"
#include "fatfs.h"
#include "w25qxx.h"


BMPUPDATE_STAT bmp_stat = BMP_SUCCESS;


#if 0
bool bmpDecode(char *bmp, u32 addr)
{
  FIL   bmpFile;
  char  magic[2];
  int   w,h,bytePerLine;
  short bpp;
  int   offset;
  u8    buf[256];
  u8    lcdcolor[4];
  u16   bnum=0;
  UINT  mybr;

  GUI_COLOR pix;

  if(f_open(&bmpFile,bmp,FA_OPEN_EXISTING | FA_READ)!=FR_OK)
  {
    bmp_stat = BMP_NOTFOUND;
    return false;
  }
  f_read(&bmpFile, magic, 2 ,&mybr);
  if (memcmp(magic, "BM", 2)){
    f_close(&bmpFile);
    bmp_stat = BMP_INVALIDFILE;
    return false;
  }

  f_lseek(&bmpFile, 10);
  f_read(&bmpFile, &offset, sizeof(int),&mybr);

  f_lseek(&bmpFile, 18);
  f_read(&bmpFile, &w, sizeof(int),&mybr);
  f_read(&bmpFile, &h, sizeof(int),&mybr);
  bmp_size.x = w;
  bmp_size.y = h;
  f_lseek(&bmpFile, 28);
  f_read(&bmpFile, &bpp, sizeof(short),&mybr);
  if(bpp<24){
    f_close(&bmpFile);
    bmp_stat = BMP_NOT24BIT;
    return false;
  }
  bpp >>=3;
  bytePerLine=w*bpp;
  if(bytePerLine%4 !=0) //bmp
    bytePerLine=(bytePerLine/4+1)*4;

  for(bnum=0; bnum<(w*h*2+W25QXX_SECTOR_SIZE-1)/W25QXX_SECTOR_SIZE; bnum++)
  {
  W25qxx_EraseSector(addr+bnum*W25QXX_SECTOR_SIZE);
  }
  bnum=0;

  for(int j=0; j<h; j++)
  {
    f_lseek(&bmpFile, offset+(h-j-1)*bytePerLine);
    for(int i=0; i<w; i++)
    {
      f_read(&bmpFile,(char *)&lcdcolor,bpp,&mybr);

      pix.RGB.r=lcdcolor[2]>>3;
      pix.RGB.g=lcdcolor[1]>>2;
      pix.RGB.b=lcdcolor[0]>>3;

      //GUI_DrawPixel(iconUpdateRect.x0 + i,iconUpdateRect.y0 + j,pix.color);

      buf[bnum++]=(u8)(pix.color>>8);
      buf[bnum++]=(u8)(pix.color&0xFF);

      if(bnum == 256)
      {
        W25Qxx_WritePage(buf,addr,256);
        addr+=256;
        bnum=0;
      }
    }
  }

  W25Qxx_WritePage(buf,addr,bnum);
  addr+=bnum;
  f_close(&bmpFile);
  bmp_stat = BMP_SUCCESS;
  return true;
}

void processIcon(char * path, u32 flashAddr)
{
  static u16 found = 0;
  static u16 notfound = 0;
  char tempstr[50];
    //display bmp update success
    GUI_POINT last_size = bmp_size;
    if (bmpDecode(path, flashAddr))
    {
      found++;
   //   GUI_ClearRect(iconUpdateRect.x0,iconUpdateRect.y0,iconUpdateRect.x0 + last_size.x,iconUpdateRect.y0 + last_size.y);
  //    ICON_CustomReadDisplay(iconUpdateRect.x0, iconUpdateRect.y0, bmp_size.x, bmp_size.y, flashAddr);
    }
    //display bmp update fail
    else
    {
      notfound++;
   //   GUI_ClearRect(iconUpdateRect.x0,iconUpdateRect.y0,iconUpdateRect.x0 + last_size.x,iconUpdateRect.y0 + last_size.y);
      dispIconFail((u8 *)path);
    }
    // Display icon update progress
    sprintf(tempstr, "Updated: %d | Not Updated: %d", found, notfound);
  //  GUI_DispString(statUpdateRect.x0, statUpdateRect.y0, (u8 *)tempstr);
}

void updateIcon(void)
{
  static u16 found = 0;
  static u16 notfound = 0;
  char nowBmp[64];
 // GUI_Clear(infoSettings.bg_color);
 // GUI_DispString(5, PADDING, (u8 *)"Updating Logo");
 // GUI_ClearPrect(&iconUpdateRect);
  if (bmpDecode(BMP_ROOT_DIR "/Logo.bmp", LOGO_ADDR))
  {
    LOGO_ReadDisplay();
    found++;
  }
  else
  {
    notfound++;
    dispIconFail((u8 *)(BMP_ROOT_DIR "/Logo.bmp"));
  }

 // GUI_Clear(infoSettings.bg_color);
 // GUI_DispString(5, PADDING, (u8 *)"Updating Icons");

  for (int i = 0; i < COUNT(iconBmpName); i++)
  {
    sprintf(nowBmp, BMP_ROOT_DIR "/%s.bmp", iconBmpName[i]);
  //  GUI_ClearPrect(&labelUpdateRect);
   // GUI_DispString(labelUpdateRect.x0, labelUpdateRect.y0, (u8 *)nowBmp);

    processIcon(nowBmp, ICON_ADDR(i));
  }
/*
  for (int i = 0; i < COUNT(smallIconBmpName); i++)
  {
    sprintf(nowBmp, BMP_ROOT_DIR "/%s_small.bmp", smallIconBmpName[i]);
    GUI_ClearPrect(&labelUpdateRect);
    GUI_DispString(labelUpdateRect.x0, labelUpdateRect.y0, (u8 *)nowBmp);

    processIcon(nowBmp, SMALL_ICON_ADDR(i));
  }
*/
  if (bmpDecode(BMP_ROOT_DIR "/InfoBox.bmp", INFOBOX_ADDR))
  {
    ICON_CustomReadDisplay(iconUpdateRect.x0, iconUpdateRect.y0, bmp_size.x, bmp_size.y, INFOBOX_ADDR);
    found++;
  }
  else
  {
    notfound++;
    dispIconFail((u8 *)(BMP_ROOT_DIR "/InfoBox.bmp"));
  }
}

void dispIconFail(u8 *lbl)
{
//  GUI_SetColor(RED);
 // GUI_ClearPrect(&labelFailedRect);
 // GUI_DispString(labelFailedRect.x0, labelFailedRect.y0, lbl);
  u8 *stat_txt;
  switch (bmp_stat)
  {
  case BMP_INVALIDFILE:
    stat_txt = (u8 *)("BMP file not valid ");
    break;
  case BMP_NOT24BIT:
    stat_txt = (u8 *)("Format is not 24Bit");
    break;
  case BMP_NOTFOUND:
  default:
    stat_txt = (u8 *)("BMP file not found ");
    break;
  }
  char error_txt[30];
  sprintf(error_txt, "Error: %s", stat_txt);
 // GUI_DispString(labelFailedRect.x0, labelFailedRect.y0 + BYTE_HEIGHT + 2, (u8*)error_txt);
 // GUI_RestoreColorDefault();
  Delay_ms(1000); // give some time to the user to read failed icon name.
}
#endif
bool f_file_exists(const TCHAR* path) {
  FIL tmp;
  if (f_open(&tmp, path, FA_OPEN_EXISTING) == FR_OK) {
    f_close(&tmp);
    return true;
  }
  return false;
}

bool f_dir_exists(const TCHAR* path) {
  DIR tmp;
  if (f_opendir(&tmp, path) == FR_OK) {
    f_closedir(&tmp);
    return true;
  }
  return false;
}

void updateFont(char *font, u32 addr)
{
  u8   progress = 0;
  UINT rnum = 0;
  u32  offset = 0;
  char buffer[128];
  FIL  myfp;
  u8*  tempbuf = NULL;
  //UART_Puts(_USART1,"testing");
  if (f_open(&myfp, font, FA_OPEN_EXISTING|FA_READ) != FR_OK)  return;

  tempbuf = malloc(W25QXX_SECTOR_SIZE);
  if (tempbuf == NULL)  return;
 // GUI_Clear(infoSettings.bg_color);
  sprintf((void *)buffer,"%s Size: %dKB",font, (u32)f_size(&myfp)>>10);
 // GUI_DispString(0, 100, (u8*)buffer);
  //GUI_DispString(0, 140, (u8*)"Updating:   %");

  while(!f_eof(&myfp))
  {
    if (f_read(&myfp, tempbuf, W25QXX_SECTOR_SIZE, &rnum) != FR_OK) break;

   W25qxx_EraseSector(addr + offset);
    W25Qxx_WriteBuffer(tempbuf, addr + offset, W25QXX_SECTOR_SIZE);
    offset += rnum;
    if(progress != offset * 100 / f_size(&myfp))
    {
      progress = offset * 100 / f_size(&myfp);
     // GUI_DispDec(0 + BYTE_WIDTH*9, 140, progress, 3, RIGHT);
    }
    if(rnum !=W25QXX_SECTOR_SIZE)break;
  }

  f_close(&myfp);
  free(tempbuf);
}

void scanResetDir(void) {
  if (f_file_exists(TFT_RESET_FILE)) {
    if (f_file_exists(TFT_RESET_FILE ".DONE")) {
      f_unlink(TFT_RESET_FILE ".DONE");
    }
    infoSettingsReset();
    LCD_RefreshDirection();
    TSC_Calibration();
    storePara();
    f_rename(TFT_RESET_FILE, TFT_RESET_FILE ".DONE");
  }
}

void scanRenameUpdate(void) {
    /*
  if (f_file_exists(ADMIN_MODE_FILE)) return; // admin mode, need not rename

  if (f_dir_exists(ROOT_DIR)) { // ROOT_DIR exists
    if (f_dir_exists(ROOT_DIR ".CUR")) { // old ROOT_DIR also exists
    //  GUI_Clear(infoSettings.bg_color);
      // It will take some time to delete the old ROOT_DIR, so display "Deleting" on the screen to tell user.
    //  GUI_DispStringInRect(0, 0, LCD_WIDTH, LCD_HEIGHT, (uint8_t *)"Deleting old ROOT_DIR...");
      f_remove_full_dir(ROOT_DIR ".CUR");
    }
    f_rename(ROOT_DIR, ROOT_DIR ".CUR");
  }*/
/*
  if (f_file_exists(FIRMWARE_NAME ".bin")) { // firmware exists
    if (f_file_exists(FIRMWARE_NAME ".CUR")) { // old firmware also exists
      f_unlink(FIRMWARE_NAME ".CUR");
    }
    f_rename(FIRMWARE_NAME ".bin", FIRMWARE_NAME ".CUR");
  }
  if (f_file_exists(CONFIG_FILE_PATH)) { // config exists
    if (f_file_exists(CONFIG_FILE_PATH ".CUR")) { // old config also exists
      f_unlink(CONFIG_FILE_PATH ".CUR");
    }
    f_rename(CONFIG_FILE_PATH, CONFIG_FILE_PATH ".CUR");
  }
  */
}

void scanUpdates(void)
{
  if(mountSDCard()) {
    if (f_dir_exists(FONT_ROOT_DIR)) {
      updateFont(FONT_ROOT_DIR"/byte_ascii.fon", BYTE_ASCII_ADDR);
      updateFont(FONT_ROOT_DIR"/word_unicode.fon", WORD_UNICODE);
      updateFont(FONT_ROOT_DIR"/large_byte_ascii.fon", LARGE_FONT_ADDR);
    }
    if (f_dir_exists(BMP_ROOT_DIR)) {
   //   updateIcon();
    }
    getConfigFromFile();
    scanRenameUpdate();
    scanResetDir();
  }
}
