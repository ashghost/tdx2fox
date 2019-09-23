#include "../include/fox/FoxFunc.h"
#include <algorithm>

struct FoxCalcParamValue
{
  FoxCalcParamValue(const CALCPARAM *p)
    : param(p)
  {
  }

  int start() const
  {
    return param->m_nParamStart;
  }

  float data(int i) const
  {
    if (param->m_nParamStart < 0)
      return param->m_fParam;

    return param->m_pfParam[i];
  }

  float data() const
  {
    return param->m_fParam;
  }

  float first_data() const
  {
    return param->m_pfParam[start()];
  }

  bool is_literal() const
  {
    return param->m_nParamStart < 0;
  }

  const CALCPARAM *param;
};

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

extern "C" int WINAPI REF(CALCINFO* pData)
{
  if (pData->m_nNumParam != 2)
    return -1;
  FoxCalcParamValue X(&pData->m_pCalcParam[0]);
  FoxCalcParamValue N(&pData->m_pCalcParam[1]);

  float first_data = X.first_data();

  for (auto i = X.start(); i < pData->m_nNumData; ++i)
  {
    int n = static_cast<int>(N.data(i));
    if (n > 0)
    {
      if ((i - X.start()) >= n)
      {
        pData->m_pResultBuf[i] = X.data(i - n);
      }
      else
      {
        pData->m_pResultBuf[i] = first_data;
      }
    }
  }

  return X.start();
}

extern "C" int WINAPI SMA(CALCINFO* pData)
{
  if (pData->m_nNumParam != 3)
    return -1;
  FoxCalcParamValue X(&pData->m_pCalcParam[0]);
  FoxCalcParamValue N(&pData->m_pCalcParam[1]);
  FoxCalcParamValue M(&pData->m_pCalcParam[2]);

  float y = X.data(X.param->m_nParamStart);

  if (N.is_literal() && M.is_literal())
  {
    auto n = (std::max)(N.data(0), 1.f);
    auto m = M.data(0);
    for (int i = X.start(); i < pData->m_nNumData; ++i)
    {
      y = (X.data(i) * m + y * (n - m)) / n;
      pData->m_pResultBuf[i] = y;
    }
    return X.start();
  }

  for (int i = X.start(); i < pData->m_nNumData; ++i)
  {
    auto n = (std::max)(N.data(i), 1.f);
    auto m = M.data(i);
    y = (X.data(i) * m + y * (n - m)) / n;
    pData->m_pResultBuf[i] = y;
  }

  return X.start();
}