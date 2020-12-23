#ifndef __JSONPARSERGENERATORRK_H
#define __JSONPARSERGENERATORRK_H


#include "Particle.h"

#include <vector>

// You can mostly ignore the stuff in this namespace block. It's part of the jsmn library
// that's used internally and you can mostly ignore. The actual API is the JsonParser C++ object
// below.
namespace JsonParserGeneratorRK {
	// begin jsmn.h
	// https://github.com/zserge/jsmn

	/**
	 * @brief JSON type identifier (object, array, string, primitive)
	 */
	typedef enum {
		JSMN_UNDEFINED = 0, //!< undefined JSON type
		JSMN_OBJECT = 1, 	//!< JSON object
		JSMN_ARRAY = 2,		//!< JSON array
		JSMN_STRING = 3,	//!< JSON string
		JSMN_PRIMITIVE = 4	//!< JSON primitive (number, true, false, or null)
	} jsmntype_t;

	/**
	 * @brief JSMN error codes
	 */
	enum jsmnerr {
		JSMN_ERROR_NOMEM = -1,	//!< Not enough tokens were provided
		JSMN_ERROR_INVAL = -2,	//!< Invalid character inside JSON string
		JSMN_ERROR_PART = -3	//!< The string is not a full JSON packet, more bytes expected
	};

	/**
	 * @brief JSON token description.
	 */
	typedef struct {
		jsmntype_t type;	//!< type (object, array, string etc.)
		int start;			//!< start position in JSON data string
		int end;			//!< end position in JSON data string
		int size;			//!< size
	#ifdef JSMN_PARENT_LINKS
		int parent;			//!< parent object
	#endif
	} jsmntok_t;

	/**
	 * @brief JSON parser
	 *
	 * Contains an array of token blocks available. Also stores
	 * the string being parsed now and current position in that string.
	 */
	typedef struct {
		unsigned int pos; 		//!< offset in the JSON string
		unsigned int toknext;	//!< next token to allocate
		int toksuper; 			//!< superior token node, e.g parent object or array
	} jsmn_parser;

	/**
	 * @brief Create JSON parser over an array of tokens
	 */
	void jsmn_init(jsmn_parser *parser);

	/**
	 * @brief Run JSON parser.
	 *
	 * It parses a JSON data string into and array of tokens, each describing
	 * a single JSON object.
	 */
	int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
			jsmntok_t *tokens, unsigned int num_tokens);

	// end jsmn.h
}

/**
 * @brief Class used internally for writing to strings.
 *
 * This is a wrapper around either
 * String (the Wiring version) or a buffer and length. This allows writing to a static buffer
 * with no dynamic memory allocation at all.
 *
 * One of the things about String is that while you can pre-allocate reserve space for data,
 * you can't get access to the internal length field, so you can't write to raw bytes then resize
 * it to the correct size. This wrapper is that allows appending to either a String or buffer
 * to get around this limitation of String.
 *
 * You can also use it for sizing only by passing NULL for buf.
 */
class JsonParserString {
public:
	/**
	 * @brief Construct a JsonParserString wrapping a Wiring String
	 *
	 * @param str A pointer Wiring String object to write to.
	 */
	JsonParserString(String *str);

	/**
	 * @brief Construct a JsonParserString wrapping a buffer and length
	 *
	 * @param buf A pointer to a buffer
	 *
	 * @param bufLen The length of the buffer in bytes
	 */
	JsonParserString(char *buf, size_t bufLen);

	/**
	 * @brief Append a single char to the underlying string
	 *
	 * @param ch The char to append.
	 */
	void append(char ch);

	/**
	 * @brief Append a buffer and length to the underlying string
	 *
	 * @param str A pointer to the character to add. Does not need to be null-terminated.
	 *
	 * @param len Length of the string to append in bytes.
	 */
	void append(const char *str, size_t len);

	/**
	 * @brief Get the length of the string.
	 *
	 * @return The string length in bytes. If the string contains UTF-8 characters, it will be the number
	 * of bytes, not characters.
	 *
	 * For buffer and bufLenb, the maximum string length will be bufLen - 1 to leave room for the null terminator.
	 */
	size_t getLength() const { return length; }

protected:
	String *str;	//!< When writing to a String, the String object.
	char *buf;		//!< When writing to a buffer, the pointer to the buffer. Not used for String.
	size_t bufLen;	//!< When writing to a buffer, the length of the buffer in bytes. Not used for String.
	size_t length;	//!< The current offset being written to.
};

/**
 * @brief Base class for managing a static or dynamic buffer, used by both JsonParser and JsonWriter
 */
class JsonBuffer {
public:
	/**
	 * @brief Construct a JsonBuffer object with no external buffer specified
	 */
	JsonBuffer();

	/**
	 * @brief Destructor. Destroying the object does not delete any underlying buffer!
	 */
	virtual ~JsonBuffer();

	/**
	 * @brief Construct a JsonBuffer with an external buffer of a given size
	 *
	 * @param buffer Pointer to the buffer
	 *
	 * @param bufferLen The length of the buffer
	 *
	 * This buffer will not be deleted when the object is destructed.
	 */
	JsonBuffer(char *buffer, size_t bufferLen);

	/**
	 * @brief Sets the buffers to the specified buffer and length
	 *
	 * @param buffer Pointer to the buffer
	 *
	 * @param bufferLen The length of the buffer
	 *
	 * This buffer will not be deleted when the object is destructed.
	 */
	void setBuffer(char *buffer, size_t bufferLen);

	/**
	 * @brief Allocate the buffer using malloc/realloc
	 *
	 * @param len The length of the buffer in bytes
	 *
	 * @returns true if the allocation/reallocation was successful or false if there was not enough free memory.
	 *
	 * There's also a version that takes a pointer and length to use a static buffer instead of a dynamically
	 * allocated one.
	 */
	bool allocate(size_t len);

	/**
	 * @brief Add a c-string to the end of the buffer
	 *
	 * @param data Pointer to a c-string (null terminated).
	 */
	bool addString(const char *data) { return addData(data, strlen(data)); }

	/**
	 * @brief Add a string to the end of the buffer
	 *
	 * @param data Pointer to the string bytes. Does not need to be null-terminated
	 *
	 * @param dataLen Length of the data in bytes. For UTF-8, this is the number of bytes, not characters!
	 */
	bool addData(const char *data, size_t dataLen);

	/**
	 * @brief Add chunked multipart data, typically from an event subscription handler
	 * 
	 * @param event The event name. Used to find the multipart segment number at the end
	 * 
	 * @param data The event data (c-string)
	 * 
	 * @param chunkSize The size of all chunks except the last one (default: 512 bytes)
	 * 
	 * This method makes several assumptions that are specific to the way chunked webhook
	 * hook-response events are returned:
	 * 
	 * - All chunks (except possibly the last) are chunkSize bytes. This is always 512, and that's 
	 * the default value of the chunkSize parameter.
	 * - Chunks may arrive out-of-order.
	 * - Event names end in /0 for the first chunk, /1 for the second, ... with the number being
	 * a decimal number in ASCII, limited only by available RAM.
	 */
	bool addChunkedData(const char *event, const char *data, size_t chunkSize = 512);
	
	/**
	 * @brief Gets a pointer to the internal buffer
	 *
	 * Note: The internal buffer is not null-terminated!
	 */
	char *getBuffer() const { return buffer; }

	/**
	 * @brief Gets the current offset for writing
	 */
	size_t getOffset() const { return offset; }

	/**
	 * @brief swets the current offset for writing
	 */
	void setOffset(size_t offset) { this->offset = offset; };

	/**
	 * @brief Gets the current length of the buffer
	 *
	 * The buffer length is either the bufferLen passed to the constructor that takes a buffer and bufferLen
	 * or the length allocated using allocate(len).
	 */
	size_t getBufferLen() const { return bufferLen; }

	/**
	 * @brief Clears the current buffer for writing.
	 *
	 * This sets the offset to 0 and also zeroes all of the bytes.
	 */
	void clear();

	/**
	 * @brief Null terminates the buffer
	 */
	void nullTerminate();

protected:
	char	*buffer; //!< The buffer to to read from or write to. This is not null-terminated.
	size_t	bufferLen; //!< The length of the buffer in bytes,
	size_t	offset; //!< The read or write offset.
	bool 	staticBuffers; //!< True if the buffers were passed in and should not freed or reallocated.

};

class JsonReference;


/**
 * @brief API to the JsonParser
 *
 * This is a memory-efficient JSON parser based on jsmn. It only keeps one copy of the data in raw format
 * and an array of tokens. You make calls to read values out.
 */
class JsonParser : public JsonBuffer {
public:
	/**
	 * @brief Construct a parser object
	 *
	 * This version dynamically allocates the buffer and token storage. If you want to minimize
	 * memory allocations you can pass in a static buffer and array of tokens to use instead.
	 */
	JsonParser();

	/**
	 * @brief Destroy a parser object
	 *
	 * If the buffer was allocated dynamically it will be deleted. If you passed in a static buffer
	 * the static buffer is not deleted.
	 */
	virtual ~JsonParser();

	/**
	 * @brief Static buffers constructor
	 */
	JsonParser(char *buffer, size_t bufferLen, JsonParserGeneratorRK::jsmntok_t *tokens, size_t maxTokens);

	/**
	 * @brief Preallocates a specific number of tokens
	 *
	 * Optional: You should set this larger than the expected number
	 * of tokens for efficiency, but if you are not using the static allocator it will resize the
	 * token storage space if it's too small.
	 */
	bool allocateTokens(size_t maxTokens);

	/**
	 * @brief Parses the data you have added using addData() or addString().
	 *
	 * When parsing data split into multiple chunks as a webhook response you can call addString()
	 * in your webhook subscription handler and call parse after each chunk. Only on the last chunk
	 * will parse return true, and you'll know the entire reponse has been received.
	 */
	bool parse();

	/**
	 * @brief Get a JsonReference object. This is used for fluent-style access to the data.
	 */
	JsonReference getReference() const;

	/**
	 * @brief Gets the outer JSON object token
	 *
	 * Typically JSON will contain an object that contains values and possibly other objects.
	 * This method gets the token for the outer object.
	 *
	 * A token (JsonParserGeneratorRK::jsmntok_t) identifies a particular piece of data in the JSON
	 * data, such as an object, array, or element within an object or array, such as a string, integer,
	 * boolean, etc..
	 */
	const JsonParserGeneratorRK::jsmntok_t *getOuterObject() const;

	/**
	 * @brief Gets the outer JSON array token
	 *
	 * Sometimes the JSON will contain an array of values (or objects) instead of starting with
	 * an object. This gets the outermost array.
	 *
	 * A token (JsonParserGeneratorRK::jsmntok_t) identifies a particular piece of data in the JSON
	 * data, such as an object, array, or element within an object or array, such as a string, integer,
	 * boolean, etc..
	 */
	const JsonParserGeneratorRK::jsmntok_t *getOuterArray() const;

	/**
	 * @brief Gets the outer JSON object or array token
	 *
	 * A token (JsonParserGeneratorRK::jsmntok_t) identifies a particular piece of data in the JSON
	 * data, such as an object, array, or element within an object or array, such as a string, integer,
	 * boolean, etc..
	 */
	const JsonParserGeneratorRK::jsmntok_t *getOuterToken() const;

	/**
	 * @brief Given a token for an JSON array in arrayContainer, gets the number of elements in the array.
	 *
	 * 0 = no elements, 1 = one element, ...
	 *
	 * The index values for getValueByIndex(), etc. are 0-based, so the last index you pass in is
	 * less than getArraySize().
	 */
	size_t getArraySize(const JsonParserGeneratorRK::jsmntok_t *arrayContainer) const;

	/**
	 * @brief Given an object token in container, gets the value with the specified key name.
	 *
	 * @param container The token for the object to obtain the data from.
	 *
	 * @param name The name of the key to retrieve
	 *
	 * @param result The returned data. The value can be of type: bool, int, unsigned long, float, double, String,
	 * or (char *, size_t&).
	 *
	 * @result true if the data was retrieved successfully, false if not (key not present or incompatible data type).
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within an object, use getValueTokenByKey() instead.
	 */
	template<class T>
	bool getValueByKey(const JsonParserGeneratorRK::jsmntok_t *container, const char *name, T &result) const {
		const JsonParserGeneratorRK::jsmntok_t *value;

		if (getValueTokenByKey(container, name, value)) {
			return getTokenValue(value, result);
		}
		else {
			return false;
		}
	}

	/**
	 * @brief Gets the value with the specified key name out of the outer object.
	 *
	 * @param name The name of the key to retrieve
	 *
	 * @param result The returned data.
	 *
	 * @result true if the data was retrieved successfully, false if not (key not present or incompatible data type).
	 *
	 * The outer object must be a JSON object, not an array.
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within an object, use getValueTokenByKey() instead.
	 */
	template<class T>
	bool getOuterValueByKey(const char *name, T &result) const {
		const JsonParserGeneratorRK::jsmntok_t *value;

		if (getValueTokenByKey(getOuterObject(), name, value)) {
			return getTokenValue(value, result);
		}
		else {
			return false;
		}
	}

	/**
	 * @brief Gets the key/value pair of an object by index
	 *
	 * @param container The object to look in (see getOuterKeyValueByIndex if you want to the outermost object you parsed)
	 *
	 * @param index 0 = first, 1 = second, ...
	 *
	 * @param key Filled in with the name of the key
	 *
	 * @param result Filled in with the value. The value can be of type: bool, int, unsigned long, float, double, String,
	 * or (char *, size_t&).
	 *
	 * @return true if the call succeeded or false if it failed.
	 *
	 * Normally you get a value in an object by its key, but if you want to iterate all of the keys you can
	 * use this method. Call it until it returns false.
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within an object, use getValueTokenByKey() instead.
	 */
	template<class T>
	bool getKeyValueByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t index, String &key, T &result) const {
		const JsonParserGeneratorRK::jsmntok_t *keyToken;
		const JsonParserGeneratorRK::jsmntok_t *valueToken;

		if (getKeyValueTokenByIndex(container, keyToken, valueToken, index)) {
			getTokenValue(keyToken, key);
			return getTokenValue(valueToken, result);
		}
		else {
			return false;
		}
	}

	/**
	 * @brief Gets the key/value pair of the outer object by index (0 = first, 1 = second, ...)
	 *
	 * Normally you get a value in an object by its key, but if you want to iterate all of the keys you can
	 * use this method.
	 *
	 * @param index 0 = first, 1 = second, ...
	 *
	 * @param key Filled in with the name of the key
	 *
	 * @param result Filled in with the value. The value can be of type: bool, int, unsigned long, float, double, String,
	 * or (char *, size_t&).
	 *
	 * @return true if the call succeeded or false if it failed.
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within an object, use getValueTokenByKey() instead.
	 */
	template<class T>
	bool getOuterKeyValueByIndex(size_t index, String &key, T &result) const {
		return getKeyValueByIndex(getOuterObject(), index, key, result);
	}


	/**
	 * @brief Given an array token in arrayContainer, gets the value with the specified index.
	 *
	 * @param arrayContainer A token for an array
	 *
	 * @param index The index in the array. 0 = first item, 1 = second item, ...
	 *
	 * @param result Filled in with the value. The value can be of type: bool, int, unsigned long, float, double, String,
	 * or (char *, size_t&).
	 *
	 * @return true if the call succeeded or false if it failed. You can call this repeatedly until it returns
	 * false to iterate the array.
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within an array, use getValueTokenByIndex() instead.
	 */
	template<class T>
	bool getValueByIndex(const JsonParserGeneratorRK::jsmntok_t *arrayContainer, size_t index, T &result) const {
		const JsonParserGeneratorRK::jsmntok_t *value;

		if (getValueTokenByIndex(arrayContainer, index, value)) {
			return getTokenValue(value, result);
		}
		else {
			return false;
		}
	}

	/**
	 * @brief This method is used to extract data from a 2-dimensional JSON array, an array of arrays of values.
	 *
	 * @param arrayContainer A token for an array containing another array
	 *
	 * @param col The column (outer array index, 0 = first column, 1 = second column, ...)
	 *
	 * @param row The row (inner array index, 0 = first row, 1 = second row, ...)
	 *
	 * @param result Filled in with the value. The value can be of type: bool, int, unsigned long, float, double, String,
	 * or (char *, size_t&).
	 *
	 * @return true if the call succeeded or false if it failed. You can call this repeatedly until it returns
	 * false to iterate the array.
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within a two-dimensional array, use getValueTokenByColRow() instead.
	 */
	template<class T>
	bool getValueByColRow(const JsonParserGeneratorRK::jsmntok_t *arrayContainer, size_t col, size_t row, T &result) const {
		const JsonParserGeneratorRK::jsmntok_t *value;

		if (getValueTokenByColRow(arrayContainer, col, row, value)) {
			return getTokenValue(value, result);
		}
		else {
			return false;
		}
	}

	/**
	 * @brief Given an object token in container, gets the token value with the specified key name.
	 *
	 * @param container The object token to look in.
	 *
	 * @param key The key to look for.
	 *
	 * @param value Filled in with the token for the value for key.
	 *
	 * @return true if the key is found or false if not.
	 *
	 * This can be used for objects whose keys are arrays or objects, to get the token for the container. It can
	 * also be used for values, but normally you'd use getValueByKey() instead, which is generally more convenient.
	 */
	bool getValueTokenByKey(const JsonParserGeneratorRK::jsmntok_t *container, const char *key, const JsonParserGeneratorRK::jsmntok_t *&value) const;

	/**
	 * @brief Given an array token in container, gets the token value with the specified index.
	 *
	 * @param container The array token to look in.
	 *
	 * @param desiredIndex The index to retrieve (0 = first, 1 = second, ...).
	 *
	 * @param value Filled in with the token for the value for key.
	 *
	 * @return true if the index is valid or false if the index exceeds the size of the array.
	 *
	 * This can be used for arrays whose values are arrays or objects, to get the token for the container. It can
	 * also be used for values, but normally you'd use getValueByIndex() instead, which is generally more convenient.
	 */
	bool getValueTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t desiredIndex, const JsonParserGeneratorRK::jsmntok_t *&value) const;

	/**
	 * @brief This method is used to extract data from a 2-dimensional JSON array, an array of arrays of values.
	 *
	 * @param container A token for an array containing another array
	 *
	 * @param col The column (outer array index, 0 = first column, 1 = second column, ...)
	 *
	 * @param row The row (inner array index, 0 = first row, 1 = second row, ...)
	 *
	 * @param value Filled in with the token for the value for key.
	 *
	 * @return true if the index row and column are valid or false if either is out of range.
	 *
	 * This can be used for 2-dimensional arrays whose values are arrays or objects, to get the token for the container. It can
	 * also be used for values, but normally you'd use getValueByColRow() instead, which is generally more convenient.
	 */
	bool getValueTokenByColRow(const JsonParserGeneratorRK::jsmntok_t *container, size_t col, size_t row, const JsonParserGeneratorRK::jsmntok_t *&value) const;


	/**
	 * @brief Given a containing object, finds the nth token in the object. Internal use only.
	 *
	 * @param container The array token to look in.
	 *
	 * @param desiredIndex The index to retrieve (0 = first, 1 = second, ...).
	 *
	 * @return The token
	 *
	 * This is used internally. It should not be used to get the nth array value, use getValueTokenByIndex instead.
	 */
	const JsonParserGeneratorRK::jsmntok_t *getTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t desiredIndex) const;

	/**
	 * @brief Given a JSON object in container, gets the key/value pair specified by index. Internal use only.
	 *
	 * @param container The array token to look in.
	 *
	 * @param key Filled in with the key token for nth key value pair.
	 *
	 * @param value Filled in with the value token for then nth key value pair.
	 *
	 * @param index The index to retrieve (0 = first, 1 = second, ...).
	 *
	 * This is a low-level function; you will typically use getValueByIndex() or getValueByKey() instead.
	 */
	bool getKeyValueTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, const JsonParserGeneratorRK::jsmntok_t *&key, const JsonParserGeneratorRK::jsmntok_t *&value, size_t index) const;


	/**
	 * @brief Used internally to skip over the token in obj.
	 *
	 * @param container The array token to look in.
	 *
	 * @param obj Object within the token, updated to the next object if true is returned
	 *
	 * @return true if there was a next object, false if not.
	 *
	 * For simple primitives and strings, this is equivalent to obj++. For objects and arrays,
	 * however, this skips over the entire object or array, including any nested objects within
	 * them.
	 */
	bool skipObject(const JsonParserGeneratorRK::jsmntok_t *container, const JsonParserGeneratorRK::jsmntok_t *&obj) const;

	/**
	 * @brief Copies the value of the token into a buffer, making it a null-terminated cstring.
	 *
	 * If the string is longer than dstLen - 1 bytes, it will be truncated and the result will
	 * still be a valid cstring.
	 *
	 * This is used internally because the token data is not null-terminated, and doing
	 * things like sscanf or strtoul on it can read past the end of the buffer. This assures
	 * that only null-terminated data is passed to these functions.
	 */
	void copyTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, char *dst, size_t dstLen) const;

	/**
	 * @brief Gets a bool (boolean) value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a bool variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, bool &result) const;

	/**
	 * @brief Gets an integer value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is an int variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, int &result) const;

	/**
	 * @brief Gets an unsigned long value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is an unsigned long variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, unsigned long &result) const;

	/**
	 * @brief Gets a float (single precision floating point) value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a float variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, float &result) const;

	/**
	 * @brief Gets a double (double precision floating point) value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a double variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, double &result) const;

	/**
	 * @brief Gets a String value into a Wiring String object.
	 *
	 * This will automatically decode Unicode character escapes in the data and the
	 * returned String will contain UTF-8.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a String variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, String &result) const;

	/**
	 * @brief Gets a string as a c-string into the specified buffer.
	 *
	 * If the token specifies too large of a string
	 * it will be truncated. This will automatically decode Unicode character escapes in the data and the
	 * returned string will contain UTF-8.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, char *str, size_t &strLen) const;

	/**
	 * @brief Gets a string as a JsonParserString object.
	 *
	 * This is used internally by getTokenValue() overloads
	 * that take a String or buffer and length; you will normally not need to use this directly.
	 *
	 * This will automatically decode Unicode character escapes in the data and the
	 * returned string will contain UTF-8.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, JsonParserString &str) const;

	/**
	 * @brief Converts a token (object, array, string, or primitive) back into JSON in a Wiring String.
	 *
	 * @param token The token to convert back to a string
	 *
	 * @param result Filled in with the string. Any previous contents in the string are cleared first.
	 */
	bool getTokenJsonString(const JsonParserGeneratorRK::jsmntok_t *token, String &result) const;

	/**
	 * @brief Converts a token (object, array, string, or primitive) back into JSON in a buffer.
	 *
	 * @param token The token to convert back to a string
	 *
	 * @param str The buffer to be written to
	 *
	 * @param strLen The length of the buffer on entry, set to the number of bytes written on exit.
	 */
	bool getTokenJsonString(const JsonParserGeneratorRK::jsmntok_t *token, char *str, size_t &strLen) const;

	/**
	 * @brief Gets a token as a JSON string.
	 *
	 * @param token The token to convert back to a string
	 *
	 * @param str The JsonParserString object to write to
	 *
	 * This overload is typically used internally, normally you'd use
	 * the version that takes a String& or char *, size_t.
	 */
	bool getTokenJsonString(const JsonParserGeneratorRK::jsmntok_t *token, JsonParserString &str) const;

	/**
	 * @brief Used internally in the test suite for printing the token list
	 */
	JsonParserGeneratorRK::jsmntok_t *getTokens() { return tokens; };

	/**
	 * @brief Used internally in the test suite for printing the token list
	 */
	JsonParserGeneratorRK::jsmntok_t *getTokensEnd() { return tokensEnd; };

	/**
	 * @brief Used internally in the test suite for printing the token list
	 */
	size_t getMaxTokens() const { return maxTokens; };

	/**
	 * @brief Given a Unicode UTF-16 code point, converts it to UTF-8 and appends it to str.
	 */
	static void appendUtf8(uint16_t unicode, JsonParserString &str);

protected:
	JsonParserGeneratorRK::jsmntok_t *tokens; //!< Array of tokens after parsing.
	JsonParserGeneratorRK::jsmntok_t *tokensEnd; //!< Pointer into tokens, points after last used token.
	size_t	maxTokens; //!< Number of tokens that can be stored in tokens.
	JsonParserGeneratorRK::jsmn_parser parser;//!< The JSMN parser object.

	friend class JsonModifier; // To access the tokens for modifying a JSON object in place
};

/**
 * @brief Creates a JsonParser with a static buffer.
 *
 * You normally use this when you're creating a parser as a global variable. For small data (under around
 * 256 bytes so) you can also allocate one on the stack.
 *
 * @param BUFFER_SIZE The maximum size of the data to be parsed, in bytes. If you are parsing a webhook response
 * split into parts, this is the total size of all parts.
 *
 * @param MAX_TOKENS The maximum number of tokens you expect. Each object has a token and two for each key/value pair.
 * Each array is a token and one for each element in the array.
 */
template <size_t BUFFER_SIZE, size_t MAX_TOKENS>
class JsonParserStatic : public JsonParser {
public:
	/**
	 * @brief Construct a JsonParser using a static buffer and static maximum number of tokens.
	 */
	explicit JsonParserStatic() : JsonParser(staticBuffer, BUFFER_SIZE, staticTokens, MAX_TOKENS) {};

private:
	char staticBuffer[BUFFER_SIZE];//!< The static buffer to hold the data
	JsonParserGeneratorRK::jsmntok_t staticTokens[MAX_TOKENS]; //!< The static buffer to hold the tokens.
};


/**
 * @brief This class provides a fluent-style API for easily traversing a tree of JSON objects to find a value
 */
class JsonReference {
public:
	/**
	 * @brief Constructs an object. Normally you use the JsonParser getReference() method to get one of these
	 * instead of constructing one.
	 *
	 * @param parser The JsonParser object you're traversing
	 */
	JsonReference(const JsonParser *parser);

	/**
	 * @brief Destructor. This does not affect the lifecycle of the JsonParser.
	 */
	virtual ~JsonReference();

	/**
	 * @brief Constructs are JsonReference for a specific token within a JsonParser
	 */
	JsonReference(const JsonParser *parser, const JsonParserGeneratorRK::jsmntok_t *token);

	/**
	 * @brief For JsonReference that refers to a JSON object, gets a new JsonReference to a value with the specified key name.
	 *
	 * @param name of the key to look for.
	 *
	 * @return A JsonReference to the value for this key.
	 */
	JsonReference key(const char *name) const;

	/**
	 * @brief For a JsonReference that refers to a JSON array, gets a new JsonReference to a value in the array by index.
	 *
	 * @param index The index to retrieve (0 = first item, 1 = second item, ...).
	 *
	 * @return A JsonReference to the value for this index.
	 */
	JsonReference index(size_t index) const ;

	/**
	 * @brief For a JsonReference that refers to a JSON array, gets the size of the array.
	 *
	 * @return 0 = an empty array, 1 = one element, ...
	 */
	size_t size() const;

	/**
	 * @brief Get a value of the specified type for a given value for a specified key, or index for an array.
	 *
	 * @param result Filled in with the value. The value can be of type: bool, int, unsigned long, float, double, String,
	 * or (char *, size_t&).
	 *
	 * There are also type-specific versions like valueBool that return the value, instead of having to pass an object
	 * to hold the value, as in this call.
	 */
	template<class T>
	bool value(T &result) const {
		if (token && parser->getTokenValue(token, result)) {
			return true;
		}
		else {
			return false;
		}
	}

	/**
	 * @brief Returns a boolean (bool) value for an object value for key, or array index
	 *
	 * @param defaultValue Optional value to use if the key or array index is not found. Default: false.
	 */
	bool valueBool(bool defaultValue = false) const;

	/**
	 * @brief Returns a integer (int) value for an object value for key, or array index
	 *
	 * @param defaultValue Optional value to use if the key or array index is not found. Default: 0.
	 */
	int valueInt(int defaultValue = 0) const;

	/**
	 * @brief Returns a unsigned long integer for an object value for key, or array index
	 *
	 * @param defaultValue Optional value to use if the key or array index is not found. Default: 0.
	 */
	unsigned long valueUnsignedLong(unsigned long defaultValue = 0) const;

	/**
	 * @brief Returns a float value for an object value for key, or array index
	 *
	 * @param defaultValue Optional value to use if the key or array index is not found. Default: 0.0.
	 */
	float valueFloat(float defaultValue = 0.0) const;

	/**
	 * @brief Returns a double value for an object value for key, or array index
	 *
	 * @param defaultValue Optional value to use if the key or array index is not found. Default: 0.0.
	 */
	double valueDouble(double defaultValue = 0.0) const;

	/**
	 * @brief Returns a String value for an object value for key, or array index
	 *
	 * @return The string value, or an empty string if the key or array index is not found.
	 */
	String valueString() const;

private:
	const JsonParser *parser;
	const JsonParserGeneratorRK::jsmntok_t *token;
};

/**
 * @brief Used internally by JsonWriter
 */
typedef struct {
	bool isFirst;		//!< True if this the first element in this object or array and doesn't need a comma before it
	char terminator;	//!< The character that will terminate the object or array when ended
} JsonWriterContext;

/**
 * @brief Class for building a JSON string
 */
class JsonWriter : public JsonBuffer {
public:
	/**
	 * @brief Construct a JsonWriter with a dynamically allocated buffer
	 *
	 * The buffer will be resized as necessary but you can improve efficiency by using the allocate() method
	 * of JsonBuffer to pre-allocate space rather than have to incrementally make it bigger as it's written to.
	 *
	 * Use getBuffer() to get the pointer to the buffer and getOffset() to get the buffer pointer and size. The
	 * buffer is not null-terminated!
	 */
	JsonWriter();

	/**
	 * @brief Destroy the object. If the buffer was dynamically allocated it will be freed.
	 *
	 * If the buffer was passed in using the buffer, bufferLen constructor the buffer is not freed by this call
	 * as it's likely statically allocated.
	 */
	virtual ~JsonWriter();

	/**
	 * @brief Construct a JsonWriter to write to a static buffer
	 *
	 * @param buffer Pointer to the buffer
	 *
	 * @param bufferLen Length of the buffer in bytes
	 *
	 */
	JsonWriter(char *buffer, size_t bufferLen);

	/**
	 * @brief Reset the writer, clearing all data
	 *
	 * You do not need to call init() as it's called from the two constructors. You can call it again
	 * if you want to reset the writer and reuse it, such as when you use JsonWriterStatic in a global
	 * variable.
	 */
	void init();

	/**
	 * @brief Start a new JSON object. Make sure you finish it with finishObjectOrArray()
	 */
	bool startObject() { return startObjectOrArray('{', '}'); };

	/**
	 * @brief Start a new JSON array. Make sure you finish it with finishObjectOrArray()
	 */
	bool startArray() { return startObjectOrArray('[', ']'); };

	/**
	 * @brief Finsh an object or array started with startObject() or startArray()
	 */
	void finishObjectOrArray();

	/**
	 * @brief Inserts a boolean value ("true" or "false").
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(bool value);

	/**
	 * @brief Inserts an integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(int value) { insertsprintf("%d", value); }

	/**
	 * @brief Inserts an unsigned integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(unsigned int value) { insertsprintf("%u", value); }

	/**
	 * @brief Inserts a long integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(long value) { insertsprintf("%ld", value); }

	/**
	 * @brief Inserts an unsigned long integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(unsigned long value) { insertsprintf("%lu", value); }

	/**
	 * @brief Inserts a floating point value.
	 *
	 * Use setFloatPlaces() to set the number of decimal places to include.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(float value);

	/**
	 * @brief Inserts a floating point double value.
	 *
	 * Use setFloatPlaces() to set the number of decimal places to include.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(double value);

	/**
	 * @brief Inserts a quoted string value. This escapes special characters and encodes utf-8.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(const char *value) { insertString(value, true); }

	/**
	 * @brief Inserts a quoted string value.
	 *
	 * This escapes special characters and encodes utf-8.
	 * See also the version that takes a plain const char *.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(const String &value) { insertString(value.c_str(), true); }

	/**
	 * @brief Inserts a new key and empty object. You must close the object using finishObjectOrArray()!

	 * @param key the key name to insert
	 */
	void insertKeyObject(const char *key);

	/**
	 * @brief Inserts a new key and empty array. You must close the object using finishObjectOrArray()!
	 *
	 * @param key the key name to insert
	 */
	void insertKeyArray(const char *key);

	/**
	 * @brief Inserts a key/value pair into an object.
	 *
	 * Uses templates so you can pass any type object that's supported by insertValue() overloads,
	 * for example: bool, int, float, double, const char *.
	 */
	template<class T>
	void insertKeyValue(const char *key, T value) {
		insertCheckSeparator();
		insertValue(key);
		insertChar(':');
		insertValue(value);
	}

	/**
	 * @brief Inserts a value into an array.
	 *
	 * Uses templates so you can pass any type object that's supported by insertValue() overloads,
	 * for example: bool, int, float, double, const char *.
	 */
	template<class T>
	void insertArrayValue(T value) {
		insertCheckSeparator();
		insertValue(value);
	}

	/**
	 * @brief Inserts an array of values into an array.
	 *
	 * Uses templates so you can pass any type object that's supported by insertValue() overloads,
	 * for example: bool, int, float, double, const char *.
	 */
	template<class T>
	void insertArray(T *pArray, size_t numElem) {
		for(size_t ii = 0; ii < numElem; ii++) {
			insertArrayValue(pArray[ii]);
		}
	}

	/**
	 * @brief Inserts a new key and vector of values
	 *
	 * @param key the key name to insert
	 * 
	 * @param vec the vector to insert
	 */
	template<class T>
	void insertKeyArray(const char *key, T *pArray, size_t numElem) {
		insertKeyArray(key);
		insertArray(pArray, numElem);
		finishObjectOrArray();
	}

	/**
	 * @brief Inserts an array of values into an array.
	 *
	 * Uses templates so you can pass any type object that's supported by insertValue() overloads,
	 * for example: bool, int, float, double, const char *.
	 */
	template<class T>
	void insertVector(std::vector<T> vec) {
		for (auto it = vec.begin(); it != vec.end(); ++it) {
			insertArrayValue(*it);
		}
	}

	/**
	 * @brief Inserts a new key and vector of values
	 *
	 * @param key the key name to insert
	 * 
	 * @param vec the vector to insert
	 */
	template<class T>
	void insertKeyVector(const char *key, std::vector<T> vec) {
		insertKeyArray(key);
		insertVector(vec);
		finishObjectOrArray();
	}

	/**
	 * If you try to insert more data than will fit in the buffer, the isTruncated flag will be
	 * set, and the buffer will likely not be valid JSON and should not be used.
	 */
	bool isTruncated() const { return truncated; }

	/**
	 * @brief Sets the number of digits for formatting float and double values.
	 *
	 * @param floatPlaces The number of decimal places for float and double.
	 * Set it to -1 to use the default for snprintf. -1 is the default value if you don't call setFloatPlaces.
	 */
	void setFloatPlaces(int floatPlaces) { this->floatPlaces = floatPlaces; }

	/**
	 * @brief Check to see if a separator needs to be inserted. Used internally.
	 *
	 * You normally don't need to use this
	 * as it's called by insertKeyValue() and insertArrayValue().
	 */
	void insertCheckSeparator();

	/**
	 * @brief Used internally to start an object or array
	 *
	 * Used internally; you should use startObject() or startArray() instead.
	 * Make sure you finish any started object or array using finishObjectOrArray().
	 */
	bool startObjectOrArray(char startChar, char endChar);

	/**
	 * @brief Used internally to insert a character
	 *
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string instead.
	 */
	void insertChar(char ch);

	/**
	 * @brief Used internally to insert a string, quoted or not.
	 *
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string instead.
	 */
	void insertString(const char *s, bool quoted = false);

	/**
	 * @brief Used internally to insert using snprintf formatting.
	 *
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string, float, or
	 * double instead.
	 *
	 * This method does not quote or escape the string - it's used mainly for formatting numbers.
	 */
	void insertsprintf(const char *fmt, ...);

	/**
	 * @brief Used internally to insert using snprintf formatting with a va_list.
	 *
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string, float, or
	 * double instead.
	 *
	 * This method does not quote or escape the string - it's used mainly for formatting numbers.
	 */
	void insertvsprintf(const char *fmt, va_list ap);

	/**
	 * @brief Used internally to set the current isFirst flag in the context
	 */
	void setIsFirst(bool isFirst = true);

	/**
	 * This constant is the maximum number of nested objects that are supported; the actual number is
	 * one less than this so when set to 9 you can have eight objects nested in each other.
	 *
	 * Overhead is 8 bytes per nested context, so 9 elements is 72 bytes.
	 */
	static const size_t MAX_NESTED_CONTEXT = 9;

protected:
	size_t contextIndex;							//!< Index into the context for the current level of nesting
	JsonWriterContext context[MAX_NESTED_CONTEXT]; 	//!< Structure for managing nested objects
	bool truncated; 								//!< true if data was added that didn't fit and was truncated
	int floatPlaces; 								//!< default number of places to display for floating point numbers (default is -1, the default for sprintf)
};


/**
 * @brief Creates a JsonWriter with a statically allocated buffer.
 *
 * You typically do this when you want to create a buffer as a global variable.
 *
 * Example:
 *
 * ```
 * JsonWriterStatic<256> jsonWriter;
 * ```
 *
 * Creates a 256 byte buffer to write JSON to. You'd normally do this as a global variable, but for smaller
 * buffers (256 and smaller should be fine) in the loop thread, you can allocate one on the stack as a local
 * variable.
 *
 * @param BUFFER_SIZE The size of the buffer to reserve.
 */
template <size_t BUFFER_SIZE>
class JsonWriterStatic : public JsonWriter {
public:
	explicit JsonWriterStatic() : JsonWriter(staticBuffer, BUFFER_SIZE) {};

private:
	char staticBuffer[BUFFER_SIZE]; //!< static buffer to write to
};

/**
 * @brief Class for creating a JSON object with JsonWriter
 *
 * When you create an object, you must call startObject() to start and finishObjectOrArray() to complete it.
 *
 * This class is instantiated on the stack to automatically start and finish for you.
 */
class JsonWriterAutoObject {
public:
	/**
	 * @brief Start a new object
	 *
	 * @param jw The JsonWriter object to insert the object into
	 */
	JsonWriterAutoObject(JsonWriter *jw) : jw(jw) {
		jw->startObject();
	}

	/**
	 * @brief End the object
	 */
	~JsonWriterAutoObject() {
		jw->finishObjectOrArray();
	}

protected:
	JsonWriter *jw; //!< JsonWriter to write to
};

/**
 * @brief Class for creating a JSON array with JsonWriter
 *
 * When you create an object, you must call startArray() to start and finishObjectOrArray() to complete it.
 *
 * This class is instantiated on the stack to automatically start and finish for you.
 */
class JsonWriterAutoArray {
public:
	/**
	 * @brief Start a new array
	 *
	 * @param jw The JsonWriter object to insert the array into
	 */
	JsonWriterAutoArray(JsonWriter *jw) : jw(jw) {
		jw->startArray();
	}

	/**
	 * @brief End the array
	 */
	~JsonWriterAutoArray() {
		jw->finishObjectOrArray();
	}

protected:
	JsonWriter *jw; //!< JsonWriter to write to
};

/**
 * @brief Class for modifying a JSON object in place, without needing to make a copy of it
 *
 * Make sure the underlying JsonParser is big enough to hold the modified object. If you use
 * JsonParserStatic<> make sure you have enough bytes and tokens.
 *
 * The most commonly used method is insertOrUpdateKeyValue(). This inserts or updates a key
 * in an array. Another is appendArrayValue() which appends a value to an array. Both methods
 * are templated so you can use them with any valid type supported by insertValue() in
 * JsonWriter: bool, int, float, double, const char *.
 *
 * This class is a subclass of JsonWriter, so you can also use the low-level functions and
 * JsonWriter methods to do unusual object manipulations.
 *
 * You can also use removeKeyValue() and removeArrayIndex() to remove keys or array entries.
 */
class JsonModifier : public JsonWriter {
public:
	JsonModifier(JsonParser &jp);
	virtual ~JsonModifier();

	/**
	 * @brief Inserts or updates a key/value pair into an object.
	 *
	 * Uses templates so you can pass any type object that's supported by insertValue() overloads,
	 * for example: bool, int, float, double, const char *.
	 *
	 * To modify the outermost object, use jp.getOuterObject() for the container.
	 *
	 * Note: This method call jp.parse() so any jsmntok_t may be changed by this method. If you've
	 * fetched one, such as by using getValueTokenByKey() be sure to fetch it again to be safe.
	 */
	template<class T>
	void insertOrUpdateKeyValue(const JsonParserGeneratorRK::jsmntok_t *container, const char *key, T value) {
		// Remove existing item (ignore failure, as the key might not exist)
		removeKeyValue(container, key);

		// Create a new key/value pair
		startAppend(container);
		insertKeyValue(key, value);
		finish();
	}

	/**
	 * @brief Appends a value to an array
	 *
	 * Uses templates so you can pass any type object that's supported by insertValue() overloads,
	 * for example: bool, int, float, double, const char *.
	 *
	 * To modify the outermost array, use jp.getOuterArray() for the arrayToken. You can also
	 * modify arrays in an object using getValueTokenByKey().
	 *
	 * Note: This method call jp.parse() so any jsmntok_t may be changed by this method. If you've
	 * fetched one, such as by using getValueTokenByKey() be sure to fetch it again to be safe.
	 */
	template<class T>
	void appendArrayValue(const JsonParserGeneratorRK::jsmntok_t *arrayToken, T value) {
		startAppend(arrayToken);
		insertArrayValue(value);
		finish();
	}

	/**
	 * @brief Removes a key and value from an object
	 *
	 * Note: This method call jp.parse() so any jsmntok_t may be changed by this method. If you've
	 * fetched one, such as by using getValueTokenByKey() be sure to fetch it again to be safe.
	 */
	bool removeKeyValue(const JsonParserGeneratorRK::jsmntok_t *container, const char *key);

	/**
	 * @brief Removes an entry from an array
	 *
	 * Note: This method call jp.parse() so any jsmntok_t may be changed by this method. If you've
	 * fetched one, such as by using getValueTokenByKey() be sure to fetch it again to be safe.
	 */
	bool removeArrayIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t index);

	/**
	 * @brief Low level function to modify a token in place
	 *
	 * @param token the jsmntok_t to modify
	 *
	 * You must call finish() after modification is done to restore the object to a valid state!
	 *
	 * Note: insertOrUpdateKeyValue() does not use this. Instead it removes then appends the new
	 * value. The reason is that startModify does not work if you change the type of the data to
	 * or from a string. This is tricky to deal with correctly, so it's easier to just remove
	 * and add the item again.
	 */
	bool startModify(const JsonParserGeneratorRK::jsmntok_t *token);

	/**
	 * @brief Low level function to append to an object or array
	 *
	 * @param arrayOrObjectToken the jsmntok_t to append to. This must be an object or array token.
	 *
	 * You must call finish() after modification is done to restore the object to a valid state.
	 */
	bool startAppend(const JsonParserGeneratorRK::jsmntok_t *arrayOrObjectToken);

	/**
	 * @brief Finish modifying the object
	 *
	 * Finish must be called after startModify or startAppend otherwise the
	 * object will be corrupted.
	 *
	 * Note: This method call jp.parse() so any jsmntok_t may be changed by this method. If you've
	 * fetched one, such as by using getValueTokenByKey() be sure to fetch it again to be safe.
	 *
	 * The high level function like insertOrUpdateKeyValue, appendArrayValue, removeKeyValue,
	 * and removeArrayIndex internally call finish so you should not call it again with those
	 * methods.
	 */
	void finish();


	/**
	 * @brief Return a copy of tok, but moving so start and end include the double quotes for strings
	 *
	 * Used internally, you probably won't need to use this.
	 */
	JsonParserGeneratorRK::jsmntok_t tokenWithQuotes(const JsonParserGeneratorRK::jsmntok_t *tok) const;

	/**
	 * @brief Find the offset of the comma to the left of the token, or -1 if there isn't one
	 *
	 * Used internally, you probably won't need to use this.
	 */
	int findLeftComma(const JsonParserGeneratorRK::jsmntok_t *tok) const;

	/**
	 * @brief Find the offset of the comma to the left of the token, or -1 if there isn't one
	 *
	 * Used internally, you probably won't need to use this.
	 */
	int findRightComma(const JsonParserGeneratorRK::jsmntok_t *tok) const;


protected:
	JsonParser &jp;				//!< The JsonParser object passed to the constructor
	int start = -1;				//!< Start offset in the buffer. Set to -1 when startModify() or startAppend() is not in progress.
	int origAfter = 0;			//!< Number of bytes after the insertion position, saved at saveLoc when start is in progress.
	int saveLoc = 0;			//!< Location where data is temporarily saved until finish() is called
	//bool addSeparator = false;	//!< Set by startAppend() and used by insertCheckSeparator()
};



#endif /* __JSONPARSERGENERATORRK_H */

