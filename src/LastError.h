#pragma once

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
	static LastError New();
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
	static LastError New(DWORD lastErrorCode);
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
	LastError();
    /**
     * @brief Construct a new Last Error object
     * 
     * The new LastError object is created by submitting an system error code.
     * 
     * @sa https://learn.microsoft.com/en-us/windows/win32/api/errhandlingapi/nf-errhandlingapi-getlasterror
     * 
     * @param lastErrorCode One of the [system error codes](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes)
     */
	LastError(DWORD lastErrorCode);
    /**
     * @brief Destroy the Last Error object
     * 
     */
	~LastError();
	/**
	 * @brief Construct a new LastError object by copying from another
	 * 
	 * @param other The object which gets copied.
	 */
	LastError(const LastError& other);
	/**
	 * @brief Move C'tor semantics
	 * 
	 * @param other The object to move from.
	 */
	LastError(LastError&& other) noexcept;

	/**
	 * @brief Copy assignment operator.
	 * 
	 * @param other The object which gets copied.
	 * @return LastError& The copy.
	 */
    LastError& operator=(const LastError& other);
	/**
	 * @brief Move assignment operator.
	 * 
	 * @param other The object to move from.
	 * @return LastError& The moved object.
	 */
	LastError& operator=(LastError&& other) noexcept;
    /**
     * @brief Returns the null-terminated current error message.
     * 
     * Returns a pointer to an array that contains a null-terminated sequence
     * of characters representing the current error message.
     * 
     * @return LPCWSTR The const pointer to the current error message.
     */
	LPCWSTR c_str() const;
    /**
     * @brief Returns the system error code perceived during creation.
     * 
     * @return DWORD The system [system error code](https://learn.microsoft.com/en-us/windows/win32/debug/system-error-codes)
     */
	DWORD getLastError() const;
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
	void resetLastErrorMessage();
    /**
     * @brief Set `_lastErrorMessage` according to the current `_lastErrorCode`.
     * 
     * If necessary `_lastErrorMessage` is reset (see resetLastErrorMessage())
     * before allocating the new error string for the system error code stored
     * in `_lastErrorCode`.
     * 
     */
	void setLastErrorMessage();
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
