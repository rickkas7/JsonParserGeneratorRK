#ifndef __JSONPARSERGENERATORRK_H
#define __JSONPARSERGENERATORRK_H


#include "Particle.h"

// You can mostly ignore the stuff in this namespace block. It's part of the jsmn library
// that's used internally and you can mostly ignore. The actual API is the JsonParser C++ object
// below.
namespace JsonParserGeneratorRK {
	// begin jsmn.h
	// https://github.com/zserge/jsmn

	/**
	 * JSON type identifier. Basic types are:
	 * 	o Object
	 * 	o Array
	 * 	o String
	 * 	o Other primitive: number, boolean (true/false) or null
	 */
	typedef enum {
		JSMN_UNDEFINED = 0,
		JSMN_OBJECT = 1,
		JSMN_ARRAY = 2,
		JSMN_STRING = 3,
		JSMN_PRIMITIVE = 4
	} jsmntype_t;

	enum jsmnerr {
		/* Not enough tokens were provided */
		JSMN_ERROR_NOMEM = -1,
		/* Invalid character inside JSON string */
		JSMN_ERROR_INVAL = -2,
		/* The string is not a full JSON packet, more bytes expected */
		JSMN_ERROR_PART = -3
	};

	/**
	 * JSON token description.
	 * type		type (object, array, string etc.)
	 * start	start position in JSON data string
	 * end		end position in JSON data string
	 */
	typedef struct {
		jsmntype_t type;
		int start;
		int end;
		int size;
	#ifdef JSMN_PARENT_LINKS
		int parent;
	#endif
	} jsmntok_t;

	/**
	 * JSON parser. Contains an array of token blocks available. Also stores
	 * the string being parsed now and current position in that string
	 */
	typedef struct {
		unsigned int pos; /* offset in the JSON string */
		unsigned int toknext; /* next token to allocate */
		int toksuper; /* superior token node, e.g parent object or array */
	} jsmn_parser;

	/**
	 * Create JSON parser over an array of tokens
	 */
	void jsmn_init(jsmn_parser *parser);

	/**
	 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
	 * a single JSON object.
	 */
	int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
			jsmntok_t *tokens, unsigned int num_tokens);

	// end jsmn.h
}

/**
 * Class used internally for writing to strings. This is a wrapper around either
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
	JsonParserString(String *str);
	JsonParserString(char *buf, size_t bufLen);

	void append(char ch);
	size_t getLength() const { return length; }

protected:
	String *str;
	char *buf;
	size_t bufLen;
	size_t length;
};

/**
 * Base class for managing a static or dynamic buffer, used by both JsonParser and JsonWriter
 */
class JsonBuffer {
public:
	JsonBuffer();
	virtual ~JsonBuffer();

	JsonBuffer(char *buffer, size_t bufferLen);

	bool allocate(size_t len);

	bool addString(const char *data) { return addData(data, strlen(data)); }
	bool addData(const char *data, size_t dataLen);

	char *getBuffer() const { return buffer; }
	size_t getOffset() const { return offset; }
	size_t getBufferLen() const { return bufferLen; }

	void clear();

protected:
	char	*buffer;
	size_t	bufferLen;
	size_t	offset;
	bool 	staticBuffers;

};

class JsonReference;


/**
 * API to the JsonParser
 *
 * This is a memory-efficient JSON parser based on jsmn. It only keeps one copy of the data in raw format
 * and an array of tokens. You make calls to read values out.
 */
class JsonParser : public JsonBuffer {
public:
	JsonParser();
	virtual ~JsonParser();

	/**
	 * Static buffers constructor
	 */
	JsonParser(char *buffer, size_t bufferLen, JsonParserGeneratorRK::jsmntok_t *tokens, size_t maxTokens);

	/**
	 * Optional: Allocates the specified number of tokens. You should set this larger than the expected number
	 * of tokens for efficiency, but if you are not using the static allocator it will resize the
	 * token storage space if it's too small.
	 */
	bool allocateTokens(size_t maxTokens);

	/**
	 * Parses the data you have added using addData() or addString().
	 */
	bool parse();

	/**
	 *
	 */
	JsonReference getReference() const;

	/**
	 * Typically JSON will contain an object that contains values and possibly other objects.
	 * This method gets the token for the outer object.
	 */
	const JsonParserGeneratorRK::jsmntok_t *getOuterObject() const;

	/**
	 * Sometimes the JSON will contain an array of values (or objects) instead of starting with
	 * an object. This gets the outermost array.
	 */
	const JsonParserGeneratorRK::jsmntok_t *getOuterArray() const;

	/**
	 * Gets the outer token, whether it's an array or object
	 */
	const JsonParserGeneratorRK::jsmntok_t *getOuterToken() const;

	/**
	 * Given a token for an JSON array in arrayContainer, gets the number of elements in the array.
	 *
	 * 0 = no elements, 1 = one element, ...
	 *
	 * The index values for getValueByIndex(), etc. are 0-based, so the last index you pass in is
	 * less than getArraySize().
	 */
	size_t getArraySize(const JsonParserGeneratorRK::jsmntok_t *arrayContainer) const;

	/**
	 * Given an object token in container, gets the value with the specified key name.
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
	 * Gets the value with the specified key name out of the outer object
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
	 * Gets the key/value pair of an object by index (0 = first, 1 = second, ...)
	 *
	 * Normally you get a value in an object by its key, but if you want to iterate all of the keys you can
	 * use this method.
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
	 * Gets the key/value pair of the outer object by index (0 = first, 1 = second, ...)
	 *
	 * Normally you get a value in an object by its key, but if you want to iterate all of the keys you can
	 * use this method.
	 *
	 * This should only be used for things like string, numbers, booleans, etc.. If you want to get a JSON array
	 * or object within an object, use getValueTokenByKey() instead.
	 */
	template<class T>
	bool getOuterKeyValueByIndex(size_t index, String &key, T &result) const {
		return getKeyValueByIndex(getOuterObject(), index, key, result);
	}


	/**
	 * Given an array token in arrayContainer, gets the value with the specified index.
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
	 * This method is used to extract data from a 2-dimensional JSON array, an array of arrays of values.
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
	 * Given an object token in container, gets the token value with the specified key name.
	 *
	 * This can be used for objects whose keys are arrays or objects, to get the token for the container. It can
	 * also be used for values, but normally you'd use getValueByKey() instead, which is generally more convenient.
	 */
	bool getValueTokenByKey(const JsonParserGeneratorRK::jsmntok_t *container, const char *key, const JsonParserGeneratorRK::jsmntok_t *&value) const;

	/**
	 * Given an array token in container, gets the token value with the specified index.
	 *
	 * This can be used for arrays whose values are arrays or objects, to get the token for the container. It can
	 * also be used for values, but normally you'd use getValueByIndex() instead, which is generally more convenient.
	 */
	bool getValueTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t desiredIndex, const JsonParserGeneratorRK::jsmntok_t *&value) const;

	/**
	 * This method is used to extract data from a 2-dimensional JSON array, an array of arrays of values.
	 *
	 * This can be used for 2-dimensional arrays whose values are arrays or objects, to get the token for the container. It can
	 * also be used for values, but normally you'd use getValueByColRow() instead, which is generally more convenient.
	 */
	bool getValueTokenByColRow(const JsonParserGeneratorRK::jsmntok_t *container, size_t col, size_t row, const JsonParserGeneratorRK::jsmntok_t *&value) const;


	/**
	 * Given a containing object, finds the nth token in the object
	 */
	const JsonParserGeneratorRK::jsmntok_t *getTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t desiredIndex) const;

	/**
	 * Given a JSON object in container, gets the key/value pair specified by index.
	 *
	 * This is a low-level function; you will typically use getValueByIndex() or getValueByKey() instead.
	 */
	bool getKeyValueTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, const JsonParserGeneratorRK::jsmntok_t *&key, const JsonParserGeneratorRK::jsmntok_t *&value, size_t index) const;


	/**
	 * Used internally to skip over the token in obj.
	 *
	 * For simple primitives and strings, this is equivalent to obj++. For objects and arrays,
	 * however, this skips over the entire object or array, including any nested objects within
	 * them.
	 */
	bool skipObject(const JsonParserGeneratorRK::jsmntok_t *container, const JsonParserGeneratorRK::jsmntok_t *&obj) const;

	/**
	 * Copies the value of the token into a buffer, making it a null-terminated cstring.
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
	 * Gets a bool (boolean) value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a bool variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, bool &result) const;

	/**
	 * Gets an integer value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is an int variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, int &result) const;

	/**
	 * Gets an unsigned long value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is an unsigned long variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, unsigned long &result) const;

	/**
	 * Gets a float (single precision floating point) value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a float variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, float &result) const;

	/**
	 * Gets a double (double precision floating point) value.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a double variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, double &result) const;

	/**
	 * Gets a String value. This will automatically decode Unicode character escapes in the data and the
	 * returned String will contain UTF-8.
	 *
	 * Normally you'd use getValueByKey(), getValueByIndex() or getValueByColRow() which will automatically
	 * use this when the result parameter is a String variable.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, String &result) const;

	/**
	 * Gets a string as a cstring into the specified buffer. If the token specifies too large of a string
	 * it will be truncated. This will automatically decode Unicode character escapes in the data and the
	 * returned string will contain UTF-8.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, char *str, size_t &strLen) const;

	/**
	 * Gets a string as a JsonParserString object. This is used internally by getTokenValue() overloads
	 * that take a String or buffer and length; you will normally not need to use this directly.
	 *
	 * This will automatically decode Unicode character escapes in the data and the
	 * returned string will contain UTF-8.
	 */
	bool getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, JsonParserString &str) const;

	/**
	 * Given a Unicode UTF-16 code point, converts it to UTF-8 and appends it to str.
	 */
	static void appendUtf8(uint16_t unicode, JsonParserString &str);


protected:
	JsonParserGeneratorRK::jsmntok_t *tokens;
	JsonParserGeneratorRK::jsmntok_t *tokensEnd;
	size_t	maxTokens;
	JsonParserGeneratorRK::jsmn_parser parser;
};

/**
 * Creates a JsonParser with a static buffer. You normally use this when you're creating a parser
 * as a global variable.
 */
template <size_t BUFFER_SIZE, size_t MAX_TOKENS>
class JsonParserStatic : public JsonParser {
public:
	explicit JsonParserStatic() : JsonParser(staticBuffer, BUFFER_SIZE, staticTokens, MAX_TOKENS) {};

private:
	char staticBuffer[BUFFER_SIZE];
	JsonParserGeneratorRK::jsmntok_t staticTokens[MAX_TOKENS];
};

/**
 * Class for containing a reference to a JSON object, array, or value token
 *
 * This provides a fluent-style API for easily traversing a tree of JSON objects to find a value
 */
class JsonReference {
public:
	JsonReference(const JsonParser *parser);
	virtual ~JsonReference();

	JsonReference(const JsonParser *parser, const JsonParserGeneratorRK::jsmntok_t *token);

	JsonReference key(const char *name) const;
	JsonReference index(size_t index) const ;
	size_t size() const;

	template<class T>
	bool value(T &result) const {
		if (token && parser->getTokenValue(token, result)) {
			return true;
		}
		else {
			return false;
		}
	}

	bool valueBool(bool defaultValue = false) const;
	int valueInt(int defaultValue = 0) const;
	unsigned long valueUnsignedLong(unsigned long defaultValue = 0) const;
	float valueFloat(float defaultValue = 0) const;
	double valueDouble(double defaultValue = 0) const;
	String valueString() const;

private:
	const JsonParser *parser;
	const JsonParserGeneratorRK::jsmntok_t *token;
};

typedef struct {
	bool isFirst;
	char terminator;
} JsonWriterContext;

/**
 * Class for building a JSON string
 */
class JsonWriter : public JsonBuffer {
public:
	JsonWriter();
	virtual ~JsonWriter();

	JsonWriter(char *buffer, size_t bufferLen);

	/**
	 * You do not need to call init() as it's called from the two constructors. You can call it again
	 * if you want to reset the writer and reuse it, such as when you use JsonWriterStatic in a global
	 * variable.
	 */
	void init();

	/**
	 *
	 */
	bool startObject() { return startObjectOrArray('{', '}'); };
	bool startArray() { return startObjectOrArray('[', ']'); };
	void finishObjectOrArray();

	/**
	 * Inserts a boolean value ("true" or "false").
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(bool value);

	/**
	 * Inserts an integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(int value) { insertsprintf("%d", value); }

	/**
	 * Inserts an unsigned integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(unsigned int value) { insertsprintf("%u", value); }

	/**
	 * Inserts a long integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(long value) { insertsprintf("%ld", value); }

	/**
	 * Inserts an unsigned long integer value.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separators between items.
	 */
	void insertValue(unsigned long value) { insertsprintf("%lu", value); }

	/**
	 * Inserts a floating point value.
	 *
	 * Use setFloatPlaces() to set the number of decimal places to include.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(float value);

	/**
	 * Inserts a floating point double value.
	 *
	 * Use setFloatPlaces() to set the number of decimal places to include.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(double value);

	/**
	 * Inserts a quoted string value. This escapes special characters and encodes utf-8.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(const char *value) { insertString(value, true); }

	/**
	 * Inserts a quoted string value. This escapes special characters and encodes utf-8.
	 * See also the version that takes a plain const char *.
	 *
	 * You would normally use insertKeyValue() or insertArrayValue() instead of calling this directly
	 * as those functions take care of inserting the separtators between items.
	 */
	void insertValue(const String &value) { insertString(value.c_str(), true); }

	/**
	 * Inserts a new key and empty object. You must close the object using finishObjectOrArray()!
	 */
	void insertKeyObject(const char *key);

	/**
	 * Inserts a new key and empty array. You must close the object using finishObjectOrArray()!
	 */
	void insertKeyArray(const char *key);

	/**
	 * Inserts a key/value pair into an object.
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
	 * Inserts a value into an array.
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
	 * If you try to insert more data than will fit in the buffer, the isTruncated flag will be
	 * set, and the buffer will likely not be valid JSON and should not be used.
	 */
	bool isTruncated() const { return truncated; }

	/**
	 * Sets the number of digits for formatting float and double values. Set it to -1 to use the
	 * default for snprintf.
	 */
	void setFloatPlaces(int floatPlaces) { this->floatPlaces = floatPlaces; }

	/**
	 * Check to see if a separator needs to be inserted. You normally don't need to use this
	 * as it's called by insertKeyValue() and insertArrayValue().
	 */
	void insertCheckSeparator();

	/**
	 * Used internally; you should use startObject() or startArray() instead.
	 * Make sure you finish any started object or array using finishObjectOrArray().
	 */
	bool startObjectOrArray(char startChar, char endChar);

	/**
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string instead.
	 */
	void insertChar(char ch);

	/**
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string instead.
	 */
	void insertString(const char *s, bool quoted = false);

	/**
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string, float, or
	 * double instead.
	 *
	 * This method does not quote or escape the string - it's used mainly for formatting numbers.
	 */
	void insertsprintf(const char *fmt, ...);

	/**
	 * Used internally. You should use insertKeyValue() or insertArrayValue() with a string, float, or
	 * double instead.
	 *
	 * This method does not quote or escape the string - it's used mainly for formatting numbers.
	 */
	void insertvsprintf(const char *fmt, va_list ap);

	/**
	 * This constant is the maximum number of nested objects that are supported; the actual number is
	 * one less than this so when set to 5 you can have four objects nested in each other.
	 */
	static const size_t MAX_NESTED_CONTEXT = 5;

protected:
	size_t contextIndex;
	JsonWriterContext context[MAX_NESTED_CONTEXT];
	bool truncated;
	int floatPlaces;
};


/**
 * Creates a JsonWriter with a statically allocated buffer. You typically do this when you want
 * to create a buffer as a global variable.
 */
template <size_t BUFFER_SIZE>
class JsonWriterStatic : public JsonWriter {
public:
	explicit JsonWriterStatic() : JsonWriter(staticBuffer, BUFFER_SIZE) {};

private:
	char staticBuffer[BUFFER_SIZE];
};

class JsonWriterAutoObject {
public:
	JsonWriterAutoObject(JsonWriter *jw) : jw(jw) {
		jw->startObject();
	}
	~JsonWriterAutoObject() {
		jw->finishObjectOrArray();
	}

protected:
	JsonWriter *jw;
};

class JsonWriterAutoArray {
public:
	JsonWriterAutoArray(JsonWriter *jw) : jw(jw) {
		jw->startArray();
	}
	~JsonWriterAutoArray() {
		jw->finishObjectOrArray();
	}

protected:
	JsonWriter *jw;
};

#endif /* __JSONPARSERGENERATORRK_H */

