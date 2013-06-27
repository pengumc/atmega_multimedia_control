 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usb.h>       
#include "opendevice.h"

#include "../requests.h"
#include "../usbconfig.h"
 
int main(int argc, char **argv)
{
  usb_dev_handle      *handle = NULL;
  const unsigned char rawVid[2] = {USB_CFG_VENDOR_ID}, rawPid[2] = {USB_CFG_DEVICE_ID};
  char                vendor[] = {USB_CFG_VENDOR_NAME, 0}, product[] = {USB_CFG_DEVICE_NAME, 0};
  unsigned char       buffer[12];
  int                 cnt, vid, pid;

  usb_init();
  /* compute VID/PID from usbconfig.h so that there is a central source of information */
  vid = rawVid[1] * 256 + rawVid[0];
  pid = rawPid[1] * 256 + rawPid[0];
  /* The following function is in opendevice.c: */
  if(usbOpenDevice(&handle, vid, vendor, pid, product, NULL, NULL, NULL) != 0){
    fprintf(stderr, "Could not find USB device \"%s\" with vid=0x%x pid=0x%x\n", product, vid, pid);
    exit(1);
  }

while(1){
  cnt = usb_control_msg(handle, USB_TYPE_VENDOR | USB_RECIP_DEVICE | USB_ENDPOINT_IN, USBRQ_GET_LAST_ADC_RESULT, 0, 0, buffer, sizeof(buffer), 5000);
  if(cnt < 1){
    if(cnt < 0){
      fprintf(stderr, "USB error: %s\n", usb_strerror());
      exit(1);
    }else{
      fprintf(stderr, "only %d bytes received.\n", cnt);
    }
  }else{
    system("cls");
    printf("bytes recieved: %d\n",cnt);
    printf("buffer[0] = %d, 0X%X\n",buffer[0], buffer[0]);
    _sleep(200);
  }
  
}
usb_close(handle);
return 0;
}
