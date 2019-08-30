#include "../include/fox/FoxFunc.h"
#include "../include/tdx/PluginTCalcFunc.h"

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <array>
#include <cassert>
#include <atomic>

#ifdef UNICODE
using string = std::wstring;
template <typename... Args>
auto to_string(Args &&... args)
{
  return std::to_wstring(std::forward<Args>(args)...);
}
#else
using string = std::string;
template <typename... Args>
auto to_string(Args &&... args)
{
  return std::to_string(std::forward<Args>(args)...);
}
#endif

struct TdxDll
{
  TdxDll(HMODULE handle, const TCHAR *path)
      : dll(handle), lib_path(path)
  {
    assert(handle);
    init_functions();
  }

  ~TdxDll()
  {
    ::FreeLibrary(dll);
  }

  TdxDll(const TdxDll &) = delete;
  TdxDll &operator=(const TdxDll &) = delete;

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

struct TdxDllLoader
{
  static constexpr size_t DLL_COUNT = 10;
  static bool copy_dll;
  static std::array<string, DLL_COUNT> dll_path;

  static HMODULE LoadLibrarySP(const TCHAR *path)
  {
    if (!copy_dll)
    {
      return ::LoadLibrary(path);
    }
    static std::atomic_uint_fast32_t seq = 0;
    uint32_t id = seq++;
    string tmpdir(MAX_PATH, 0);
    auto length = ::GetTempPath(std::size(tmpdir), &tmpdir[0]);
    tmpdir.resize(length);
    tmpdir += TEXT("TDX2FOX-DLL-COPY-") + to_string(id) + TEXT(".dll");
    if (::CopyFile(path, tmpdir.c_str(), FALSE))
    {
      return ::LoadLibrary(tmpdir.c_str());
    }
    return NULL;
  }

  TdxDllLoader()
  {
    for (size_t i = 0; i < dlls.size(); i++)
    {
      if (!dll_path[i].empty())
      {
        HMODULE handle = LoadLibrarySP(dll_path[i].c_str());
        if (handle)
        {
          dlls[i] = std::make_shared<TdxDll>(handle, dll_path[i].c_str());
        }
      }
    }
  }

  std::array<ConstTdxDllPtr, DLL_COUNT> dlls;
};

std::array<string, TdxDllLoader::DLL_COUNT> TdxDllLoader::dll_path;
bool TdxDllLoader::copy_dll = false;

static const TCHAR *const KEY_BANDS[] = {
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
static_assert(std::size(KEY_BANDS) == TdxDllLoader::DLL_COUNT, "size(KEY_BANDS) must equal DLL_COUNT");

static int tdxdll_function_entry(int dll_no, CALCINFO *pData)
{
  static thread_local TdxDllLoader loader;
  const auto &dll = loader.dlls[dll_no];
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

#define DEFINE_TDXDLL_FUNCTION_ENTRY(dll_no)                                              \
  extern "C" int WINAPI TDXDLL##dll_no(CALCINFO *pData)                                   \
  {                                                                                       \
    static_assert(dll_no > 0 && dll_no <= TdxDllLoader::DLL_COUNT,                        \
                  "dll_no [" #dll_no "] overflow, must in [1, TdxDllLoader::DLL_COUNT]"); \
    return tdxdll_function_entry(dll_no - 1, pData);                                      \
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
    TdxDllLoader::copy_dll = ::GetPrivateProfileInt(TEXT("TDX2FOX"),
                                                    TEXT("DLL_THREAD_SAFE"),
                                                    0,
                                                    dll_ini_path.c_str())? true: false;
    for (size_t i = 0; i < TdxDllLoader::DLL_COUNT; ++i)
    {
      TCHAR tdx_dll_path[MAX_PATH] = {0};
      ::GetPrivateProfileString(TEXT("BAND"),
                                KEY_BANDS[i],
                                TEXT(""),
                                tdx_dll_path,
                                std::size(tdx_dll_path),
                                dll_ini_path.c_str());
      TdxDllLoader::dll_path[i] = tdx_dll_path;
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
