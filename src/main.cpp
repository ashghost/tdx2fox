#include "../include/fox/FoxFunc.h"
#include "../include/tdx/PluginTCalcFunc.h"

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <array>
#include <cassert>

#ifdef UNICODE
using string = std::wstring;
#else
using string = std::string;
#endif

struct TdxDll
{
  TdxDll(const TCHAR *path)
      : dll(::LoadLibrary(path)), lib_path(path)
  {
    if (!dll)
    {
      return;
    }
    init_functions();
  }

  ~TdxDll()
  {
    if (dll)
    {
      ::FreeLibrary(dll);
    }
  }

  bool init_functions()
  {
    pRegisterPluginFUNC reg_fn = (pRegisterPluginFUNC)::GetProcAddress(dll, "RegisterTdxFunc");
    if (!reg_fn)
    {
      return false;
    }
    PluginTCalcFuncInfo *infos = nullptr;
    if (!reg_fn(&infos))
    {
      return false;
    }

    while (infos && infos->nFuncMark != 0 && infos->pCallFunc)
    {
      funcs[infos->nFuncMark] = infos->pCallFunc;
      infos++;
    }
    return true;
  }

  HMODULE dll;
  std::map<int, pPluginFUNC> funcs;
  string lib_path;
};

struct FoxCalcParamValueTransform
{
  FoxCalcParamValueTransform(const CALCPARAM *p, size_t size)
      : param(p)
  {
    if (param->m_nParamStart < 0)
    {
      array_value = std::vector<float>(size, param->m_fParam);
    }
  }

  const float *data() const
  {
    if (array_value.empty())
      return param->m_pfParam;
    return array_value.data();
  }

  const CALCPARAM *param;
  std::vector<float> array_value;
};

using ConstTdxDllPtr = std::shared_ptr<const TdxDll>;

static std::array<ConstTdxDllPtr, 10> dlls;
static const TCHAR *const bands[] = {
    TEXT("band1"),
    TEXT("band2"),
    TEXT("band3"),
    TEXT("band4"),
    TEXT("band5"),
    TEXT("band6"),
    TEXT("band7"),
    TEXT("band8"),
    TEXT("band9"),
    TEXT("band10"),
};
static_assert(std::size(bands) == std::size(dlls), "size(dlls) must equal size(bands)");

static int tdxdll_function_entry(int dll_no, CALCINFO *pData)
{
  const auto &dll = dlls[dll_no];
  if (dll && pData->m_nNumParam == 4 && pData->m_pCalcParam[0].m_nParamStart < 0)
  {
    int function_id = static_cast<int>(pData->m_pCalcParam[0].m_fParam);
    auto i = dll->funcs.find(function_id);
    if (i == dll->funcs.end())
      return -1;
    FoxCalcParamValueTransform argv1(&pData->m_pCalcParam[1], pData->m_nNumData);
    FoxCalcParamValueTransform argv2(&pData->m_pCalcParam[2], pData->m_nNumData);
    FoxCalcParamValueTransform argv3(&pData->m_pCalcParam[3], pData->m_nNumData);
    i->second(pData->m_nNumData, pData->m_pResultBuf, argv1.data(), argv2.data(), argv3.data());
    return 0;
  }
  return -1;
}

#define DEFINE_TDXDLL_FUNCTION_ENTRY(dll_no)                                                                       \
  extern "C" int WINAPI TDXDLL##dll_no(CALCINFO *pData)                                                            \
  {                                                                                                                \
    assert(sizeof(CALCINFO) == pData->m_dwSize);                                                                   \
    static_assert(dll_no > 0 && dll_no <= dlls.size(), "dll_no [" #dll_no "] overflow, must in [1, dlls.size()]"); \
    return tdxdll_function_entry(dll_no - 1, pData);                                                               \
  }

DEFINE_TDXDLL_FUNCTION_ENTRY(1);
DEFINE_TDXDLL_FUNCTION_ENTRY(2);
DEFINE_TDXDLL_FUNCTION_ENTRY(3);
DEFINE_TDXDLL_FUNCTION_ENTRY(4);
DEFINE_TDXDLL_FUNCTION_ENTRY(5);
DEFINE_TDXDLL_FUNCTION_ENTRY(6);
DEFINE_TDXDLL_FUNCTION_ENTRY(7);
DEFINE_TDXDLL_FUNCTION_ENTRY(8);
DEFINE_TDXDLL_FUNCTION_ENTRY(9);
DEFINE_TDXDLL_FUNCTION_ENTRY(10);

#undef DEFINE_TDXDLL_FUNCTION_ENTRY

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID /*lpReserved*/)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
  {
    string path;
    path.resize(MAX_PATH);
    ::GetModuleFileName((HMODULE)hModule, &path[0], path.size());
    string dll_ini_path = path.substr(0, path.find_last_of('\\')) + TEXT("\\dlls.ini");
    TCHAR tdx_dll_path[MAX_PATH];
    for (size_t i = 0; i < dlls.size(); ++i)
    {
      if (::GetPrivateProfileString(TEXT("BAND"), bands[i], TEXT(""), tdx_dll_path, std::size(tdx_dll_path), dll_ini_path.c_str()))
      {
        dlls[i] = ConstTdxDllPtr(new TdxDll(tdx_dll_path));
      }
    }
  }
  break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}
