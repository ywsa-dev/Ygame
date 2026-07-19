// Ysetting.cpp - 수정 버전
#define _WIN32_WINNT 0x0600
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    wchar_t exePath[MAX_PATH];
    wchar_t desktopPath[MAX_PATH];
    
    // 1. 현재 실행 중인 exe 파일의 전체 경로 얻기
    if (GetModuleFileNameW(NULL, exePath, MAX_PATH) == 0) {
        MessageBoxW(NULL, L"실행 파일 경로를 얻을 수 없습니다.", L"오류", MB_ICONERROR);
        return 1;
    }

    // 2. exe 파일의 디렉토리 경로 추출
    std::wstring exeDir(exePath);
    size_t lastSlash = exeDir.find_last_of(L'\\');
    if (lastSlash != std::wstring::npos) {
        exeDir = exeDir.substr(0, lastSlash);
    }

    // 3. 바로가기 대상 경로
    std::wstring targetPath = exeDir + L"\\Ygame2\\Ygame2.exe";

    // 4. 아이콘 경로
    std::wstring iconPath = exeDir + L"\\icon\\Y.ico";

    // 5. 바로가기 저장 경로: 바탕화면
    if (SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, desktopPath) != S_OK) {
        MessageBoxW(NULL, L"바탕화면 경로를 얻을 수 없습니다.", L"오류", MB_ICONERROR);
        return 1;
    }
    std::wstring shortcutPath = std::wstring(desktopPath) + L"\\Ygame2.lnk";

    // ===== 대상 파일 존재 확인 =====
    if (GetFileAttributesW(targetPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wstring errMsg = L"대상 파일이 없습니다:\n" + targetPath;
        MessageBoxW(NULL, errMsg.c_str(), L"오류", MB_ICONERROR);
        return 1;
    }

    // ===== 아이콘 파일 존재 확인 =====
    if (GetFileAttributesW(iconPath.c_str()) == INVALID_FILE_ATTRIBUTES) {
        std::wstring errMsg = L"아이콘 파일이 없습니다:\n" + iconPath + L"\n\n실행 파일의 아이콘을 사용합니다.";
        MessageBoxW(NULL, errMsg.c_str(), L"경고", MB_OK | MB_ICONWARNING);
        iconPath = targetPath;  // 아이콘을 실행 파일로 대체
    }

    // 6. COM 초기화
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        MessageBoxW(NULL, L"COM 초기화에 실패했습니다.", L"오류", MB_ICONERROR);
        return 1;
    }

    // 7. ShellLink 객체 생성
    IShellLinkW* pShellLink = NULL;
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                          IID_IShellLinkW, (void**)&pShellLink);

    if (SUCCEEDED(hr)) {
        // 8. 바로가기 속성 설정
        pShellLink->SetPath(targetPath.c_str());
        pShellLink->SetWorkingDirectory(exeDir.c_str());
        pShellLink->SetIconLocation(iconPath.c_str(), 0);

        // 9. IPersistFile 인터페이스로 저장
        IPersistFile* pPersistFile = NULL;
        hr = pShellLink->QueryInterface(IID_IPersistFile, (void**)&pPersistFile);
        
        if (SUCCEEDED(hr)) {
            hr = pPersistFile->Save(shortcutPath.c_str(), TRUE);
            pPersistFile->Release();
            
            if (SUCCEEDED(hr)) {
                MessageBoxW(NULL, L"바탕화면에 Ygame2 바로가기가 생성됐습니다!", L"완료", MB_OK | MB_ICONINFORMATION);
            } else {
                MessageBoxW(NULL, L"바로가기 파일 저장에 실패했습니다.", L"오류", MB_ICONERROR);
            }
        } else {
            MessageBoxW(NULL, L"IPersistFile 인터페이스를 얻을 수 없습니다.", L"오류", MB_ICONERROR);
        }

        pShellLink->Release();
    } else {
        MessageBoxW(NULL, L"ShellLink 객체 생성에 실패했습니다.", L"오류", MB_ICONERROR);
    }

    CoUninitialize();
    return 0;
}