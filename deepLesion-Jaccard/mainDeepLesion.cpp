
#pragma hdrstop
#include "appDeepLesion.h"

#pragma argsused
int main(int argc, char *argv[])
{
   AppDeepLesion app;

   app.Init();

   app.Run();

   app.Done();

   return 0;
}
