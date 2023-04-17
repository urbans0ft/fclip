#include "pch.h"

LastError LastError::New()
{
    return New(GetLastError());
}

LastError LastError::New(DWORD lastErrorCode)
{
    return LastError{lastErrorCode};
}

LastError::LastError() : LastError{GetLastError()}
{
    DBGPRINT(L"Default c'tor.");
}

LastError::LastError(DWORD lastErrorCode) : _lastErrorCode{lastErrorCode}, _lastErrorMessage{nullptr}
{
    DBGPRINT(L"C'tor(lastErrorCode := %d)", _lastErrorCode);
    setLastErrorMessage();
}

LastError::~LastError()
{
    DBGPRINT(L"Destructor");
    resetLastErrorMessage();
}

LastError::LastError(const LastError& other) : LastError{other._lastErrorCode}
{
    DBGPRINT(L"Copy c'tor");
}

LastError::LastError(LastError&& other) noexcept
    : _lastErrorCode(std::exchange(other._lastErrorCode, 0)),
        _lastErrorMessage(std::exchange(other._lastErrorMessage, nullptr))
{
    DBGPRINT(L"Move c'tor");
}

LastError& LastError::operator=(const LastError& other)
{
    DBGPRINT(L"copy assignment");
    if (this == &other)
        return *this;

    _lastErrorCode = other._lastErrorCode;
    setLastErrorMessage();

    return *this;
}

LastError& LastError::operator=(LastError&& other) noexcept
{
    DBGPRINT(L"move assignment");
    std::swap(_lastErrorCode, other._lastErrorCode);
    std::swap(_lastErrorMessage, other._lastErrorMessage);
    return *this;
}

LPCWSTR LastError::c_str() const
{
    return _lastErrorMessage;
}

DWORD LastError::getLastError() const
{
    return _lastErrorCode;
}

void LastError::resetLastErrorMessage()
{
    if (_lastErrorMessage != nullptr) {
        LocalFree(_lastErrorMessage);
        _lastErrorMessage = nullptr;
    }
}

void LastError::setLastErrorMessage()
{
    resetLastErrorMessage();
    DWORD msgLength = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        _lastErrorCode,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        (LPWSTR)&_lastErrorMessage,
        0,
        NULL
    );
    // remove cr lf
    _lastErrorMessage[msgLength - 2] = '\0';
}

std::wostream& operator<<(std::wostream& os, const LastError& error)
{
	os << error._lastErrorMessage;
	return os;
}