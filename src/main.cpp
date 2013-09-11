
#include "stdafx.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#include "frame.h"

/* Enables the visual style using an inline manifest */
#pragma comment(linker, "\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

HACCEL kMainAccel;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE,
  LPSTR lpCmdLine, int nShowCmd)
{
  kMainAccel = NULL;
  InitCommonControls();
  kInstance = hInstance;
  MainFrame::Register();

  MainFrame *frame = new MainFrame(lpCmdLine);

  frame->Create("DKC2 Editor", 128, 128, 512, 512, NULL);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    //if (!kMainAccel || !TranslateAccelerator(frame->GetHWND(), kMainAccel, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    //}
  }
  
  delete frame;
  _CrtDumpMemoryLeaks();

  return msg.wParam;
}
