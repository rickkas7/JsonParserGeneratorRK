/**
 ******************************************************************************
 * file    spark_wiring_string.h
 * author  Mohit Bhoite
 * version V1.0.0
 * date    13-March-2013
 * brief   Header for spark_wiring_string.c module
 ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.
  ...mostly rewritten by Paul Stoffregen...
  Copyright (c) 2009-10 Hernando Barragan.  All rights reserved.
  Copyright 2011, Paul Stoffregen, paul@pjrc.com

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
 */

#ifndef String_class_h
#define String_class_h
//#ifdef __cplusplus

#include <stdarg.h>
#include "spark_wiring_print.h" // for HEX, DEC ... constants
#include "spark_wiring_printable.h"

// When compiling programs with this class, the following gcc parameters
// dramatically increase performance and memory (RAM) efficiency, typically
// with little or no increase in code size.
//     -felide-constructors
//     -std=c++0x

class __FlashStringHelper;
#define F(X) (X)

// An inherited class for holding the result of a concatenation.  These
// result objects are assumed to be writable by subsequent concatenations.
class StringSumHelper;


/**
 * @brief Wiring String: A class to hold and manipulate a dynamically allocated string.
 */
class String
{
	// use a function pointer to allow for "if (s)" without the
	// complications of an operator bool(). for more information, see:
	// http://www.artima.com/cppsource/safebool.html
	typedef void (String::*StringIfHelperType)() const;
	void StringIfHelper() const {}

public:
	// constructors
	// creates a copy of the initial value.
	// if the initial value is null or invalid, or if memory allocation
	// fails, the string will be marked as invalid (i.e. "if (s)" will
	// be false).

	/**
	 * @brief Construct a String object from a c-string (null-terminated)
	 *
	 * @param cstr The string to copy, optional. If not specified, starts with an empty string
	 */
	String(const char *cstr = "");

	/**
	 * @brief Construct a String object from a pointer and length
	 *
	 * @param cstr Pointer to a bytes, typically ASCII or UTF-8. Does not need to be null-terminated.
	 *
	 * @param length Length in bytes of the string.
	 */
	String(const char *cstr, unsigned int length);

	/**
	 * @brief Construct a String object as a copy of another string.
	 *
	 * @param str The string to copy. Changes made to str in the future won't be reflected in this copy.
	 */
	String(const String &str);

#ifndef DOXYGEN_DO_NOT_DOCUMENT
	String(const __FlashStringHelper *pstr);
#endif

	/**
	 * @brief Construct a String object from any Printable object
	 *
	 * @param printable The Printable object. The toPrint() method will be called on it to print to this
	 * String the textual representation of the object.
	 *
	 * For example, IPAddress is printable, so you can pass an IPAddress to this constructor and this
	 * string will contain a textual representation of the IPAddress (dotted quad).
	 */
        String(const Printable& printable);
	#ifdef __GXX_EXPERIMENTAL_CXX0X__
	String(String &&rval);
	String(StringSumHelper &&rval);
	#endif

	/**
	 * @brief Construct a String containing a single character
	 *
	 * @param c The character to set the String to
	 */
	explicit String(char c);

	/**
	 * @brief Construct a String from a unsigned char (uint8_t) value, expressed as a number
	 *
	 * @param b The value.
	 *
	 * @param base The number base, default is 10 (decimal). Other values include 8 (octal) and 16 (hexadecimal).
	 */
	explicit String(unsigned char b, unsigned char base=10);

	/**
	 * @brief Construct a String from a int (32 bit signed integer) value, expressed as a number
	 *
	 * @param value The value.
	 *
	 * @param base The number base, default is 10 (decimal). Other values include 8 (octal) and 16 (hexadecimal).
	 */
	explicit String(int value, unsigned char base=10);

	/**
	 * @brief Construct a String from a unsigned int (32 bit unsigned integer) value, expressed as a number
	 *
	 * @param value The value.
	 *
	 * @param base The number base, default is 10 (decimal). Other values include 8 (octal) and 16 (hexadecimal).
	 */
	explicit String(unsigned int value, unsigned char base=10);

	/**
	 * @brief Construct a String from a long (32 bit signed integer) value, expressed as a number
	 *
	 * @param value The value.
	 *
	 * @param base The number base, default is 10 (decimal). Other values include 8 (octal) and 16 (hexadecimal).
	 */
	explicit String(long value, unsigned char base=10);

	/**
	 * @brief Construct a String from a unsigned long (32 bit unsigned integer) value, expressed as a number
	 *
	 * @param value The value.
	 *
	 * @param base The number base, default is 10 (decimal). Other values include 8 (octal) and 16 (hexadecimal).
	 */
	explicit String(unsigned long value, unsigned char base=10);

	/**
	 * @brief Construct a String from a float (32 bit single precision floating point) value, expressed as a number
	 *
	 * @param value The value.
	 *
	 * @param decimalPlaces The number of decimal places to show. Default = 6.
	 */
    explicit String(float value, int decimalPlaces=6);

    /**
	 * @brief Construct a String from a double (64 bit double precision floating point) value, expressed as a number
	 *
	 * @param value The value.
	 *
	 * @param decimalPlaces The number of decimal places to show. Default = 6.
	 */
    explicit String(double value, int decimalPlaces=6);

    /**
     * @brief Destructor. Also deletes the underlying dynamically allocated string.
     */
	~String(void);

	// memory management
	// return true on success, false on failure (in which case, the string
	// is left unchanged).  reserve(0), if successful, will validate an
	// invalid string (i.e., "if (s)" will be true afterwards)

	/**
	 * @brief Reserves a buffer of size
	 *
	 * This can improve the efficiency if you know approximately how big your string will be. Otherwise, the string
	 * is made larger in increments, which is much less efficient.
	 *
	 * If, for example you reserve 100 bytes in a new empty string, the length will still be 0 until you append
	 * characters to it. It just will be able to append 100 bytes until it has to expand the internal dynamically
	 * allocated buffer.
	 */
	unsigned char reserve(unsigned int size);

	/**
	 * @brief Returns the length of the string in bytes
	 *
	 * Note that for UTF-8 strings, this is the number of bytes, not characters.
	 */
	inline unsigned int length(void) const {return len;}

	// creates a copy of the assigned value.  if the value is null or
	// invalid, or if the memory allocation fails, the string will be
	// marked as invalid ("if (s)" will be false).

	/**
	 * @brief Assigns this string to have a copy of String rhs
	 *
	 * @param rhs The string to copy from.
	 */
	String & operator = (const String &rhs);

	/**
	 * @brief Assigns this string to have a copy of c-string (null-terminated) cstr
	 *
	 * @param cstr The string to copy from.
	 */
	String & operator = (const char *cstr);

#ifndef DOXYGEN_DO_NOT_DOCUMENT
	String & operator = (const __FlashStringHelper *pstr);
#endif
	#ifdef __GXX_EXPERIMENTAL_CXX0X__
	String & operator = (String &&rval);
	String & operator = (StringSumHelper &&rval);
	#endif

	/**
	 * @brief Returns the contents this String as a c-string (null-terminated)
	 *
	 * See also c_str() which is another way to do this.
	 */
        operator const char*() const { return c_str(); }

	// concatenate (works w/ built-in types)

	// returns true on success, false on failure (in which case, the string
	// is left unchanged).  if the argument is null or invalid, the
	// concatenation is considered unsucessful.

    /**
     * @brief Append (concatenate) a String object to the end of this String, modifying this string in place.
     *
     * @param str The string to copy from. It is not modified.
     *
     * @return true if the append succeeded or false if there was not enough memory or the parameter was invalid.
     */
	unsigned char concat(const String &str);

	/**
     * @brief Append (concatenate) a c-string (null-terminated) to the end of this String, modifying this string in place.
     *
     * @param cstr The string to copy from. It is not modified.
     *
     * @return true if the append succeeded or false if there was not enough memory or the parameter was invalid.
     */
	unsigned char concat(const char *cstr);
#ifndef DOXYGEN_DO_NOT_DOCUMENT
	unsigned char concat(const __FlashStringHelper * str);
#endif
	/**
     * @brief Append (concatenate) a single character to the end of this String, modifying this string in place.
     *
     * @param c The character to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
     */
	unsigned char concat(char c);

	/**
	 * @brief Append (concatenate) the byte value c to the end of this String as a decimal number 0 - 255, modifying this string in place.
	 *
	 * @param c The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(unsigned char c);

	/**
	 * @brief Append (concatenate) the integer value num to the end of this String as a signed decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(int num);

	/**
	 * @brief Append (concatenate) the unsigned integer value num to the end of this String as a unsigned decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(unsigned int num);

	/**
	 * @brief Append (concatenate) the long integer value num to the end of this String as a signed decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(long num);

	/**
	 * @brief Append (concatenate) the unsigned long value num to the end of this String as a unsigned decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(unsigned long num);

	/**
	 * @brief Append (concatenate) the float n to the end of this String as a decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(float num);

	/**
	 * @brief Append (concatenate) the double precision float n to the end of this String as a decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return true if the append succeeded or false if there was not enough memory.
	 */
	unsigned char concat(double num);

	// if there's not enough memory for the concatenated value, the string
	// will be left unchanged (but this isn't signalled in any way)

	/**
     * @brief Appends (concatenate) a String object to the end of this String, modifying this string in place.
     *
     * @param rhs The string to copy from. It is not modified.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
     */
	String & operator += (const String &rhs)	{concat(rhs); return (*this);}

	/**
     * @brief Appends (concatenate) a c-string (null-terminated) to the end of this String, modifying this string in place.
     *
     * @param cstr The string to copy from. It is not modified.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
     */
	String & operator += (const char *cstr)		{concat(cstr); return (*this);}

	/**
     * @brief Appends (concatenate) a single character to the end of this String, modifying this string in place.
     *
     * @param c The character to append.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
     */
	String & operator += (char c)			{concat(c); return (*this);}

	/**
	 * @brief Append (concatenate) the byte value num to the end of this String as a decimal number 0 - 255, modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
	 */
	String & operator += (unsigned char num)		{concat(num); return (*this);}

	/**
	 * @brief Append (concatenate) the integer value num to the end of this String as a signed decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
	 */
	String & operator += (int num)			{concat(num); return (*this);}

	/**
	 * @brief Append (concatenate) the unsigned integer value num to the end of this String as a unsigned decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
	 */
	String & operator += (unsigned int num)		{concat(num); return (*this);}

	/**
	 * @brief Append (concatenate) the long integer value num to the end of this String as a signed decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
	 */
	String & operator += (long num)			{concat(num); return (*this);}

	/**
	 * @brief Append (concatenate) the unsigned long value num to the end of this String as a unsigned decimal number (base 10), modifying this string in place.
	 *
	 * @param num The value to append.
     *
     * @return This string to you can chain operations together. If there was not enough memory
     * or other error occurs, this String will be left unmodified.
	 */
	String & operator += (unsigned long num)	{concat(num); return (*this);}

	/**
	 * @brief Append (concatenate) a String to the end of lhs
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param rhs The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, const String &rhs);

	/**
	 * @brief Append (concatenate) a c-string (null-terminated) to the end of lhs
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param cstr The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, const char *cstr);

	/**
	 * @brief Append (concatenate) the character c the end of lhs a
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param c The character to append
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, char c);

	/**
	 * @brief Append (concatenate) the unsigned char num to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned char num);

	/**
	 * @brief Append (concatenate) the signed int num to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, int num);

	/**
	 * @brief Append (concatenate) the unsigned int num to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned int num);

	/**
	 * @brief Append (concatenate) the long integer num to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, long num);

	/**
	 * @brief Append (concatenate) the unsigned long integer to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, unsigned long num);

	/**
	 * @brief Append (concatenate) the float num to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, float num);

	/**
	 * @brief Append (concatenate) the double precision float num to the end of lhs as a decimal number (base 10)
	 *
	 * @param lhs The string to append to. String lhs is not modified.
	 *
	 * @param num The value to append.
     *
     * @return the combined string
	 */
	friend StringSumHelper & operator + (const StringSumHelper &lhs, double num);

#ifndef DOXYGEN_DO_NOT_DOCUMENT
	// comparison (only works w/ Strings and "strings")
	operator StringIfHelperType() const { return buffer ? &String::StringIfHelper : 0; }
#endif

	/**
	 * @brief Compares this string to another string using strcmp (case-sensitive)
	 *
	 * @param s the string to compare to
	 *
	 * @return < 0 if s is less than this, == 0 is s equals this, or > 0 if s is greater than this
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	int compareTo(const String &s) const;

	/**
	 * @brief Returns true if this string is equal to another string (case-sensitive)
	 *
	 * @param s the string to compare to
	 *
	 * @return true if the other string is equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char equals(const String &s) const;

	/**
	 * @brief Returns true if this string equal to another string (case-sensitive)
	 *
	 * @param cstr the c-string (null-terminated) to compare to
	 *
	 * @return true if the other string is equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char equals(const char *cstr) const;

	/**
	 * @brief Returns true if this string is equal to another string (case-sensitive)
	 *
	 * @param rhs the string to compare to
	 *
	 * @return true if the other string is equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator == (const String &rhs) const {return equals(rhs);}

	/**
	 * @brief Returns true if this string equal to another string (case-sensitive)
	 *
	 * @param cstr the c-string (null-terminated) to compare to
	 *
	 * @return true if the other string is equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator == (const char *cstr) const {return equals(cstr);}


	/**
	 * @brief Returns true if this string is greater than to another string (case-sensitive)
	 *
	 * @param rhs the string to compare to
	 *
	 * @return true if the other string is greater than this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator != (const String &rhs) const {return !equals(rhs);}

	/**
	 * @brief Returns true if this string not equal to another string (case-sensitive)
	 *
	 * @param cstr the c-string (null-terminated) to compare to
	 *
	 * @return true if the other string is not equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator != (const char *cstr) const {return !equals(cstr);}

	/**
	 * @brief Returns true if this string is less than to another string (case-sensitive)
	 *
	 * @param rhs the string to compare to
	 *
	 * @return true if the other string is less than this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator <  (const String &rhs) const;

	/**
	 * @brief Returns true if this string is greater than to another string (case-sensitive)
	 *
	 * @param rhs the string to compare to
	 *
	 * @return true if the other string is greater than this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator >  (const String &rhs) const;

	/**
	 * @brief Returns true if this string is less than or equal to another string (case-sensitive)
	 *
	 * @param rhs the string to compare to
	 *
	 * @return true if the other string is less than or equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator <= (const String &rhs) const;

	/**
	 * @brief Returns true if this string is greater than or equal to another string (case-sensitive)
	 *
	 * @param rhs the string to compare to
	 *
	 * @return true if the other string is greater than or equal to this string.
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char operator >= (const String &rhs) const;

	/**
	 * @brief Returns true if this string equals another string (case-insensitive)
	 *
	 * @param s the string to compare to
	 *
	 * @return true if equal, false if not
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and does not correctly
	 * compare UTF-8 characters.
	 */
	unsigned char equalsIgnoreCase(const String &s) const;

	/**
	 * @brief Returns true if this string starts with prefix (case-sensitive)
	 *
	 * @param prefix the string containing the string to test against
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and may not work properly
	 * with UTF-8 characters.
	 */
	unsigned char startsWith( const String &prefix) const;

	/**
	 * @brief Returns true if this string contains prefix at specified offset (case-sensitive)
	 *
	 * @param prefix the string containing the string to test against
	 *
	 * @param offset the offset to check at (0 = first characters)
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and may not work properly
	 * with UTF-8 characters.
	 */
	unsigned char startsWith(const String &prefix, unsigned int offset) const;

	/**
	 * @brief Returns true if this string ends with suffix (case-sensitive)
	 *
	 * @param suffix the string containing the suffix to test
	 *
	 * Uses the C standard library function strcmp which is case-sensitive and may not work properly
	 * with UTF-8 characters.
	 */
	unsigned char endsWith(const String &suffix) const;

	// character acccess


	/**
	 * @brief Gets the character at offset index
	 *
	 * @param index The index to set (0 = first character)
	 *
	 * @return The character is 0 if the index is larger than the length of the string.
	 */
	char charAt(unsigned int index) const;

	/**
	 * @brief Set the character at offset index.
	 *
	 * @param index The index to set (0 = first character)
	 *
	 * @param c The value to set the character to.
	 *
	 * If index is greater than the length of the string, nothing is done. In other words, you cannot use this
	 * to append to the string, only modify an existing character.
	 */
	void setCharAt(unsigned int index, char c);

	/**
	 * @brief Gets the character at offset index
	 *
	 * @param index The index to set (0 = first character)
	 *
	 * @return The character is 0 if the index is larger than the length of the string.
	 */
	char operator [] (unsigned int index) const;

	/**
	 * @brief Set the character at offset index.
	 *
	 * @param index The index to set (0 = first character)
	 *
	 * @return A reference to set.
	 *
	 * If index is greater than the length of the string, a dummy reference is returned instead. This allows
	 * operation to execute without error, but also discards the change. In other words, you cannot use this
	 * to append to the string, only modify an existing character.
	 */
	char& operator [] (unsigned int index);

	/**
	 * @brief Copy the data out of this String into another buffer
	 *
	 * @param buf The buffer to copy into
	 *
	 * @param bufsize The size of the buffer. The buffer will contain a null-terminted string so the maximum
	 * string length is bufsize - 1.
	 *
	 * @param index The index to start copying from (0 = first character). Optional. Default is from 0, the start
	 * of the string.
	 *
	 * If bufsize is smaller than the string the string will be truncated and still null-terminated. If the string
	 * is truncated and UTF-8, it may break a multi-byte character sequence in the middle, resulting in invalid UTF-8.
	 */
	void getBytes(unsigned char *buf, unsigned int bufsize, unsigned int index=0) const;

	/**
	 * @brief Copy the data out of this String into another buffer
	 *
	 * @param buf The buffer to copy into
	 *
	 * @param bufsize The size of the buffer. The buffer will contain a null-terminted string so the maximum
	 * string length is bufsize - 1.
	 *
	 * @param index The index to start copying from (0 = first character). Optional. Default is from 0, the start
	 * of the string.
	 *
	 * If bufsize is smaller than the string the string will be truncated and still null-terminated. If the string
	 * is truncated and UTF-8, it may break a multi-byte character sequence in the middle, resulting in invalid UTF-8.
	 */
	void toCharArray(char *buf, unsigned int bufsize, unsigned int index=0) const
		{getBytes((unsigned char *)buf, bufsize, index);}

	/**
	 * @brief Returns a c-string (null-terminated)
	 *
	 * This allows the String object to be passed to anything that requires a c-string. See also
	 * operator const char *.
	 *
	 * One place where you need to explicitly use c_str() or cast is when passing a String
	 * as a variable argument to sprintf:
	 *
	 * ```
	 * String str;
	 * snprintf(buf, sizeof(buf), "string=%s", str.c_str());
	 * ```
	 *
	 * If you leave off the c_str() the value won't be printed as string. This also applies to
	 * things that use sprintf internally, like Log:
	 *
	 * ```
	 * Log.info("string=%s", str.c_str());
	 * ```
	 *
	 * This method returns a pointer to the internal buffer. If the underlying string is reallocated because
	 * the string is appended to, this pointer will be invalid.
	 */
	const char * c_str() const { return buffer; }

	// search

	/**
	 * @brief Search this string for a given character
	 *
	 * @param ch The ASCII character to search for
	 *
	 * @return index of the character or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strchr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings.
	 */
	int indexOf( char ch ) const;

	/**
	 * @brief Search this string for a given character starting at an offset
	 *
	 * @param ch The ASCII character to t search for
	 *
	 * @param fromIndex The index to start from (0 = first character)
	 *
	 * @return index of the character or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strchr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings.
	 */
	int indexOf( char ch, unsigned int fromIndex ) const;

	/**
	 * @brief Search this string for a given String
	 *
	 * @param str The string to search for
	 *
	 * @return index of the string or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strstr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings. It is case-sensitive.
	 */
	int indexOf( const String &str ) const;

	/**
	 * @brief Search this string for a given String starting at an offset.
	 *
	 * @param str The string to search for
	 *
	 * @param fromIndex The index to start from (0 = first character)
	 *
	 * @return index of the string or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strstr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings. It is case-sensitive.
	 */
	int indexOf( const String &str, unsigned int fromIndex ) const;

	/**
	 * @brief Search this string for a given character, starting at the end
	 *
	 * @param ch The ASCII character to search for
	 *
	 * @return index of the character or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strrchr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings.
	 */
	int lastIndexOf( char ch ) const;

	/**
	 * @brief Search this string for a given character, starting at the fromIndex and going toward the beginning
	 *
	 * @param ch The ASCII character to search for
	 *
	 * @param fromIndex The index to start from (0 = first character)
	 *
	 * @return index of the character or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strrchr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings.
	 */
	int lastIndexOf( char ch, unsigned int fromIndex ) const;

	/**
	 * @brief Search this string for a last occurrence of str
	 *
	 * @param str The string to search for
	 *
	 * @return index of the start of the string or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strstr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings. It is case-sensitive.
	 */
	int lastIndexOf( const String &str ) const;


	/**
	 * @brief Search this string for a last occurrence of str starting at fromIndex
	 *
	 * @param str The string to search for
	 *
	 * @param fromIndex The index to start from (0 = first character)
	 *
	 * @return index of the start of the string or -1 if not found. 0 = the first character.
	 *
	 * This uses the C standard library function strstr and is only compatible with ASCII characters.
	 * It can return invalid results for UTF-8 strings. It is case-sensitive.
	 */
	int lastIndexOf( const String &str, unsigned int fromIndex ) const;


	/**
	 * @brief Returns a String object with a copy of the characters starting at beginIndex through the end of the string.
	 *
	 * @param beginIndex The index to start copying from, inclusive (0 = first byte, 1 = second byte, ...)
	 *
	 * @return A copy of the specified substring
	 *
	 * Note: If the String contains UTF-8 characters, beginIndex and endIndex are in bytes, not characters!
	 * It does not prevent splitting a UTF-8 multi-byte sequence.
	 */
	String substring( unsigned int beginIndex ) const;

	/**
	 * @brief Returns a String object with a copy of the characters in the specified range
	 *
	 * @param beginIndex The index to start copying from, inclusive (0 = first byte, 1 = second byte, ...)
	 *
	 * @param endIndex The index to stop at, exclusive. The last character copied is the one before this one.
	 *
	 * @return A copy of the specified substring
	 *
	 * Note: If the String contains UTF-8 characters, beginIndex and endIndex are in bytes, not characters!
	 * It does not prevent splitting a UTF-8 multi-byte sequence.
	 */
	String substring( unsigned int beginIndex, unsigned int endIndex ) const;

	// modification

	/**
	 * @brief Replaces every occurrence of a character in the string with another character, modifying it in place
	 *
	 * @param find the character to look for
	 *
	 * @param replace the character to  replace it with
	 *
	 * @return this String, so you can chain multiple operations
	 */
	String& replace(char find, char replace);

	/**
	 * @brief Replaces every occurrence of a String with another String, modifying it in place
	 *
	 * @param find the string to look for (case-sensitive)
	 *
	 * @param replace the string to replace it with
	 *
	 * @return this String, so you can chain multiple operations
	 */
	String& replace(const String& find, const String& replace);

	/**
	 * @brief Removes characters from the String, modifying it in place
	 *
	 * @param index Index to start removing from, inclusive. 0 = first character of the string
	 * through the end of the string.
	 *
	 * @return this String, so you can chain multiple operations
	 */
	String& remove(unsigned int index);

	/**
	 * @brief Removes characters from the String, modifying it in place
	 *
	 * @param index Index to start removing from, inclusive. 0 = first character of the string.
	 *
	 * @param count Number of characters to remove. Typically 1 (remove one character) or more.
	 * Removes to the end of the string if count is larger than the size of the string.
	 *
	 * @return this String, so you can chain multiple operations
	 *
	 */
	String& remove(unsigned int index, unsigned int count);

	/**
	 * @brief Converts this String to lower case, modifying it in place
	 *
	 * @return this String, so you can chain multiple operations
	 *
	 * This is done using the C standard library function tolower() on each character. It only works
	 * with 7-bit ASCII characters and will corrupt UTF-8 data.
	 */
	String& toLowerCase(void);

	/**
	 * @brief Converts this String to upper case, modifying it in place
	 *
	 * @return this String, so you can chain multiple operations
	 *
	 * This is done using the C standard library function toupper() on each character. It only works
	 * with 7-bit ASCII characters and will corrupt UTF-8 data.
	 */
	String& toUpperCase(void);

	/**
	 * @brief Removes leading an trailing white spaces from this string, modifying it in place
	 *
	 * @return this String, so you can chain multiple operations
	 *
	 * Whitespace is determined by the C standard library function isspace().
	 */
	String& trim(void);

	// parsing/conversion
	/**
	 * @brief Converts this string to a signed integer (32-bit)
	 *
	 * @return An integer value or 0 if a parsing error occurs (not an integer).
	 */
	long toInt(void) const;

	/**
	 * @brief Converts this string to a float (single precision floating point value)
	 *
	 * @return a float value or 0.0 if a parsing error occurs (not a float).
	 */
	float toFloat(void) const;

	/**
	 * @brief Uses sprintf-style formatting to build a String object [static]
	 *
	 * @param format The formatting string
	 *
	 * @param ... Variable arguments corresponding to the formatting string
	 *
	 * @return Returns a String object formatted as specified
	 */
    static String format(const char* format, ...);

protected:
	char *buffer;	        //!< The buffer containing the data. It is always null-terminated.
	unsigned int capacity;  //!< The capacity of the buffer. The longest string is one byte less than this.
	unsigned int len;       //!< The String length (not counting the null terminator).
	unsigned char flags;    //!< Unused, for future features
protected:
#ifndef DOXYGEN_DO_NOT_DOCUMENT
	void init(void);
	void invalidate(void);
	unsigned char changeBuffer(unsigned int maxStrLen);
	unsigned char concat(const char *cstr, unsigned int length);

	// copy and move
	String & copy(const char *cstr, unsigned int length);
	String & copy(const __FlashStringHelper *pstr, unsigned int length);
#endif

	#ifdef __GXX_EXPERIMENTAL_CXX0X__
	void move(String &rhs);
	#endif

        friend class StringPrintableHelper;

};

/**
 * @brief Class used when appending mutiple String and other values using +
 */
class StringSumHelper : public String
{
public:
	/**
	 * @brief Append a String object
	 *
	 * @param s The string to append.
	 *
	 * @return StringSumHelper object that encapsulates a copy of that string for appending to another string.
	 */
	StringSumHelper(const String &s) : String(s) {}

	/**
	 * @brief Append a const char * (c-string, null terminated)
	 *
	 * @param p The string to append.
	 *
	 * @return StringSumHelper object that encapsulates a copy of that string for appending to another string.
	 */
	StringSumHelper(const char *p) : String(p) {}

	/**
	 * @brief Append a single character
	 *
	 * @param c The character to append.
	 *
	 * @return StringSumHelper object that encapsulates a copy of that character for appending to another string.
	 */
	StringSumHelper(char c) : String(c) {}

	/**
	 * @brief Append a byte as a decimal number 0 - 255.
	 *
	 * @param num The byte value to append.
	 *
	 * @return StringSumHelper object that encapsulates the textual representation of the number for appending to another string.
	 */
	StringSumHelper(unsigned char num) : String(num) {}

	/**
	 * @brief Append a 32-bit signed integer as a decimal number.
	 *
	 * @param num The byte value to append.
	 *
	 * @return StringSumHelper object that encapsulates the textual representation of the number for appending to another string.
	 */
	StringSumHelper(int num) : String(num) {}

	/**
	 * @brief Append a 32-bit unsigned integer as a decimal number.
	 *
	 * @param num The byte value to append.
	 *
	 * @return StringSumHelper object that encapsulates the textual representation of the number for appending to another string.
	 */
	StringSumHelper(unsigned int num) : String(num) {}

	/**
	 * @brief Append a 32-bit long integer as a decimal number.
	 *
	 * @param num The byte value to append.
	 *
	 * @return StringSumHelper object that encapsulates the textual representation of the number for appending to another string.
	 */
	StringSumHelper(long num) : String(num) {}

	/**
	 * @brief Append a 32-bit unsigned long as a decimal number.
	 *
	 * @param num The byte value to append.
	 *
	 * @return StringSumHelper object that encapsulates the textual representation of the number for appending to another string.
	 */
	StringSumHelper(unsigned long num) : String(num) {}
};

#ifndef DOXYGEN_DO_NOT_DOCUMENT
#include <ostream>
std::ostream& operator << ( std::ostream& os, const String& value );
#endif


//#endif  // __cplusplus
#endif  // String_class_h
