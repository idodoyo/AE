HANDLE _stdcall OpenUA300();
HANDLE _stdcall OpenUA300m(int kn);

HANDLE _stdcall CloseUA300(HANDLE hUsbAd);
void _stdcall ssinit(HANDLE hUsbAd);
short int _stdcall ssad(HANDLE hUsbAd,short int adch,short int gain);
UCHAR _stdcall GetVer(HANDLE hUsbAd);
void _stdcall minit(HANDLE hUsbAd, short fch,short chn,short int gain);
void _stdcall minit4(HANDLE hUsbAd, short fch,short chn,short int gain);
void _stdcall readdata(HANDLE hUsbAd, short *addat,unsigned short fcode, long leng);
void _stdcall dout(HANDLE hUsbAd, UCHAR ddat);
UCHAR _stdcall din(HANDLE hUsbAd);
void _stdcall endread2(HANDLE hUsbAd);
void _stdcall readdata2(HANDLE hUsbAd, short *addat, long leng);
UCHAR _stdcall readdata3(HANDLE hUsbAd, short *addat, long leng);

void _stdcall minit2(HANDLE hUsbAd, short fch, short chn,unsigned short fcode, short gain);
void _stdcall outb(HANDLE hUsbAd, UCHAR port ,UCHAR dat1);
UCHAR _stdcall inb(HANDLE hUsbAd, UCHAR port);
void _stdcall writedata(HANDLE hUsbKbd, short *addat, long leng);
void _stdcall minit1(HANDLE hUsbAd, short fch,short chn,short int gain,short tr);
void _stdcall startad(HANDLE hUsbKbd, short fch, short chn,unsigned short fcode, short gain);
void _stdcall writebyte(HANDLE hUsbKbd, unsigned char *addat, long leng);

