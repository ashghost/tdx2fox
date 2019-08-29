#include "../include/fox/FoxFunc.h"

extern "C" int WINAPI PERIOD(CALCINFO *pData)
{
  float period = -1.f;
  switch (pData->m_dataType)
  {
  case MIN1_DATA:
  {
    period = 0.f;
  }
  break;
  case MIN5_DATA:
  {
    period = 1.f;
  }
  break;
  case MIN15_DATA:
  {
    period = 2.f;
  }
  break;
  case MIN30_DATA:
  {
    period = 3.f;
  }
  break;
  case MIN60_DATA:
  {
    period = 4.f;
  }
  break;
  case DAY_DATA:
  {
    period = 5.f;
  }
  break;
  case WEEK_DATA:
  {
    period = 6.f;
  }
  break;
  case MONTH_DATA:
  {
    period = 7.f;
  }
  break;
  case MULTIMIN_DATA:
  {
    period = 8.f;
  }
  break;
  case MULTIDAY_DATA:
  {
    period = 9.f;
  }
  break;
  case MULTISEC_DATA:
  {
    period = 13.f;
  }
  break;
  default:
  {
    return -1;
  }
  break;
  }
  for (auto i = 0; i < pData->m_nNumData; ++i)
  {
    pData->m_pResultBuf[i] = period;
  }
  return 0;
}
