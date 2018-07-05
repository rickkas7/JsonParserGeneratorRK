/**
 ******************************************************************************
 * @file    spark_wiring_print.h
 * @author  Mohit Bhoite
 * @version V1.0.0
 * @date    13-March-2013
 * @brief   Header for spark_wiring_print.c module
 ******************************************************************************
  Copyright (c) 2013-2015 Particle Industries, Inc.  All rights reserved.
  Copyright (c) 2010 David A. Mellis.  All right reserved.

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

#ifndef __SPARK_WIRING_PRINTx_
#define __SPARK_WIRING_PRINT_

#include <stddef.h>
#include <string.h>
#include <stdint.h> // for uint8_t

#include "spark_wiring_string.h"
#include "spark_wiring_printable.h"

const unsigned char DEC = 10; //!< Decimal number format (0-9)
const unsigned char HEX = 16; //!< Hexadecimal number format (0-9, a-f)
const unsigned char OCT = 8;  //!< Octal number format (0-7)
const unsigned char BIN = 2;  //!< Binary number format (0-1)

class String;
class __FlashStringHelper;

/**
 * @brief Class for printing to a stream or file
 *
 * Various classes include serial, TCP network streams, and files inherit from this and can use these methods.
 */
class Print
{
  private:
    int write_error;
    size_t printNumber(unsigned long, uint8_t);
    size_t printFloat(double, uint8_t);
  protected:
#ifndef DOXYGEN_DO_NOT_DOCUMENT
    void setWriteError(int err = 1) { write_error = err; }
    size_t printf_impl(bool newline, const char* format, ...);
#endif

  public:
#ifndef DOXYGEN_DO_NOT_DOCUMENT
    Print() : write_error(0) {}

    virtual ~Print() {}
#endif
    /**
     * @brief Return the last error code. 0 means no error.
     */
    int getWriteError() { return write_error; }

    /**
     * @brief Clear the last error code to 0.
     */
    void clearWriteError() { setWriteError(0); }

    /**
     * @brief Write a single byte to the stream or file.
     *
     * @param c The byte to write. All values 0 - 255 are allowed.
     */
    virtual size_t write(uint8_t c) = 0;

    /**
     * @brief Write a null-terminated c-string the stream or file.
     *
     * @param str point to a null-terminated c-string.
     */
    size_t write(const char *str) {
      if (str == NULL) return 0;
      return write((const uint8_t *)str, strlen(str));
    }

    /**
     * @brief Write a bytes specified by a buffer and length to the stream or file
     *
     * @param buffer pointer to the buffer. The data does not need to be null-terminated.
     * @param size size in bytes
     */
    virtual size_t write(const uint8_t *buffer, size_t size);

    /**
     * @brief Print a null-terminated array of char variables (a c-string) to the stream or file
     */
    size_t print(const char[]);

    /**
     * @brief Print a single character to the stream or file
     */
    size_t print(char);

    /**
     * @brief Print an unsigned char (byte value, 8 bits) in the specified base to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t print(unsigned char value, int base = DEC);

    /**
     * @brief Print an int (32 bit integer) the specified base to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t print(int value, int base = DEC);

    /**
     * @brief Print an unsigned int (32 bit unsigned integer) the specified base to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t print(unsigned int value, int base = DEC);

    /**
     * @brief Print a long (32 bit integer) the specified base to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t print(long value, int base = DEC);

    /**
     * @brief Print a unsigned long (32 bit unsigned integer) the specified base to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t print(unsigned long value, int base = DEC);

    /**
     * @brief Print a double floating point value to the stream or file.
     *
     * @param value The value to print.
     * @param dec The number of decimal places to include for the fractional part. Default: 2
     */
    size_t print(double value, int dec = 2);

    /**
     * @brief Print an object derived from Printable to the stream or file.
     */
    size_t print(const Printable&);

#ifndef DOXYGEN_DO_NOT_DOCUMENT
    size_t print(const __FlashStringHelper*);
#endif

    /**
     * @brief Print a null-terminated array of char variables (a c-string) plus a CRLF end-of-line terminator to the stream or file.
     */
    size_t println(const char[]);

    /**
     * @brief Print a single character plus a CRLF end-of-line terminator to the stream or file.
     */
    size_t println(char value);

    /**
     * @brief Print an unsigned char (byte value. 8 bits) in the specified base plus a CRLF end-of-line terminator to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t println(unsigned char value, int base = DEC);
    /**
     * @brief Print an int (32 bit integer) the specified base to plus a CRLF end-of-line terminator the stream or file.
     *
     * @param value The value to print
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t println(int value, int base = DEC);

    /**
     * @brief Print an unsigned int (32 bit unsigned integer) the specified base plus a CRLF end-of-line terminator to the stream or file.
     *
     * @param value The value to print
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t println(unsigned int value, int base = DEC);
    /**
     * @brief Print a long (32 bit signed integer) the specified base plus a CRLF end-of-line terminator to the stream or file.
     *
     * @param value The value to print
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t println(long value, int base = DEC);
    /**
     * @brief Print a unsigned long (32 bit unsigned integer) the specified base plus a CRLF end-of-line terminator to the stream or file.
     *
     * @param value The value to print.
     * @param base The base to print. Default is DEC (decimal). Other values are HEX (hexadecimal),
     * OCT (octal), and BIN (binary).
     */
    size_t println(unsigned long value, int base = DEC);

    /**
     * @brief Print a double floating point value plus a CRLF end-of-line terminator to the stream or file.
     *
     * @param value The value to print.
     * @param dec The number of decimal places to include for the fractional part. Default: 2
     */
    size_t println(double value, int dec = 2);
    /**
     * @brief Print an object derived from Printable plus a CRLF end-of-line terminator to the stream or file
     */
    size_t println(const Printable&);

    /**
     * @brief Print a CRLF end-of-line terminator to the stream or file
     */
    size_t println(void);

#ifndef DOXYGEN_DO_NOT_DOCUMENT
    size_t println(const __FlashStringHelper*);
#endif

    /**
     * @brief Print using printf-style formatting to the stream or file
     *
     * @param format printf-style formatting string
     *
     * @param args variable arguments
     */
    template <typename... Args>
    inline size_t printf(const char* format, Args... args)
    {
        return this->printf_impl(false, format, args...);
    }

    /**
     * @brief Print using printf-style formatting plus a CRLF end-of-line terminator to the stream or file
     *
     * @param format printf-style formatting string
     *
     * @param args variable arguments
     */
    template <typename... Args>
    inline size_t printlnf(const char* format, Args... args)
    {
        return this->printf_impl(true, format, args...);
    }

};

#endif
