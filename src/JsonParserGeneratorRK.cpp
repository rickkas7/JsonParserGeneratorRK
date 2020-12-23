
#include "Particle.h"
#include "JsonParserGeneratorRK.h"


JsonBuffer::JsonBuffer()  : buffer(0), bufferLen(0), offset(0), staticBuffers(false) {

}
JsonBuffer::~JsonBuffer() {
	if (!staticBuffers && buffer) {
		free(buffer);
	}
}

JsonBuffer::JsonBuffer(char *buffer, size_t bufferLen)  : buffer(buffer), bufferLen(bufferLen), offset(0), staticBuffers(true) {

}

void JsonBuffer::setBuffer(char *buffer, size_t bufferLen) {
	this->buffer = buffer;
	this->bufferLen = bufferLen;
	this->staticBuffers = true;
}

bool JsonBuffer::allocate(size_t len) {
	if (!staticBuffers) {
		char *newBuffer;
		if (buffer) {
			newBuffer = (char *) realloc(buffer, len);
		}
		else {
			newBuffer = (char *) malloc(len);
		}
		if (newBuffer) {
			buffer = newBuffer;
			bufferLen = len;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool JsonBuffer::addData(const char *data, size_t dataLen) {
	if (!buffer || (offset + dataLen) > bufferLen) {
		// Need to allocate more space for data
		if (!allocate(offset + dataLen)) {
			return false;
		}
	}

	memcpy(&buffer[offset], data, dataLen);
	offset += dataLen;

	return true;
}

bool JsonBuffer::addChunkedData(const char *event, const char *data, size_t chunkSize) {

	// Multipart hook-response events end in /0, /1, ... 
	int responseIndex = 0;
	const char *slashOffset = strrchr(event, '/');
	if (slashOffset) {
		responseIndex = atoi(slashOffset + 1);
	}

	// Note: We don't clear on /0 (responseIndex == 0) here, because an out-of-order
	// response of /1 then /0 would cause the /1 chunk to be discarded. You need to
	// clear after a successful parse, not on /0.

	size_t len = strlen(data);

	// Assumption: len will be chunkSize (512 bytes), except for the last chunk which
	// can be smaller.
	size_t curOffset = responseIndex * chunkSize;
	if (!buffer || (curOffset + len) > bufferLen) {
		// Need to allocate more space for data
		if (!allocate(curOffset + len)) {
			return false;
		}
	}

	memcpy(&buffer[curOffset], data, len);

	curOffset += len;
	if (curOffset > offset) {
		offset = curOffset;
	}

	return true;
}


void JsonBuffer::clear() {
	offset = 0;
	if (buffer && bufferLen) {
		memset(buffer, 0, bufferLen);
	}
}

void JsonBuffer::nullTerminate() {
	if (buffer) {
		if (offset < bufferLen) {
			buffer[offset] = 0;
		}
		else {
			buffer[bufferLen - 1] = 0;
		}
	}
}


//

JsonParser::JsonParser() : JsonBuffer(), tokens(0), tokensEnd(0), maxTokens(0) {
}

JsonParser::JsonParser(char *buffer, size_t bufferLen, JsonParserGeneratorRK::jsmntok_t *tokens, size_t maxTokens) :
		JsonBuffer(buffer, bufferLen), tokens(tokens), maxTokens(maxTokens) {

}


JsonParser::~JsonParser() {
	if (!staticBuffers && tokens) {
		free(tokens);
	}
}

bool JsonParser::allocateTokens(size_t maxTokens) {
	if (!staticBuffers) {
		JsonParserGeneratorRK::jsmntok_t *newTokens;
		if (tokens) {
			newTokens = (JsonParserGeneratorRK::jsmntok_t *)realloc(buffer, sizeof(JsonParserGeneratorRK::jsmntok_t) * maxTokens);
		}
		else {
			newTokens = (JsonParserGeneratorRK::jsmntok_t *)malloc(sizeof(JsonParserGeneratorRK::jsmntok_t) * maxTokens);
		}
		if (newTokens) {
			tokens = newTokens;
			this->maxTokens = maxTokens;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
}

bool JsonParser::parse() {
	if (offset == 0) {
		// If addString or addData is not called, or called with an empty string,
		// do not return true, see issue #7.
		return false;
	}

	if (tokens) {
		// Try to use the existing token buffer if possible
		JsonParserGeneratorRK::jsmn_init(&parser);
		int result = JsonParserGeneratorRK::jsmn_parse(&parser, buffer, offset, tokens, maxTokens);
		if (result == JsonParserGeneratorRK::JSMN_ERROR_NOMEM) {
			if (staticBuffers) {
				// If using static buffers and there is not enough space, fail
				return false;
			}
			free(tokens);
			tokens = 0;
			maxTokens = 0;
		}
		else
		if (result < 0) {
			// Failed to parse: JSMN_ERROR_INVAL or JSMN_ERROR_PART
			return false;
		}
		else {
			tokensEnd = &tokens[result];
			return true;
		}
	}

	// Pass 1: determine now many tokens we need
	JsonParserGeneratorRK::jsmn_init(&parser);
	int result = JsonParserGeneratorRK::jsmn_parse(&parser, buffer, offset, 0, 0);
	if (result < 0) {
		// Failed to parse: JSMN_ERROR_INVAL or JSMN_ERROR_PART
		return false;
	}

	// If we get here, tokens will always be == 0; it would have been freed if it was
	// too small, and this code is never executed for staticBuffers == true

	maxTokens = (size_t) result;
	if (maxTokens > 0) {
		tokens = (JsonParserGeneratorRK::jsmntok_t *)malloc(sizeof(JsonParserGeneratorRK::jsmntok_t) * maxTokens);

		JsonParserGeneratorRK::jsmn_init(&parser);
		int result = JsonParserGeneratorRK::jsmn_parse(&parser, buffer, offset, tokens, maxTokens);

		tokensEnd = &tokens[result];
	}
	else {
		tokensEnd = tokens;
	}

	/*
	for(const JsonParserGeneratorRK::jsmntok_t *token = tokens; token < tokensEnd; token++) {
		printf("%d, %d, %d, %d\n", token->type, token->start, token->end, token->size);
	}
	*/

	return true;
}

JsonReference JsonParser::getReference() const {

	if (tokens < tokensEnd) {
		return JsonReference(this, &tokens[0]);
	}
	else {
		return JsonReference(this);
	}
}

const JsonParserGeneratorRK::jsmntok_t *JsonParser::getOuterArray() const {
	for(const JsonParserGeneratorRK::jsmntok_t *token = tokens; token < tokensEnd; token++) {
		if (token->type == JsonParserGeneratorRK::JSMN_ARRAY) {
			return token;
		}
	}
	return 0;
}

const JsonParserGeneratorRK::jsmntok_t *JsonParser::getTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t desiredIndex) const {

	size_t index = 0;
	const JsonParserGeneratorRK::jsmntok_t *token = container + 1;

	while(token < tokensEnd && token->end < container->end) {
		if (desiredIndex == index) {
			return token;
		}
		index++;
		skipObject(container, token);
	}

	return 0;
}


const JsonParserGeneratorRK::jsmntok_t *JsonParser::getOuterObject() const {
	if (tokens < tokensEnd && tokens[0].type == JsonParserGeneratorRK::JSMN_OBJECT) {
		return &tokens[0];
	}
	else {
		return 0;
	}
}

const JsonParserGeneratorRK::jsmntok_t *JsonParser::getOuterToken() const {
	for(const JsonParserGeneratorRK::jsmntok_t *token = tokens; token < tokensEnd; token++) {
		if (token->type == JsonParserGeneratorRK::JSMN_OBJECT || token->type == JsonParserGeneratorRK::JSMN_ARRAY) {
			return token;
		}
	}
	return 0;
}


bool JsonParser::skipObject(const JsonParserGeneratorRK::jsmntok_t *container, const JsonParserGeneratorRK::jsmntok_t *&obj) const {
	int curObjectEnd = obj->end;

	while(++obj < tokensEnd && obj->end < container->end && obj->end <= curObjectEnd) {
	}

	if (obj >= tokensEnd || obj->end > container->end) {
		// No object after index
		return false;
	}
	return true;
}

bool JsonParser::getKeyValueTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, const JsonParserGeneratorRK::jsmntok_t *&key, const JsonParserGeneratorRK::jsmntok_t *&value, size_t desiredIndex) const {

	size_t index = 0;
	const JsonParserGeneratorRK::jsmntok_t *token = container + 1;

	while(token < tokensEnd && token->end < container->end) {
		if (desiredIndex == index) {
			key = token;
			if (skipObject(container, token)) {
				value = token;
				return true;
			}
		}
		index++;
		skipObject(container, token);
		skipObject(container, token);
	}

	return false;
}


bool JsonParser::getValueTokenByKey(const JsonParserGeneratorRK::jsmntok_t *container, const char *name, const JsonParserGeneratorRK::jsmntok_t *&value) const {

	const JsonParserGeneratorRK::jsmntok_t *key;
	String keyName;

	for(size_t ii = 0; getKeyValueTokenByIndex(container, key, value, ii); ii++) {
		if (getTokenValue(key, keyName) && keyName == name) {
			return true;
		}
	}
	return false;
}

bool JsonParser::getValueTokenByIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t desiredIndex, const JsonParserGeneratorRK::jsmntok_t *&value) const {
	size_t index = 0;
	const JsonParserGeneratorRK::jsmntok_t *token = container + 1;

	while(token < tokensEnd && token->end < container->end) {
		if (desiredIndex == index) {
			value = token;
			return true;
		}
		index++;
		skipObject(container, token);
	}

	return false;
}

bool JsonParser::getValueTokenByColRow(const JsonParserGeneratorRK::jsmntok_t *container, size_t col, size_t row, const JsonParserGeneratorRK::jsmntok_t *&value) const {

	const JsonParserGeneratorRK::jsmntok_t *columnContainer;

	if (getValueTokenByIndex(container, col, columnContainer)) {
		return getValueTokenByIndex(columnContainer, row, value);
	}
	else {
		return false;
	}
}



size_t JsonParser::getArraySize(const JsonParserGeneratorRK::jsmntok_t *arrayContainer) const {
	size_t index = 0;
	const JsonParserGeneratorRK::jsmntok_t *token = arrayContainer + 1;

	while(token < tokensEnd && token->end < arrayContainer->end) {
		index++;
		skipObject(arrayContainer, token);
	}

	return index;
}

void JsonParser::copyTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, char *dst, size_t dstLen) const {

	int ii;
	for(ii = 0; ii < (token->end - token->start) && ii < ((int)dstLen - 1); ii++) {
		dst[ii] = buffer[token->start + ii];
 	}
	dst[ii] = 0;
}



bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, bool &result) const {
	if (token->end > token->start) {
		switch(buffer[token->start]) {
		case 't': // should be this
		case 'T':
		case 'y':
		case 'Y':
		case '1':
			result = true;
			break;

		default:
			result = false;
			break;
		}
		return true;
	}
	else {
		return false;
	}
}

bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, int &result) const {
	// Copy data here, because tokens are not null terminated
	char tmp[16];
	copyTokenValue(token, tmp, sizeof(tmp));

	if (sscanf(tmp, "%d", &result) == 1) {
		return true;
	}
	else {
		return false;
	}
}

bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, unsigned long &result) const {
	// Copy data here, because tokens are not null terminated
	char tmp[16];
	copyTokenValue(token, tmp, sizeof(tmp));

	if (sscanf(tmp, "%lu", &result) == 1) {
		return true;
	}
	else {
		return false;
	}
}



bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, float &result) const {
	// Copy data here, because tokens are not null terminated
	char tmp[16];
	copyTokenValue(token, tmp, sizeof(tmp));

	result = strtof(tmp, 0);
	return true;
}

bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, double &result) const {
	// Copy data here, because tokens are not null terminated
	char tmp[16];
	copyTokenValue(token, tmp, sizeof(tmp));

	result = strtod(tmp, 0);
	return true;
}



bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, String &result) const {

	result = "";
	result.reserve(token->end - token->start + 1);

	JsonParserString strWrapper(&result);
	return getTokenValue(token, strWrapper);
}

bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, char *str, size_t &bufLen) const {
	JsonParserString strWrapper(str, bufLen);
	bool result = getTokenValue(token, strWrapper);
	bufLen = strWrapper.getLength() + 1;
	return result;
}


bool JsonParser::getTokenValue(const JsonParserGeneratorRK::jsmntok_t *token, JsonParserString &str) const {

	int unicode;
	bool escape = false;

	for(int ii = token->start; ii < token->end; ii++) {
		if (escape) {
			escape = false;
			switch(buffer[ii]) {
			case 'b':
				str.append('\b');
				break;

			case 'f':
				str.append('\f');
				break;

			case 'n':
				str.append('\n');
				break;

			case 'r':
				str.append('\r');
				break;

			case 't':
				str.append('\t');
				break;

			case 'u':
				if ((ii + 4) < token->end) {
					// Copy data here because buffer is not null terminated and this could
					// read past the end otherwise
					char tmp[5];
					for(size_t jj = 0; jj < 4; jj++) {
						tmp[jj] = buffer[ii + jj + 1];
					}
					tmp[4] = 0;
					if (sscanf(tmp, "%04x", &unicode) == 1) {
						appendUtf8((uint16_t)unicode, str);
						ii += 5; // also increments in loop
					}
				}
				break;

			default:
				str.append(buffer[ii]);
				break;
			}
		}
		else
		if (buffer[ii] == '\\') {
			escape = true;
		}
		else {
			str.append(buffer[ii]);
		}
	}

	return true;
}

bool JsonParser::getTokenJsonString(const JsonParserGeneratorRK::jsmntok_t *token, String &result) const {
	result = "";
	result.reserve(token->end - token->start + 3);

	JsonParserString strWrapper(&result);
	return getTokenJsonString(token, strWrapper);
}

bool JsonParser::getTokenJsonString(const JsonParserGeneratorRK::jsmntok_t *token, char *str, size_t &bufLen) const {
	JsonParserString strWrapper(str, bufLen);
	bool result = getTokenJsonString(token, strWrapper);
	bufLen = strWrapper.getLength() + 1;
	return result;
}

bool JsonParser::getTokenJsonString(const JsonParserGeneratorRK::jsmntok_t *token, JsonParserString &str) const {
	str.append(&buffer[token->start], token->end - token->start);
	return true;
}


// [static]
void JsonParser::appendUtf8(uint16_t unicode, JsonParserString &str) {

	unsigned char value;

	if (unicode <= 0x007f) {
		// 0x00000000 - 0x0000007F:
		str.append((char)unicode);
	}
	else
	if (unicode <= 0x7ff) {
		// 0x00000080 - 0x000007FF:
		// 110xxxxx 10xxxxxx

		value = (0b11000000 | ((unicode >> 6) & 0b11111));
		str.append((char)value);

		value = (0b10000000 | (unicode & 0b111111));
		str.append((char)value);
	}
	else {
		// 0x00000800 - 0x0000FFFF:
		// 1110xxxx 10xxxxxx 10xxxxxx
		value = 0b11100000 | ((unicode >> 12) & 0b1111);
		str.append((char)value);

		value = 0b10000000 | ((unicode >> 6) & 0b111111);
		str.append((char)value);

		value = 0b10000000 | (unicode & 0b111111);
		str.append((char)value);
	}
}

//
//
//

JsonReference::JsonReference(const JsonParser *parser) : parser(parser), token(0) {

}

JsonReference::~JsonReference() {
}

JsonReference::JsonReference(const JsonParser *parser, const JsonParserGeneratorRK::jsmntok_t *token) : parser(parser), token(token) {
}

JsonReference JsonReference::key(const char *name) const {
	const JsonParserGeneratorRK::jsmntok_t *newToken;

	if (token && parser->getValueTokenByKey(token, name, newToken)) {
		return JsonReference(parser, newToken);
	}
	else {
		return JsonReference(parser);
	}
}

JsonReference JsonReference::index(size_t index) const {
	const JsonParserGeneratorRK::jsmntok_t *newToken;

	if (token && parser->getValueTokenByIndex(token, index, newToken)) {
		return JsonReference(parser, newToken);
	}
	else {
		return JsonReference(parser);
	}
}

size_t JsonReference::size() const {
	if (token) {
		return parser->getArraySize(token);
	}
	else {
		return 0;
	}
}

bool JsonReference::valueBool(bool result) const {
	(void) value(result);
	return result;
}

int JsonReference::valueInt(int result) const {
	(void) value(result);
	return result;
}

unsigned long JsonReference::valueUnsignedLong(unsigned long result) const {
	(void) value(result);
	return result;
}

float JsonReference::valueFloat(float result) const {
	(void) value(result);
	return result;
}

double JsonReference::valueDouble(double result) const {
	(void) value(result);
	return result;
}

String JsonReference::valueString() const {
	String result;

	(void) value(result);
	return result;
}


//
//
//
JsonParserString::JsonParserString(String *str) : str(str), buf(0), bufLen(0), length(0){
}

JsonParserString::JsonParserString(char *buf, size_t bufLen) : str(0), buf(buf), bufLen(bufLen), length(0){
	if (buf && bufLen) {
		memset(buf, 0, bufLen);
	}
}

void JsonParserString::append(char ch) {
	if (str) {
		str->concat(ch);
		length++;
	}
	else {
		if (buf && length < (bufLen - 1)) {
			buf[length] = ch;
		}
		length++;
	}
}

void JsonParserString::append(const char *str, size_t len) {
	for(size_t ii = 0; ii < len; ii++) {
		append(str[ii]);
	}
}


//
//
//
JsonWriter::JsonWriter() : JsonBuffer(), floatPlaces(-1) {
	init();
}

JsonWriter::~JsonWriter() {

}

JsonWriter::JsonWriter(char *buffer, size_t bufferLen) : JsonBuffer(buffer, bufferLen), floatPlaces(-1) {
	init();
}

void JsonWriter::init() {
	// Save start of insertion point for later
	offset = 0;

	contextIndex = 0;
	context[contextIndex].isFirst = true;
	context[contextIndex].terminator = 0;

	truncated = false;

}

bool JsonWriter::startObjectOrArray(char startChar, char endChar) {
	if ((contextIndex + 1) >= MAX_NESTED_CONTEXT) {
		return false;
	}
	insertCheckSeparator();

	contextIndex++;

	context[contextIndex].isFirst = true;
	context[contextIndex].terminator = endChar;

	insertChar(startChar);
	return true;
}


void JsonWriter::finishObjectOrArray() {
	if (contextIndex > 0) {
		if (context[contextIndex].terminator != 0) {
			insertChar(context[contextIndex].terminator);
		}
		contextIndex--;
	}
	// Make sure buffer is null terminated
	if (offset < bufferLen) {
		buffer[offset] = 0;
	}
	else {
		buffer[bufferLen - 1] = 0;
	}
}


void JsonWriter::insertChar(char ch) {
	if (offset < bufferLen) {
		buffer[offset++] = ch;
	}
	else {
		truncated = true;
	}
}

void JsonWriter::insertString(const char *s, bool quoted) {
	// 0x00000000 - 0x0000007F:

	// 0x00000080 - 0x000007FF:
	// 110xxxxx 10xxxxxx

	// 0x00000800 - 0x0000FFFF:
	// 1110xxxx 10xxxxxx 10xxxxxx

	if (quoted) {
		insertChar('"');
	}

	for(size_t ii = 0; s[ii] && offset < bufferLen; ii++) {
		if (s[ii] & 0x80) {
			// High bit set: convert UTF-8 to JSON Unicode escape
			if (((s[ii] & 0b11110000) == 0b11100000) && ((s[ii+1] & 0b11000000) == 0b10000000) && ((s[ii+2] & 0b11000000) == 0b10000000)) {
				// 3-byte
				uint16_t utf16 = ((s[ii] & 0b1111) << 12) | ((s[ii+1] & 0b111111) << 6) | (s[ii+2] & 0b111111);
				insertsprintf("\\u%04X", utf16);
				ii += 2; // plus one more in loop increment
			}
			else
			if (((s[ii] & 0b11100000) == 0b11000000) && ((s[ii+1] & 0b11000000) == 0b10000000)) {
				// 2-byte
				uint16_t utf16 = ((s[ii] & 0b11111) << 6) | (s[ii+1] & 0b111111);
				insertsprintf("\\u%04X", utf16);
				ii++; // plus one more in loop increment
			}
			else {
				// Not valid unicode, just pass characters through
				insertChar(s[ii]);
			}
		}
		else {
			switch(s[ii]) {
			case '\b':
				insertChar('\\');
				insertChar('b');
				break;

			case '\f':
				insertChar('\\');
				insertChar('f');
				break;

			case '\n':
				insertChar('\\');
				insertChar('n');
				break;

			case '\r':
				insertChar('\\');
				insertChar('r');
				break;

			case '\t':
				insertChar('\\');
				insertChar('t');
				break;

			case '"':
			case '\\':
				insertChar('\\');
				insertChar(s[ii]);
				break;

			default:
				insertChar(s[ii]);
				break;
			}
		}
	}
	if (quoted) {
		insertChar('"');
	}

}



void JsonWriter::insertsprintf(const char *fmt, ...) {
	va_list ap;
	va_start(ap, fmt);
	insertvsprintf(fmt, ap);
	va_end(ap);
}

void JsonWriter::insertvsprintf(const char *fmt, va_list ap) {
	size_t spaceAvailable = bufferLen - offset;

	size_t count = vsnprintf(&buffer[offset], spaceAvailable, fmt, ap);
	if (count <= spaceAvailable) {
		offset += count;
	}
	else {
		// Truncated, no more space left
		offset = bufferLen;
		truncated = true;
	}
}

void JsonWriter::insertCheckSeparator() {
	if (context[contextIndex].isFirst) {
		context[contextIndex].isFirst = false;
	}
	else {
		insertChar(',');
	}
}

void JsonWriter::insertValue(bool value) {
	if (value) {
		insertString("true");
	}
	else {
		insertString("false");
	}
}

void JsonWriter::insertValue(float value) {
	if (floatPlaces >= 0) {
		insertsprintf("%.*f", floatPlaces, value);
	}
	else {
		insertsprintf("%f", value);
	}
}
void JsonWriter::insertValue(double value) {
	if (floatPlaces >= 0) {
		insertsprintf("%.*lf", floatPlaces, value);
	}
	else {
		insertsprintf("%lf", value);
	}
}


void JsonWriter::insertKeyObject(const char *key) {
	insertCheckSeparator();
	insertValue(key);
	insertChar(':');
	setIsFirst();
	startObject();
}

void JsonWriter::insertKeyArray(const char *key) {
	insertCheckSeparator();
	insertValue(key);
	insertChar(':');
	setIsFirst();
	startArray();
}

void JsonWriter::setIsFirst(bool isFirst /* = true */) {
	context[contextIndex].isFirst = isFirst;
}



JsonModifier::JsonModifier(JsonParser &jp) : jp(jp) {

}

JsonModifier::~JsonModifier() {

}


bool JsonModifier::removeKeyValue(const JsonParserGeneratorRK::jsmntok_t *container, const char *key) {


	const JsonParserGeneratorRK::jsmntok_t *keyToken, *valueToken;

	bool bResult = jp.getValueTokenByKey(container, key, valueToken);
	if (!bResult) {
		return false;
	}

	// The key token always proceeds the value token
	keyToken = &valueToken[-1];

	// Include the double quotes
	const JsonParserGeneratorRK::jsmntok_t expandedKeyToken = tokenWithQuotes(keyToken);
	const JsonParserGeneratorRK::jsmntok_t expandedValueToken = tokenWithQuotes(valueToken);

	int left = findLeftComma(keyToken);
	int right = findRightComma(valueToken);

	if (left >= 0 && right >= 0) {
		// Commas on both sides, just remove the one on the right
		left = expandedKeyToken.start;
		right++;
	}
	else
	if (left >= 0) {
		// Only comma on left, removing the last item so remove the comma
		right = expandedValueToken.end;
	}
	else
	if (right >= 0) {
		// Only comma on the right, removing the first item so remove the comma
		right++;
		left = expandedKeyToken.start;
	}
	else {
		// Single item, leaving array empty afterwards (both < 0), no commads
		left = expandedKeyToken.start;
		right = expandedValueToken.end;
	}

	origAfter = jp.getOffset() - right;

	if (origAfter > 0) {
		memmove(jp.getBuffer() + left, jp.getBuffer() + right, origAfter);
	}

	jp.setOffset(left + origAfter);
	jp.parse();

	return true;
}

bool JsonModifier::removeArrayIndex(const JsonParserGeneratorRK::jsmntok_t *container, size_t index) {

	const JsonParserGeneratorRK::jsmntok_t *tok = jp.getTokenByIndex(container, index);
	if (!tok) {
		return false;
	}

	const JsonParserGeneratorRK::jsmntok_t expandedToken = tokenWithQuotes(tok);

	int left = findLeftComma(tok);
	int right = findRightComma(tok);

	if (left >= 0 && right >= 0) {
		// Commas on both sides, just remove the one on the right
		left = expandedToken.start;
	}
	else
	if (left >= 0) {
		// Only comma on left, removing the last item so remove the comma
		right = expandedToken.end;
	}
	else
	if (right >= 0) {
		// Only comma on the right, removing the first item so remove the comma
		right++;
		left = expandedToken.start;
	}
	else {
		// Single item, leaving array empty afterwards (both < 0), no commads
		left = expandedToken.start;
		right = expandedToken.end;
	}

	origAfter = jp.getOffset() - right;

	if (origAfter > 0) {
		memmove(jp.getBuffer() + left, jp.getBuffer() + right, origAfter);
	}

	jp.setOffset(left + origAfter);
	jp.parse();

	return true;
}
bool JsonModifier::startModify(const JsonParserGeneratorRK::jsmntok_t *token) {
	if (start != -1) {
		// Modification or insertion already in progress
		return false;
	}
	start = token->start;
	origAfter = jp.getOffset() - token->end;
	saveLoc = jp.getBufferLen() - origAfter;

	//printf("start=%d origAfter=%d saveLoc=%d bufferSize=%d\n", start, origAfter, saveLoc, saveLoc - start);

	if (origAfter > 0) {
		memmove(jp.getBuffer() + saveLoc, jp.getBuffer() + token->end, origAfter);
	}

	setBuffer(jp.getBuffer() + start, saveLoc - start);
	init();

	return true;
}

bool JsonModifier::startAppend(const JsonParserGeneratorRK::jsmntok_t *arrayOrObjectToken) {
	if (start != -1) {
		// Modification or insertion already in progress
		return false;
	}

	start = arrayOrObjectToken->end - 1; // Before the closing ] or }
	origAfter = jp.getOffset() - start;
	saveLoc = jp.getBufferLen() - origAfter;

	if (origAfter > 0) {
		memmove(jp.getBuffer() + saveLoc, jp.getBuffer() + start, origAfter);
	}

	setBuffer(jp.getBuffer() + start, saveLoc - start);
	init();

	// If array is not empty, add a separator
	setIsFirst(arrayOrObjectToken->size == 0);

	return true;
}


void JsonModifier::finish() {
	if (start == -1) {
		return;
	}
	//printf("finishing offset=%d\n", getOffset());

	if (origAfter > 0) {
		memmove(jp.getBuffer() + start + getOffset(), jp.getBuffer() + saveLoc, origAfter);
	}
	jp.setOffset(start + getOffset() + origAfter);
	jp.parse();
	start = -1;
}


JsonParserGeneratorRK::jsmntok_t JsonModifier::tokenWithQuotes(const JsonParserGeneratorRK::jsmntok_t *tok) const {
	JsonParserGeneratorRK::jsmntok_t expandedToken = *tok;

	if (tok->type == JsonParserGeneratorRK::JSMN_STRING) {
		expandedToken.start--;
		expandedToken.end++;
	}
	return expandedToken;
}

int JsonModifier::findLeftComma(const JsonParserGeneratorRK::jsmntok_t *tok) const {

	JsonParserGeneratorRK::jsmntok_t expandedToken = tokenWithQuotes(tok);

	int ii = expandedToken.start - 1;
	while(ii >= 0 && jp.getBuffer()[ii] == ' ') {
		// Whitespace
		ii--;
	}
	// printf("after whitespace check ii=%d c=%c\n", ii, jp.getBuffer()[ii]);

	if (ii < 0 || jp.getBuffer()[ii] != ',') {
		return -1;
	}


	return ii;
}

int JsonModifier::findRightComma(const JsonParserGeneratorRK::jsmntok_t *tok) const {
	JsonParserGeneratorRK::jsmntok_t expandedToken = tokenWithQuotes(tok);

	int ii = expandedToken.end;
	while(ii < jp.getOffset() && jp.getBuffer()[ii] == ' ') {
		// Whitespace
		ii++;
	}

	if (ii < 0 || jp.getBuffer()[ii] != ',') {
		return -1;
	}

	return ii;
}



// begin jsmn.cpp
// https://github.com/zserge/jsmn
namespace JsonParserGeneratorRK {

/**
 * Allocates a fresh unused token from the token pull.
 */
static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser,
		jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *tok;
	if (parser->toknext >= num_tokens) {
		return NULL;
	}
	tok = &tokens[parser->toknext++];
	tok->start = tok->end = -1;
	tok->size = 0;
#ifdef JSMN_PARENT_LINKS
	tok->parent = -1;
#endif
	return tok;
}

/**
 * Fills token type and boundaries.
 */
static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type,
                            int start, int end) {
	token->type = type;
	token->start = start;
	token->end = end;
	token->size = 0;
}

/**
 * Fills next available token with JSON primitive.
 */
static int jsmn_parse_primitive(jsmn_parser *parser, const char *js,
		size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;
	int start;

	start = parser->pos;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		switch (js[parser->pos]) {
#ifndef JSMN_STRICT
			/* In strict mode primitive must be followed by "," or "}" or "]" */
			case ':':
#endif
			case '\t' : case '\r' : case '\n' : case ' ' :
			case ','  : case ']'  : case '}' :
				goto found;
		}
		if (js[parser->pos] < 32 || js[parser->pos] >= 127) {
			parser->pos = start;
			return JSMN_ERROR_INVAL;
		}
	}
#ifdef JSMN_STRICT
	/* In strict mode primitive must be followed by a comma/object/array */
	parser->pos = start;
	return JSMN_ERROR_PART;
#endif

found:
	if (tokens == NULL) {
		parser->pos--;
		return 0;
	}
	token = jsmn_alloc_token(parser, tokens, num_tokens);
	if (token == NULL) {
		parser->pos = start;
		return JSMN_ERROR_NOMEM;
	}
	jsmn_fill_token(token, JSMN_PRIMITIVE, start, parser->pos);
#ifdef JSMN_PARENT_LINKS
	token->parent = parser->toksuper;
#endif
	parser->pos--;
	return 0;
}

/**
 * Fills next token with JSON string.
 */
static int jsmn_parse_string(jsmn_parser *parser, const char *js,
		size_t len, jsmntok_t *tokens, size_t num_tokens) {
	jsmntok_t *token;

	int start = parser->pos;

	parser->pos++;

	/* Skip starting quote */
	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c = js[parser->pos];

		/* Quote: end of string */
		if (c == '\"') {
			if (tokens == NULL) {
				return 0;
			}
			token = jsmn_alloc_token(parser, tokens, num_tokens);
			if (token == NULL) {
				parser->pos = start;
				return JSMN_ERROR_NOMEM;
			}
			jsmn_fill_token(token, JSMN_STRING, start+1, parser->pos);
#ifdef JSMN_PARENT_LINKS
			token->parent = parser->toksuper;
#endif
			return 0;
		}

		/* Backslash: Quoted symbol expected */
		if (c == '\\' && parser->pos + 1 < len) {
			int i;
			parser->pos++;
			switch (js[parser->pos]) {
				/* Allowed escaped symbols */
				case '\"': case '/' : case '\\' : case 'b' :
				case 'f' : case 'r' : case 'n'  : case 't' :
					break;
				/* Allows escaped symbol \uXXXX */
				case 'u':
					parser->pos++;
					for(i = 0; i < 4 && parser->pos < len && js[parser->pos] != '\0'; i++) {
						/* If it isn't a hex character we have an error */
						if(!((js[parser->pos] >= 48 && js[parser->pos] <= 57) || /* 0-9 */
									(js[parser->pos] >= 65 && js[parser->pos] <= 70) || /* A-F */
									(js[parser->pos] >= 97 && js[parser->pos] <= 102))) { /* a-f */
							parser->pos = start;
							return JSMN_ERROR_INVAL;
						}
						parser->pos++;
					}
					parser->pos--;
					break;
				/* Unexpected symbol */
				default:
					parser->pos = start;
					return JSMN_ERROR_INVAL;
			}
		}
	}
	parser->pos = start;
	return JSMN_ERROR_PART;
}

/**
 * Parse JSON string and fill tokens.
 */
int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
		jsmntok_t *tokens, unsigned int num_tokens) {
	int r;
	int i;
	jsmntok_t *token;
	int count = parser->toknext;

	for (; parser->pos < len && js[parser->pos] != '\0'; parser->pos++) {
		char c;
		jsmntype_t type;

		c = js[parser->pos];
		switch (c) {
			case '{': case '[':
				count++;
				if (tokens == NULL) {
					break;
				}
				token = jsmn_alloc_token(parser, tokens, num_tokens);
				if (token == NULL)
					return JSMN_ERROR_NOMEM;
				if (parser->toksuper != -1) {
					tokens[parser->toksuper].size++;
#ifdef JSMN_PARENT_LINKS
					token->parent = parser->toksuper;
#endif
				}
				token->type = (c == '{' ? JSMN_OBJECT : JSMN_ARRAY);
				token->start = parser->pos;
				parser->toksuper = parser->toknext - 1;
				break;
			case '}': case ']':
				if (tokens == NULL)
					break;
				type = (c == '}' ? JSMN_OBJECT : JSMN_ARRAY);
#ifdef JSMN_PARENT_LINKS
				if (parser->toknext < 1) {
					return JSMN_ERROR_INVAL;
				}
				token = &tokens[parser->toknext - 1];
				for (;;) {
					if (token->start != -1 && token->end == -1) {
						if (token->type != type) {
							return JSMN_ERROR_INVAL;
						}
						token->end = parser->pos + 1;
						parser->toksuper = token->parent;
						break;
					}
					if (token->parent == -1) {
						if(token->type != type || parser->toksuper == -1) {
							return JSMN_ERROR_INVAL;
						}
						break;
					}
					token = &tokens[token->parent];
				}
#else
				for (i = parser->toknext - 1; i >= 0; i--) {
					token = &tokens[i];
					if (token->start != -1 && token->end == -1) {
						if (token->type != type) {
							return JSMN_ERROR_INVAL;
						}
						parser->toksuper = -1;
						token->end = parser->pos + 1;
						break;
					}
				}
				/* Error if unmatched closing bracket */
				if (i == -1) return JSMN_ERROR_INVAL;
				for (; i >= 0; i--) {
					token = &tokens[i];
					if (token->start != -1 && token->end == -1) {
						parser->toksuper = i;
						break;
					}
				}
#endif
				break;
			case '\"':
				r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
				if (r < 0) return r;
				count++;
				if (parser->toksuper != -1 && tokens != NULL)
					tokens[parser->toksuper].size++;
				break;
			case '\t' : case '\r' : case '\n' : case ' ':
				break;
			case ':':
				parser->toksuper = parser->toknext - 1;
				break;
			case ',':
				if (tokens != NULL && parser->toksuper != -1 &&
						tokens[parser->toksuper].type != JSMN_ARRAY &&
						tokens[parser->toksuper].type != JSMN_OBJECT) {
#ifdef JSMN_PARENT_LINKS
					parser->toksuper = tokens[parser->toksuper].parent;
#else
					for (i = parser->toknext - 1; i >= 0; i--) {
						if (tokens[i].type == JSMN_ARRAY || tokens[i].type == JSMN_OBJECT) {
							if (tokens[i].start != -1 && tokens[i].end == -1) {
								parser->toksuper = i;
								break;
							}
						}
					}
#endif
				}
				break;
#ifdef JSMN_STRICT
			/* In strict mode primitives are: numbers and booleans */
			case '-': case '0': case '1' : case '2': case '3' : case '4':
			case '5': case '6': case '7' : case '8': case '9':
			case 't': case 'f': case 'n' :
				/* And they must not be keys of the object */
				if (tokens != NULL && parser->toksuper != -1) {
					jsmntok_t *t = &tokens[parser->toksuper];
					if (t->type == JSMN_OBJECT ||
							(t->type == JSMN_STRING && t->size != 0)) {
						return JSMN_ERROR_INVAL;
					}
				}
#else
			/* In non-strict mode every unquoted value is a primitive */
			default:
#endif
				r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
				if (r < 0) return r;
				count++;
				if (parser->toksuper != -1 && tokens != NULL)
					tokens[parser->toksuper].size++;
				break;

#ifdef JSMN_STRICT
			/* Unexpected char in strict mode */
			default:
				return JSMN_ERROR_INVAL;
#endif
		}
	}

	if (tokens != NULL) {
		for (i = parser->toknext - 1; i >= 0; i--) {
			/* Unmatched opened object or array */
			if (tokens[i].start != -1 && tokens[i].end == -1) {
				return JSMN_ERROR_PART;
			}
		}
	}

	return count;
}

/**
 * Creates a new parser based over a given  buffer with an array of tokens
 * available.
 */
void jsmn_init(jsmn_parser *parser) {
	parser->pos = 0;
	parser->toknext = 0;
	parser->toksuper = -1;
}
}

// end jsmn.cpp
