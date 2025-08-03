#include "Machine.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_WIN32)

#include "platform/win32/CCUtils-win32.h"

NS_BEGIN

std::string Machine::getModel() const
{
	std::string model;
	HKEY hKey;

	if (RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DESCRIPTION\\System\\BIOS"), &hKey) == ERROR_SUCCESS)
	{
		TCHAR buf[256] = { 0 };
		DWORD dwBufSize = sizeof(buf);
		std::string manufacturer;
		std::string productName;

		if (RegQueryValueEx(hKey, TEXT("SystemManufacturer"), NULL, NULL, (LPBYTE)buf, &dwBufSize) == ERROR_SUCCESS)
			manufacturer = StringWideCharToUtf8(buf);

		dwBufSize = sizeof(buf);
		if (RegQueryValueEx(hKey, TEXT("SystemProductName"), NULL, NULL, (LPBYTE)buf, &dwBufSize) == ERROR_SUCCESS)
			productName = StringWideCharToUtf8(buf);

		model = StringUtils::format("%s %s", manufacturer.c_str(), productName.c_str());

		RegCloseKey(hKey);
	}

	return model;
}

std::string Machine::getOS() const
{
	std::string os;

	OSVERSIONINFOEX osVerInfo;
	osVerInfo.dwOSVersionInfoSize = sizeof(osVerInfo);
	if(GetVersionEx((OSVERSIONINFO *)&osVerInfo))
		os = cocos2d::StringUtils::format("Windows %d.%d.%d %ls", osVerInfo.dwMajorVersion, osVerInfo.dwMinorVersion, osVerInfo.dwBuildNumber, osVerInfo.szCSDVersion);

	return os;
}

std::string Machine::getDeviceGUID() const
{
	std::wstring key = L"SOFTWARE\\Microsoft\\Cryptography";
	std::wstring name = L"MachineGuid";

	std::string guidString;
	HKEY hKey;
	
	REGSAM samDesired = KEY_READ;
	BOOL f64 = FALSE;
	if (IsWow64Process(GetCurrentProcess(), &f64) && f64)
		samDesired |= KEY_WOW64_64KEY;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, key.c_str(), 0, samDesired, &hKey) == ERROR_SUCCESS)
	{
		DWORD type;
		DWORD cbData;
		if (RegQueryValueEx(hKey, name.c_str(), NULL, &type, NULL, &cbData) == ERROR_SUCCESS)
		{
			if (type == REG_SZ)
			{
				std::wstring value(cbData / sizeof(wchar_t), L'\0');
				if (RegQueryValueEx(hKey, name.c_str(), NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) == ERROR_SUCCESS)
					guidString = StringWideCharToUtf8(value);
			}
		}

		RegCloseKey(hKey);
	}
	else
	{
		GUID guid;
		if (CoCreateGuid(&guid) == S_OK)
		{
			char guidStr[37];
			sprintf_s(
				guidStr,
				"%08lx-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
			guidString = guidStr;
		}
	}

	return guidString;
}

Rect Machine::getSafeAreaRect() const
{
    return Director::getInstance()->getSafeAreaRect();
}

bool Machine::isTablet() const
{
    return false;
}

void Machine::setMultipleTouchEnabled(bool enabled)
{
}

bool Machine::isAppInstalled(std::string const& packageName) const
{
    return false;
}

void Machine::vibrate(VibrationType type)
{
}

NS_END

#endif // CC_TARGET_PLATFORM == CC_PLATFORM_WIN32
