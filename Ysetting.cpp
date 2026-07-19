#pragma comment(lib, "Ole32.lib")
#pragma comment(lib, "Shell32.lib")

#include <windows.h>
#include <shlobj.h>
#include <string>


bool CreateLink(const std::wstring& exePath, const std::wstring& startInPath, const std::wstring& iconPath, const std::wstring& lnkPath) {
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) return false;

    IShellLinkW* psl = nullptr;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&psl);
    
    if (SUCCEEDED(hr)) {
        IPersistFile* ppf = nullptr;
        

        psl->SetPath(exePath.c_str());

        psl->SetWorkingDirectory(startInPath.c_str());

        psl->SetIconLocation(iconPath.c_str(), 0);

        hr = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
        if (SUCCEEDED(hr)) {

            hr = ppf->Save(lnkPath.c_str(), TRUE);
            ppf->Release();
        }
        psl->Release();
    }
    CoUninitialize();
    return SUCCEEDED(hr);
}

int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    wchar_t currentExePath[MAX_PATH];
    GetModuleFileNameW(NULL, currentExePath, MAX_PATH);

    std::wstring exeStr(currentExePath);
    size_t lastSlash = exeStr.rfind(L'\\');
    if (lastSlash == std::wstring::npos) {
        return 0;
    }

    std::wstring baseDir = exeStr.substr(0, lastSlash);


    std::wstring gameExePath = baseDir + L"\\Ygame\\Ygame.exe";
    std::wstring iconPath    = baseDir + L"\\icon\\Y.ico";


    wchar_t desktopPath[MAX_PATH];
    if (SHGetFolderPathW(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, desktopPath) == S_OK) {
        std::wstring lnkPath = std::wstring(desktopPath) + L"\\Ygame.lnk";


        if (CreateLink(gameExePath, baseDir, iconPath, lnkPath)) {
            MessageBoxW(NULL, L"바탕화면에 Ygame 바로가기가 생성됐습니다!", L"완료", MB_OK | MB_ICONINFORMATION);
            return 0;
        }
    }

    MessageBoxW(NULL, L"바로가기 생성에 실패했습니다.", L"오류", MB_OK | MB_ICONERROR);
    return 0;
}