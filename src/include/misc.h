/*
**
MODULEID("$Id: misc.h,v 1.1 1999/07/26 01:46:59 ayman Exp $")
**
*/

 typedef void signal_f(int);

 signal_f *nsignal (int, signal_f *);
 void InitSignals (void);
 void InitMsredird (void);

