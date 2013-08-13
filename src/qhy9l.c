#include "qhy9l.h"
#include "QHYCAM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <opencv/cv.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern QHYCCD QCam;
extern CCDREG ccdreg;

void  CorrectQHY9LWH(int *w,int *h)
{
    if(*w <= 896 && *h <= 644)
    {
	*w = 896;
	*h = 644;
	initQHY9L_896x644();
    }
    else if(*w <= 1792 && *h <= 1287)
    {
	*w = 1792;
	*h = 1287;
	initQHY9L_1792x1287();
    }
    else 
    {
	*w = 3584;
	*h = 2574;
	initQHY9L_3584x2574();
    }
}

void initQHY9L_regs(void)
{
    ccdreg.devname = "QHY9L-0";
    ccdreg.Gain = QCam.camGain;
    ccdreg.Offset = QCam.camOffset;
    ccdreg.Exptime = QCam.camTime;       //unit: ms
    ccdreg.SKIP_TOP = 0;
    ccdreg.SKIP_BOTTOM = 0;
    ccdreg.AMPVOLTAGE = 1;
    ccdreg.LiveVideo_BeginLine = 0;
    ccdreg.AnitInterlace = 0;
    ccdreg.MultiFieldBIN = 0;
    ccdreg.TgateMode = 0;
    ccdreg.ShortExposure = 0;
    ccdreg.VSUB = 0;
    ccdreg.TransferBIT = 0;
    ccdreg.TopSkipNull = 30;
    ccdreg.TopSkipPix = 0;
    ccdreg.MechanicalShutterMode = 0;
    ccdreg.DownloadCloseTEC = 0;
    ccdreg.SDRAM_MAXSIZE = 100;
    ccdreg.ClockADJ = 0x0000;
    ccdreg.CLAMP = 0;
    ccdreg.MotorHeating = 1;
}

void initQHY9L_896x644(void)
{
    ccdreg.HBIN = 2;
    ccdreg.VBIN = 4;
    ccdreg.LineSize = 1792;
    ccdreg.VerticalSize = 644;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 896;
    QCam.cameraH = 644;
   
}

void initQHY9L_1792x1287(void)
{
    ccdreg.HBIN = 2;
    ccdreg.VBIN = 2;
    ccdreg.LineSize = 1792;
    ccdreg.VerticalSize = 1287;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 1792;
    QCam.cameraH = 1287;
}
void initQHY9L_3584x2574(void)
{
    ccdreg.HBIN = 1;
    ccdreg.VBIN = 1;
    ccdreg.LineSize = 3584;
    ccdreg.VerticalSize = 2574;
    ccdreg.DownloadSpeed = QCam.transferspeed;
    QCam.cameraW = 3584;
    QCam.cameraH = 2574;
}

void ConvertQHY9LDataBIN11(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix)
{
     unsigned char *Buf = NULL;

     SWIFT_MSBLSB(ImgData,x,y);

     Buf=(unsigned char *)malloc(x*y*2);
     
     memcpy(Buf,ImgData+TopSkipPix*2,x*y*2);
     memcpy(ImgData,Buf,x*y*2);
     free(Buf);
}



void ConvertQHY9LDataBIN22(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix)
{
    unsigned char *Buf = NULL;

    SWIFT_MSBLSB(ImgData,x,y );

    Buf=(unsigned char *) malloc(x*y*2);
    memcpy(Buf,ImgData+TopSkipPix*2,x*y*2);
    memcpy(ImgData,Buf,x*y*2);
    free(Buf);
}

void ConvertQHY9LDataBIN44(unsigned char *ImgData,int x, int y, unsigned short TopSkipPix)
{
     unsigned char * Buf = NULL;
     unsigned int pix;

     SWIFT_MSBLSB(ImgData,x*2 ,y);

     Buf=(unsigned char *) malloc(x*y*2);

     unsigned long k=0;
     unsigned long s=TopSkipPix*2;

     while(k < x*y*2)
     {
         pix=(ImgData[s]+ImgData[s+1]*256+ImgData[s+2]+ImgData[s+3]*256)/2;
         if (pix>65535) pix=65535;

         Buf[k]  =LSB(pix);
         Buf[k+1]=MSB(pix);
         s=s+4;
         k=k+2;
     }

    memcpy(ImgData,Buf,x*y*2);
    free(Buf);
}

void writec(unsigned char value)
{
	char data[1];
	data[0]=value;
        libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE, 0xbb,0x78,0,data,1, 0);
	//vendTXD_Ex("QHY9L-0",0XBB,1,0x00,0x78,data);
}

void writed(unsigned char value)
{
	char data[1];
	data[0]=value;
	 libusb_control_transfer(QCam.ccd_handle, QHYCCD_REQUEST_WRITE, 0xbb,0x78,0x40,data,1, 0);
       //vendTXD_Ex("QHY9L-0",0XBB,1,0x40,0x78,data);
}
void oled(unsigned char buffer[])
{
	unsigned char i,j;
	unsigned char byte;

	writec(0xAF);
	writec(0x00);    	//set lower column address
	writec(0x10);    	//set higher column address
	writec(0xb0);
	writec(0xB0+6);    	//set page address

	for(i=0;i<128;i++)
	{
		byte=buffer[i];
		writed(byte);
	};
	writec(0x00);    	//set lower column address
	writec(0x10);    	//set higher column address
	writec(0xB0+7);    	//set page address
	for(i=128;i<255;i++)
	{
		byte=buffer[i];
		writed(byte);
	};
}

void send2oled(char message[])
{
	unsigned char data[128*16];
	memset(data,0,128*16);

	IplImage *img = cvCreateImage(cvSize(128,16),8,1);
  img->imageData = data;
  CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX,0.4,0.4,0,0,1);
	cvPutText(img,message,cvPoint(0,15),&font,CV_RGB(255,255,255));

  unsigned char data8[256];

  int e = 0;
	int s = 0;
  int k = 0;
  int i;
	CvScalar pix;
  memset(data8,0,256);

  s = 0;
  e = 0;
  for(k = 0;k < 128;k++)
  {
        while(e < 8)
        {
            pix = cvGet2D(img,e,k);

            if(pix.val[0] > 128)
            {
                data8[s] = data8[s] | (1 << e);
            }
            e++;
         }
         e = 0;
         s++;
  }

  e = 8;
  s = 128;
  for(k = 0;k < 128;k++)
  {
    while(e < 16)
		{
            pix = cvGet2D(img,e,k);

			if(pix.val[0] > 128)
			{
                data8[s] = data8[s] | (1 << (e%8));
			}
			e++;
    }
		e = 8;
    s++;
	}

	oled(data8);
}


#ifdef __cplusplus
}
#endif
