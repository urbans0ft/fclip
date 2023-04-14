class LastError
{
public:
    /**
     * @brief Creates a new LastError object.
     * 
     * The new LastError object is created with the latest error code received
     * by the `GetLastError` Win API function.
     * 
     * @sa https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
     * 
     * @return New LastError object
     */
	static LastError New()
	{
		return New(GetLastError());
	}
    /**
     * @brief Creates a new LastError object.
     * 
     * The new LastError object is created by submitting an system error code.
     * 
     * @sa https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes
     * 
     * @param lastErrorCode On of the [system error codes](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes)
     * @return New LastError object
     */
	static LastError New(DWORD lastErrorCode)
	{
		return LastError{lastErrorCode};
	}
private:
	/// @brief The system error code perceived during creation.
	DWORD _lastErrorCode;
	/// @brief The error string for the system error code.
	LPWSTR _lastErrorMessage;
public:
    /**
     * @brief Construct a new Last Error object.
     * 
     * The new LastError object is created with the latest error code received
     * by the `GetLastError` Win API function.
     * 
     */
	LastError() : LastError{GetLastError()}
	{
		DBGPRINT(L"Default c'tor.");
	}
    /**
     * @brief Construct a new Last Error object
     * 
     * The new LastError object is created by submitting an system error code.
     * 
     * @sa https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
     * 
     * @param lastErrorCode On of the [system error codes](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes)
     */
	LastError(DWORD lastErrorCode) : _lastErrorCode{lastErrorCode}, _lastErrorMessage{nullptr}
	{
		DBGPRINT(L"C'tor(lastErrorCode := %d)", _lastErrorCode);
		setLastErrorMessage();
	}
    /**
     * @brief Destroy the Last Error object
     * 
     */
	~LastError()
	{
		DBGPRINT(L"Destructor");
		resetLastErrorMessage();
	}
	/**
	 * @brief Construct a new LastError object by copying from another
	 * 
	 * @param other The object which gets copied.
	 */
	LastError(const LastError& other) : LastError{other._lastErrorCode}
	{
		DBGPRINT(L"Copy c'tor");
	}
	/**
	 * @brief Move C'tor semantics
	 * 
	 * @param other The object to move from.
	 */
	LastError(LastError&& other) noexcept
		: _lastErrorCode(std::exchange(other._lastErrorCode, 0)),
    	  _lastErrorMessage(std::exchange(other._lastErrorMessage, nullptr))
	{
		DBGPRINT(L"Move c'tor");
	}

	/**
	 * @brief Copy assignment operator.
	 * 
	 * @param other The object which gets copied.
	 * @return LastError& The copy.
	 */
    LastError& operator=(const LastError& other)
    {
		DBGPRINT(L"copy assignment");
        if (this == &other)
            return *this;
 
        _lastErrorCode = other._lastErrorCode;
		setLastErrorMessage();
 
        return *this;
    }
	/**
	 * @brief Move assignment operator.
	 * 
	 * @param other The object to move from.
	 * @return LastError& The moved object.
	 */
	LastError& operator=(LastError&& other) noexcept
    {
		DBGPRINT(L"move assignment");
        std::swap(_lastErrorCode, other._lastErrorCode);
        std::swap(_lastErrorMessage, other._lastErrorMessage);
        return *this;
    }
    /**
     * @brief Returns the null-terminated current error message.
     * 
     * Returns a pointer to an array that contains a null-terminated sequence
     * of characters representing the current error message.
     * 
     * @return LPCWSTR The const pointer to the current error message.
     */
	LPCWSTR c_str() const
	{
		return _lastErrorMessage;
	}
    /**
     * @brief Returns the system error code perceived during creation.
     * 
     * @return DWORD The system [system error code](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes)
     */
	DWORD getLastError() const
	{
		return _lastErrorCode;
	}
private:
    /**
     * @brief Sets `_lastErrorMessage` back to its original state (`nullptr`).
     * 
     * Resets the `_lastErrorMessage` to nullptr and frees its occupied memory
     * if necessary.
     * 
     * @note The method is used to prevent redundant code within the
     *       [Rule of five](https://en.cppreference.com/w/cpp/language/rule_of_three#Rule_of_five).
     * 
     */
	void resetLastErrorMessage()
	{
		if (_lastErrorMessage != nullptr) {
			LocalFree(_lastErrorMessage);
			_lastErrorMessage = nullptr;
		}
	}
    /**
     * @brief Set `_lastErrorMessage` according to the current `_lastErrorCode`.
     * 
     * If necessary `_lastErrorMessage` is reset (see resetLastErrorMessage())
     * before allocating the new error string for the system error code stored
     * in `_lastErrorCode`.
     * 
     */
	void setLastErrorMessage()
	{
		resetLastErrorMessage();
		DWORD msgLength = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			_lastErrorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&_lastErrorMessage,
			0,
			NULL
		);
		// remove cr lf
		_lastErrorMessage[msgLength - 2] = '\0';
	}
    /**
     * @brief Stream operator to use with (UTF-16) output stream.
     * 
     * Stream operator so a LastError object can be used in output streams.
     * The error message gets submitted to the stream which is equivalent
     * to the message received by c_str().
     * 
     * @param os    the (UTF-16) ouptut stream
     * @param error the LastError object to submit to the stream.
     * @return      std::wostream& the altered output stream.
     */
	friend std::wostream& operator<<(std::wostream& os, const LastError& error);
};
std::wostream& operator<<(std::wostream& os, const LastError& error)
{
	os << error._lastErrorMessage;
	return os;
}