#include "Particle.h"
#include "JsonParserGeneratorRK.h"

void printTokens(JsonParser &jp);
void printToken(JsonParser &jp, const JsonParserGeneratorRK::jsmntok_t *tok);
void printJson(JsonParser &jp);

char *readTestData(const char *filename) {
	char *data;

	FILE *fd = fopen(filename, "r");
	if (!fd) {
		printf("failed to open %s", filename);
		return 0;
	}

	fseek(fd, 0, SEEK_END);
	size_t size = ftell(fd);
	fseek(fd, 0, SEEK_SET);

	data = (char *) malloc(size + 1);
	fread(data, 1, size, fd);
	data[size] = 0;

	fclose(fd);

	return data;
}

char chunkBuf[513];
char eventName[65];

bool getChunk(const char *data, size_t chunkIndex, const char *eventPrefix) {
	size_t len = strlen(data);

	size_t offset = chunkIndex * 512;
	if (offset >= len) {
		return false;
	}

	len -= offset;
	if (len > 512) {
		len = 512;
	}
	memcpy(chunkBuf, &data[offset], len);
	chunkBuf[len] = 0;

	snprintf(eventName, sizeof(eventName), "%s/%zu", eventPrefix, chunkIndex);

	return true;
}

void _assertJsonParserBuffer(JsonParser &jp, const char *expected, size_t line) {
	char *actual = (char *) malloc(jp.getOffset() + 1);
	strncpy(actual, jp.getBuffer(), jp.getOffset());
	actual[jp.getOffset()] = 0;

	size_t expectedLen = strlen(expected);
	if (expectedLen != jp.getOffset() || strcmp(actual, expected) != 0) {
		printf("line %lu: expectedLen=%lu actualLen=%lu\n", line, expectedLen, jp.getOffset());
		printf("expected: %s\n", expected);
		printf("actual:   %s\n", actual);
		assert(false);
	}
	free(actual);
}
#define assertJsonParserBuffer(jp, expected) _assertJsonParserBuffer(jp, expected, __LINE__)


void _assertJsonWriterBuffer(JsonWriter &jw, const char *expected, size_t line) {
	char *actual = (char *) malloc(jw.getOffset() + 1);
	strncpy(actual, jw.getBuffer(), jw.getOffset());
	actual[jw.getOffset()] = 0;

	size_t expectedLen = strlen(expected);
	if (expectedLen != jw.getOffset() || strcmp(actual, expected) != 0) {
		printf("line %lu: expectedLen=%lu actualLen=%lu\n", line, expectedLen, jw.getOffset());
		printf("expected: %s\n", expected);
		printf("actual:   %s\n", actual);
		assert(false);
	}
	free(actual);
}
#define assertJsonWriterBuffer(jw, expected) _assertJsonWriterBuffer(jw, expected, __LINE__)

// assert(jw.getOffset() == 14 && strncmp(jw.getBuffer(), "\\b\\f\\n\\r\\t\\\"\\", 8) == 0);

int main(int argc, char *argv[]) {

	{
		JsonParser jp;
		char *data2a = readTestData("test2a.json");
		jp.addData(data2a, strlen(data2a));
		free(data2a);

		jp.parse();

		String s;

		assert(jp.getValueByKey(jp.getOuterObject(), "range", s));
		assert(s == "Sheet1!A2:B7");

		assert(jp.getReference().key("range").valueString() == "Sheet1!A2:B7");

		assert(jp.getValueByKey(jp.getOuterObject(), "majorDimension", s));
		assert(s == "COLUMNS");

		assert(jp.getReference().key("majorDimension").valueString() == "COLUMNS");

		const JsonParserGeneratorRK::jsmntok_t *values;
		assert(jp.getValueTokenByKey(jp.getOuterObject(), "values", values));
		assert(jp.getArraySize(values) == 2);

		assert(jp.getReference().key("values").size() == 2);

		const JsonParserGeneratorRK::jsmntok_t *col[2];
		assert(jp.getValueTokenByIndex(values, 0, col[0]));
		assert(jp.getArraySize(col[0]) == 4);

		assert(jp.getValueTokenByIndex(values, 1, col[1]));
		assert(jp.getArraySize(col[1]) == 4);

		assert(jp.getReference().key("values").index(0).size() == 4);

		assert(jp.getValueByIndex(col[0], 0, s));
		assert(s == "Albert Albrecht");

		assert(jp.getReference().key("values").index(0).index(0).valueString() == "Albert Albrecht");

		assert(jp.getValueByIndex(col[0], 1, s));
		assert(s == "Bob Billings");

		assert(jp.getReference().key("values").index(0).index(1).valueString() == "Bob Billings");

		assert(jp.getValueByIndex(col[0], 2, s));
		assert(s == "Charlie Chaplin");

		assert(jp.getReference().key("values").index(0).index(2).valueString() == "Charlie Chaplin");

		assert(jp.getValueByIndex(col[0], 3, s));
		assert(s == "Dave Dink");

		assert(jp.getReference().key("values").index(0).index(3).valueString() == "Dave Dink");

		int intValue;
		assert(jp.getValueByIndex(col[1], 0, intValue));
		assert(intValue == 1234);

		assert(jp.getReference().key("values").index(1).index(0).valueInt() == 1234);

		assert(jp.getValueByIndex(col[1], 1, intValue));
		assert(intValue == 2234);

		assert(jp.getReference().key("values").index(1).index(1).valueInt() == 2234);

		assert(jp.getValueByIndex(col[1], 2, intValue));
		assert(intValue == 3234);

		assert(jp.getReference().key("values").index(1).index(2).valueInt() == 3234);

		assert(jp.getValueByIndex(col[1], 3, intValue));
		assert(intValue == 4234);

		assert(jp.getReference().key("values").index(1).index(3).valueInt() == 4234);

		//

		assert(jp.getValueByColRow(values, 0, 0, s));
		assert(s == "Albert Albrecht");

		assert(jp.getValueByColRow(values, 1, 0, intValue));
		assert(intValue == 1234);

		assert(jp.getValueByColRow(values, 0, 1, s));
		assert(s == "Bob Billings");

		assert(jp.getValueByColRow(values, 1, 1, intValue));
		assert(intValue == 2234);

		assert(jp.getValueByColRow(values, 1, 1, s));
		assert(s == "2234");

		// printJson(jp);
	}
	{
		JsonParser jp;

		char *data2b = readTestData("test2b.json");
		//printf("%s", data2b);

		jp.addData(data2b, strlen(data2b));
		free(data2b);

		jp.parse();

		const JsonParserGeneratorRK::jsmntok_t *key, *value;
		String s;
		int intValue;
		float floatValue;
		double doubleValue;
		bool boolValue;

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 0));
		//printf("key: start=%d end=%d\n", key->start, key->end);
		//printf("value: start=%d end=%d\n", value->start, value->end);
		assert(key->start == 5);
		assert(key->end == 7);
		assert(jp.getTokenValue(key, s));
		assert(s == "t1");
		assert(value->start == 10);
		assert(value->end == 13);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 1));
		assert(key->start == 19);
		assert(key->end == 21);
		assert(value->start == 23);
		assert(value->end == 27);
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getTokenValue(key, s));
		assert(s== "t2");

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t2", value));
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getValueByKey(jp.getOuterObject(), "t2", intValue));
		assert(intValue == 1234);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 2));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);
		assert(jp.getTokenValue(value, doubleValue));
		assert(doubleValue == 1234.5);

		assert(jp.getTokenValue(key, s));
		assert(s == "t3");

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", value));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);


		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 3));
		assert(jp.getTokenValue(key, s));
		assert(s == "t4");
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t4", value));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 4));
		assert(jp.getTokenValue(key, s));
		assert(s == "t5");
		assert(jp.getTokenValue(value, boolValue));
		assert(!boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 6));
		assert(jp.getTokenValue(key, s));
		assert(s == "t7");

		assert(jp.getTokenValue(value, s));
		assert(s == "\"quoted\"");

		assert(!jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 999));
		assert(!jp.getValueTokenByKey(jp.getOuterObject(), "xxx", value));


		//printJson(jp);


	}
	// Static buffer test
	{
		JsonParserStatic<256, 25> jp;

		char *data2b = readTestData("test2b.json");

		jp.addData(data2b, strlen(data2b));
		free(data2b);

		jp.parse();

		const JsonParserGeneratorRK::jsmntok_t *key, *value;
		char buf[256];
		size_t bufLen;
		int intValue;
		float floatValue;
		double doubleValue;
		bool boolValue;

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 0));
		//printf("key: start=%d end=%d\n", key->start, key->end);
		//printf("value: start=%d end=%d\n", value->start, value->end);
		assert(key->start == 5);
		assert(key->end == 7);
		assert(value->start == 10);
		assert(value->end == 13);
		bufLen = sizeof(buf);
		assert(jp.getTokenValue(value, buf, bufLen));
		assert(bufLen == 4); // includes null terminator
		assert(strcmp(buf, "abc") == 0);

		// Test exactly-sized buffer with null
		bufLen = 4;
		assert(jp.getTokenValue(value, buf, bufLen));
		assert(bufLen == 4);
		assert(strcmp(buf, "abc") == 0);

		// Test short buffer
		bufLen = 3;
		assert(jp.getTokenValue(value, buf, bufLen));
		assert(bufLen == 4);
		assert(strcmp(buf, "ab") == 0);

		// Test null buffer sizing
		assert(jp.getTokenValue(value, NULL, bufLen));
		assert(bufLen == 4);


		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 1));
		assert(key->start == 19);
		assert(key->end == 21);
		assert(value->start == 23);
		assert(value->end == 27);
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t2", value));
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getValueByKey(jp.getOuterObject(), "t2", intValue));
		assert(intValue == 1234);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 2));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);
		assert(jp.getTokenValue(value, doubleValue));
		assert(doubleValue == 1234.5);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", value));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);


		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 3));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t4", value));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 4));
		assert(jp.getTokenValue(value, boolValue));
		assert(!boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 6));

		bufLen = sizeof(buf);
		assert(jp.getTokenValue(value, buf, bufLen));
		assert(bufLen == 9); // includes null
		assert(strcmp(buf, "\"quoted\"") == 0);

		assert(!jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 999));
		assert(!jp.getValueTokenByKey(jp.getOuterObject(), "xxx", value));

	}

	// Static buffer too small
	{
		JsonParserStatic<50, 25> jp;

		char *data2b = readTestData("test2b.json");

		assert(!jp.addData(data2b, strlen(data2b)));

		free(data2b);
	}

	// Static buffer tokens too small
	{
		JsonParserStatic<256, 5> jp;

		char *data2b = readTestData("test2b.json");

		assert(jp.addData(data2b, strlen(data2b)));
		free(data2b);

		assert(!jp.parse());
	}

	// Unicode and some other odd data tests
	{
		JsonParser jp;
		char *data = readTestData("test2c.json");
		jp.addData(data, strlen(data));
		free(data);

		jp.parse();

		String s;
		int intValue;

		// UTF-8 test: "t1":"ab\"\u00A2c\u20AC"
		assert(jp.getValueByKey(jp.getOuterObject(), "t1", s));

		const char *sc = s;
		printf("%s\n", sc);

		const unsigned char *uc = (const unsigned char *)sc;

		size_t ii = 0;
		assert(sc[ii++] == 'a');
		assert(sc[ii++] == 'b');
		assert(sc[ii++] == '"');

		assert(uc[ii++] == 0xc2);
		assert(uc[ii++] == 0xa2);

		assert(uc[ii++] == 0xe2);
		assert(uc[ii++] == 0x82);
		assert(uc[ii++] == 0xac);


		const JsonParserGeneratorRK::jsmntok_t *t2obj;
		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t2", t2obj));

		assert(jp.getValueByKey(t2obj, "a", s));
		assert(s == "foo");

		const JsonParserGeneratorRK::jsmntok_t *t3obj;
		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", t3obj));


		assert(jp.getValueByIndex(t3obj, 0, intValue));
		assert(intValue == 1);

		assert(jp.getValueByIndex(t3obj, 1, intValue));
		assert(intValue == 2);

		assert(jp.getValueByIndex(t3obj, 2, intValue));
		assert(intValue == 3);

		assert(jp.getValueByIndex(t3obj, 2, s));
		assert(s == "3");

		// Using fluent API
		assert(jp.getReference().key("t2").key("a").valueString() == "foo");
		assert(jp.getReference().key("t3").index(0).valueInt() == 1);
		assert(jp.getReference().key("t3").index(1).valueInt() == 2);
		assert(jp.getReference().key("t3").index(2).valueInt() == 3);
		assert(jp.getReference().key("t3").index(2).valueUnsignedLong() == 3);
		assert(jp.getReference().key("t3").index(2).valueString() == "3");
		assert(jp.getReference().key("t3").index(2).valueFloat() == 3.0);
		assert(jp.getReference().key("t3").index(2).valueDouble() == 3.0);

	}

	// Test large data from OpenWeatherMap:
	// https://openweathermap.org/forecast5
	// http://api.openweathermap.org/data/2.5/forecast?id=524901&APPID=YOUR_API_KEY
	{
		JsonParser jp;
		char *data = readTestData("test2d.json");
		size_t dataLen = strlen(data);

		// Break it up into 255 byte segments like a subscription
		for(size_t offset = 0; offset < dataLen; offset += 255) {
			size_t count = dataLen - offset;
			if (count > 255) {
				count = 255;
			}
			jp.addData(&data[offset], count);
		}
		free(data);

		jp.parse();

		/*
		{"city":{"id":524901,"name":"Moscow","coord":{"lon":37.615555,"lat":55.75222},"country":"RU","population":0,"sys":{"population":0}},
			"cod":"200","message":0.0168,"cnt":40,
			"list":[{"dt":1478984400,"main":{"temp":267.67,"temp_min":267.324,"temp_max":267.67,"pressure":1010.4,"sea_level":1031.53,"grnd_level":1010.4,"humidity":90,"temp_kf":0.34},
				"weather":[{"id":600,"main":"Snow","description":"light snow","icon":"13n"}],"clouds":{"all":88},"wind":{"speed":4.93,"deg":278.007},"snow":{"3h":0.22125},"sys":{"pod":"n"},"dt_txt":"2016-11-12 21:00:00"},
			{"dt":1478995200,"main":{"temp":268.12,"temp_min":267.864
		 */
		String strValue;
		int intValue;
		float floatValue;

		const JsonParserGeneratorRK::jsmntok_t *cityToken; // is an object

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "city", cityToken));

		assert(jp.getValueByKey(cityToken, "id", intValue));
		assert(intValue == 524901);

		assert(jp.getValueByKey(cityToken, "name", strValue));
		assert(strValue == "Moscow");

		assert(jp.getValueByKey(cityToken, "country", strValue));
		assert(strValue == "RU");

		const JsonParserGeneratorRK::jsmntok_t *listToken; // is an array
		assert(jp.getValueTokenByKey(jp.getOuterObject(), "list", listToken));
		assert(jp.getArraySize(listToken) > 1);

		const JsonParserGeneratorRK::jsmntok_t *listObj; // is an object
		assert(jp.getValueTokenByIndex(listToken, 0, listObj));

		assert(jp.getValueByKey(listObj, "dt", intValue));
		assert(intValue == 1478984400);

		const JsonParserGeneratorRK::jsmntok_t *mainObj; // is an array
		assert(jp.getValueTokenByKey(listObj, "main", mainObj));

		assert(jp.getValueByKey(mainObj, "temp", floatValue));
		assert(floatValue >= 267.6 && floatValue <= 267.7); // 267.67

		// Second in the list
		assert(jp.getValueTokenByIndex(listToken, 1, listObj));

		assert(jp.getValueByKey(listObj, "dt", intValue));
		assert(intValue == 1478995200);

		// This is the last array element to make sure we've parsed correctly all the way to to the end
		// {"dt":1479405600,"main":{"temp":267.329,"temp_min":267.329,"temp_max":267.329,"pressure":1024.37,"sea_level":1045.84,"grnd_level":1024.37,"humidity":91,"temp_kf":0},"weather":[{"id":600,"main":"Snow","description":"light snow","icon":"13n"}],"clouds":{"all":80},"wind":{"speed":4.95,"deg":179.502},"snow":{"3h":0.105},"sys":{"pod":"n"},"dt_txt":"2016-11-17 18:00:00"}]}
		assert(jp.getValueTokenByIndex(listToken, jp.getArraySize(listToken) - 1, listObj));

		assert(jp.getValueByKey(listObj, "dt", intValue));
		assert(intValue == 1479405600);

		assert(jp.getValueByKey(listObj, "dt_txt", strValue));
		assert(strValue == "2016-11-17 18:00:00");


	}
	// Test large data from wunderground.com
	{
		JsonParser jp;
		char *data = readTestData("test2e.json");
		size_t dataLen = strlen(data);

		// Break it up into 255 byte segments like a subscription
		for(size_t offset = 0; offset < dataLen; offset += 255) {
			size_t count = dataLen - offset;
			if (count > 255) {
				count = 255;
			}
			jp.addData(&data[offset], count);
		}
		free(data);

		jp.parse();

		// Use the fluent-style API here
		assert(jp.getReference().key("response").key("version").valueString() == "0.1");

		assert(jp.getReference().key("forecast").key("txt_forecast").key("date").valueString() == "12:25 PM EST");
		assert(jp.getReference().key("forecast").key("txt_forecast").key("forecastday").key("period").valueInt() == 7);

	}

	// Test data from Github issue
	// https://github.com/rickkas7/JsonParserGeneratorRK/issues/1
	// {"A--":{"M":{"M":2,"U":5000,"T":10,"C":[0,255]}}}
	{
		JsonParser jp;
		String s;

		char *data = readTestData("test2f.json");

		jp.addString(data);
		free(data);

		bool bResult = jp.parse();
		assert(bResult);

		// Check fluent parser
		assert(jp.getReference().key("A--").key("M").key("U").valueInt() == 5000);

		// Check iteration
		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *keyToken;
			const JsonParserGeneratorRK::jsmntok_t *valueToken;
			String name;

			if (!jp.getKeyValueTokenByIndex(jp.getOuterObject(), keyToken, valueToken, ii)) {
				// Reached end
				assert(ii == 1);
				break;
			}

			bResult = jp.getTokenValue(keyToken, name);
			assert(bResult);

			assert(name == "A--");

			const JsonParserGeneratorRK::jsmntok_t *valueTokenInnerM;

			bResult = jp.getValueTokenByKey(valueToken, "M", valueTokenInnerM);
			assert(bResult);

			// Convert back to a JSON string
			bResult = jp.getTokenJsonString(valueTokenInnerM, s);
			assert(bResult);
			assert(s == "{\"M\":2,\"U\":5000,\"T\":10,\"C\":[0,255]}");

			// Parse inner
			for(size_t jj = 0; ; jj++) {
				const JsonParserGeneratorRK::jsmntok_t *keyTokenInner;
				const JsonParserGeneratorRK::jsmntok_t *valueTokenInner;

				if (!jp.getKeyValueTokenByIndex(valueTokenInnerM, keyTokenInner, valueTokenInner, jj)) {
					// Reached end
					break;
				}
				bResult = jp.getTokenValue(keyTokenInner, name);
				assert(bResult);

				int intValue;

				if (name == "M") {
					bResult = jp.getTokenValue(valueTokenInner, intValue);
					assert(bResult);
					assert(intValue == 2);
				}
				else
				if (name == "U") {
					bResult = jp.getTokenValue(valueTokenInner, intValue);
					assert(bResult);
					assert(intValue == 5000);

					// Convert back to a string
					bResult = jp.getTokenJsonString(valueTokenInner, s);
					assert(bResult);

					assert(s == "5000");
				}
				else
				if (name == "T") {
					bResult = jp.getTokenValue(valueTokenInner, intValue);
					assert(bResult);
					assert(intValue == 10);
				}
				else
				if (name == "C") {
					assert(jp.getArraySize(valueTokenInner) == 2);

					bResult = jp.getValueByIndex(valueTokenInner, 0, intValue);
					assert(bResult);
					assert(intValue == 0);

					bResult = jp.getValueByIndex(valueTokenInner, 1, intValue);
					assert(bResult);
					assert(intValue == 255);

					// Convert back to a string
					bResult = jp.getTokenJsonString(valueTokenInner, s);
					assert(bResult);

					assert(s == "[0,255]");
				}
				else {
					assert(0);
				}
			}

			// Test converting back to JSON
			// {"A--":{"M":{"M":2,"U":5000,"T":10,"C":[0,255]}}}
			String s;
			bResult = jp.getTokenJsonString(jp.getOuterToken(), s);
			assert(bResult);

			const char *expected = "{\"A--\":{\"M\":{\"M\":2,\"U\":5000,\"T\":10,\"C\":[0,255]}}}";

			assert(s == expected);

			char newJsonBuf[64];
			size_t bufLen = sizeof(newJsonBuf);
			bResult = jp.getTokenJsonString(jp.getOuterToken(), newJsonBuf, bufLen);
			assert(bResult);

			assert(bufLen = (strlen(expected) + 1));
			assert(strcmp(newJsonBuf, expected) == 0);

		}

		{
			// {"DID":"0000111122223333395abcd","NOM":"T","BV":"3.8","SOC":"3","PWR":"F","CHG":"T"}
			JsonParserStatic<256, 14> jp;
			String s;

			char *data = readTestData("test2g.json");

			jp.addString(data);
			free(data);

			bool bResult = jp.parse();
			assert(bResult);

			bResult = jp.getOuterValueByKey("DID", s);
			assert(bResult);
			assert(s == "0000111122223333395abcd");


			bResult = jp.getOuterValueByKey("NOM", s);
			assert(bResult);
			assert(s == "T");

			float f;
			bResult = jp.getOuterValueByKey("BV", f);
			assert(bResult);
			assert(f > 3.7 && f < 3.9);

			int i;
			bResult = jp.getOuterValueByKey("SOC", i);
			assert(bResult);
			assert(i == 3);

			bResult = jp.getOuterValueByKey("PWR", s);
			assert(bResult);
			assert(s == "F");

			bResult = jp.getOuterValueByKey("CHG", s);
			assert(bResult);
			assert(s == "T");

		}
	}

	// Calling parse on an empty buffer should return false
	// https://github.com/rickkas7/JsonParserGeneratorRK/issues/7
	{
		JsonParser jp;
		String s;

		jp.addString("");
		
		bool bResult = jp.parse();
		assert(!bResult);
	}

	{
		// https://community.particle.io/t/jsonparsergeneratorrk-parsing-a-child-key-from-a-firebase-get-webhook/56395
		// {"-M5sN1MfCHcXHkLBlwWW":{"aug":false,"fan":true,"ign":true}}
		JsonParserStatic<256, 14> jp;
		String s;

		char *data = readTestData("test2h.json");

		jp.addString(data);
		free(data);

		bool bResult = jp.parse();
		assert(bResult);

		const JsonParserGeneratorRK::jsmntok_t *firstObject;
		bResult = jp.getValueTokenByIndex(jp.getOuterObject(), 1, firstObject);
		assert(bResult);
		assert(firstObject != 0);

		bool bValue;

		bResult = jp.getValueByKey(firstObject, "aug", bValue);
		assert(bResult);
		assert(bValue == false);

		bResult = jp.getValueByKey(firstObject, "fan", bValue);
		assert(bResult);
		assert(bValue == true);

		bResult = jp.getValueByKey(firstObject, "ign", bValue);
		assert(bResult);
		assert(bValue == true);

	}

	// 
	// Chunked Tests
	//

	// Chunked single part (511 bytes)
	{
		JsonParserStatic<1024, 10> jp;
		bool bResult;

		char *data = readTestData("test3a.json");

		bResult = getChunk(data, 0, "hook-response/testEvent");
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		free(data);
	}
	// Chunked single part (512 bytes)
	{
		JsonParserStatic<1024, 10> jp;
		bool bResult;

		char *data = readTestData("test3b.json");

		bResult = getChunk(data, 0, "hook-response/testEvent");
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		free(data);
	}
	// Chunked multi part (513 bytes)
	{
		JsonParserStatic<1024, 10> jp;
		bool bResult;

		char *data = readTestData("test3d.json");
		
		const char *eventPrefix = "hook-response/testEvent";

		bResult = getChunk(data, 0, eventPrefix);
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = jp.parse(); // Should be false here
		assert(!bResult);

		bResult = getChunk(data, 1, eventPrefix);
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		free(data);
	}

	// Chunked multi part (513 bytes) - reversed order
	{
		JsonParserStatic<1024, 10> jp;
		bool bResult;

		char *data = readTestData("test3d.json");
		
		const char *eventPrefix = "hook-response/testEvent";

		bResult = getChunk(data, 1, eventPrefix);
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = jp.parse(); // Should be false here
		assert(!bResult);

		bResult = getChunk(data, 0, eventPrefix);
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		free(data);
	}
	// Chunked multi part (6023 bytes) - 12 chunks
	{
		JsonParserStatic<8192, 20> jp;
		bool bResult;

		char *data = readTestData("test3e.json");
		
		const char *eventPrefix = "hook-response/testEvent";

		for(size_t chunkIndex = 0; ; chunkIndex++) {
			bResult = getChunk(data, chunkIndex, eventPrefix);
			if (!bResult) {
				break;
			}

			bResult = jp.addChunkedData(eventName, chunkBuf);
			assert(bResult);
		}

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		free(data);
	}

	// Chunked multi part (6023 bytes) - 12 chunks out of order
	{
		JsonParserStatic<8192, 20> jp;
		bool bResult;

		char *data = readTestData("test3e.json");
		
		const char *eventPrefix = "hook-response/testEvent";

		const size_t ordering[12] = { 0, 1, 4, 5, 6, 7, 8, 9, 10, 11, 2, 3 };

		for(size_t chunkIndex = 0; chunkIndex < 12; chunkIndex++) {
			bResult = getChunk(data, ordering[chunkIndex], eventPrefix);
			assert(bResult);

			bResult = jp.addChunkedData(eventName, chunkBuf);
			assert(bResult);
		}

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		// Test buffer clearing
		jp.clear();

		bResult = getChunk(data, 0, eventPrefix);
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		bResult = getChunk(data, 11, eventPrefix);
		assert(bResult);

		bResult = jp.addChunkedData(eventName, chunkBuf);
		assert(bResult);

		// The previous data should have been cleared from the buffer so adding 
		// data at the end should still fail to parse
		bResult = jp.parse();
		assert(!bResult);

		free(data);
	}

	// Chunked multi part (6023 bytes) - 12 chunks, dynamic allocation
	{
		JsonParser jp;
		bool bResult;

		char *data = readTestData("test3e.json");
		
		const char *eventPrefix = "012345678901234567890123/hook-response/testEvent";

		for(size_t chunkIndex = 0; ; chunkIndex++) {
			bResult = getChunk(data, chunkIndex, eventPrefix);
			if (!bResult) {
				break;
			}

			bResult = jp.addChunkedData(eventName, chunkBuf);
			assert(bResult);
		}

		bResult = jp.parse();
		assert(bResult);

		assert(jp.getOffset() == strlen(data));
		assert(strncmp(jp.getBuffer(), data, strlen(data)) == 0);

		free(data);
	}	


	// Writer test, unallocated buffer
	{
		JsonWriter jw;
		jw.addString("abcdefgh");
		assertJsonWriterBuffer(jw, "abcdefgh");
	}
	// Writer test, small allocated buffer
	{
		JsonWriter jw;
		jw.allocate(6);
		jw.addString("abcdefgh");
		assertJsonWriterBuffer(jw, "abcdefgh");
	}
	// Writer test - sprintf
	{
		JsonWriterStatic<100> jw;

		jw.insertsprintf("%04x", 0x1234);
		assertJsonWriterBuffer(jw, "1234");
	}
	// Writer test - string insert Unicode
	{
		JsonWriterStatic<100> jw;

		char tmp[4];
		tmp[0] = 0xc2;
		tmp[1] = 0xa2;
		tmp[2] = 0;

		jw.insertString(tmp, true);
		assertJsonWriterBuffer(jw, "\"\\u00A2\"");
	}
	{
		JsonWriterStatic<100> jw;
		char tmp[4];
		tmp[0]= 0xe2;
		tmp[1]= 0x82;
		tmp[2] = 0xac;
		tmp[3] = 0;

		jw.insertString(tmp, false);
		assertJsonWriterBuffer(jw, "\\u20AC");
	}

	// Writer test - special chars
	{
		JsonWriterStatic<100> jw;

		jw.insertString("\b\f\n\r\t\"\\", false);
		assertJsonWriterBuffer(jw, "\\b\\f\\n\\r\\t\\\"\\\\");

	}

	// Writer test - simple array
	{
		JsonWriterStatic<256> jw;

		jw.startArray();

		jw.insertArrayValue(true);
		jw.insertArrayValue(1234);
		jw.insertArrayValue("test");

		jw.finishObjectOrArray();

		//printf("'%s'\n", jw.getBuffer());

		assertJsonWriterBuffer(jw, "[true,1234,\"test\"]");

	}

	// Writer test - simple object
	{
		JsonWriterStatic<256> jw;

		jw.startObject();

		jw.insertKeyValue("a", true);
		jw.insertKeyValue("b", 1234);
		jw.insertKeyValue("c", "test");

		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":true,\"b\":1234,\"c\":\"test\"}");

	}

	// Writer test - null termination test
	{
		char buf[256];
		memset(buf, 'x', sizeof(buf));
		JsonWriter jw(buf, sizeof(buf));

		jw.startObject();

		jw.insertKeyValue("a", true);
		jw.insertKeyValue("b", 1234);
		jw.insertKeyValue("c", "test");

		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":true,\"b\":1234,\"c\":\"test\"}");

	}

	// Writer test - nested
	{
		JsonWriterStatic<256> jw;

		jw.startObject();

		jw.insertKeyArray("a");
		jw.insertArrayValue(123);
		jw.insertArrayValue(456);
		jw.insertArrayValue(789);
		jw.finishObjectOrArray();

		jw.insertKeyObject("b");
		jw.insertKeyValue("ba", true);
		jw.insertKeyValue("bb", 1234);
		jw.finishObjectOrArray();

		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":[123,456,789],\"b\":{\"ba\":true,\"bb\":1234}}");

	}

	// Writer test - array of objects - low level API
	{
		JsonWriterStatic<256> jw;

		jw.startArray();

		for(int ii = 0; ii < 5; ii++) {
			// This used to be necessary, but now startObject takes care of this automatically
			// jw.insertCheckSeparator();
			jw.startObject();
			jw.insertKeyValue("ii", ii);

			jw.finishObjectOrArray();
		}

		jw.finishObjectOrArray();

		// printf("'%s'\n", jw.getBuffer());

		assertJsonWriterBuffer(jw, "[{\"ii\":0},{\"ii\":1},{\"ii\":2},{\"ii\":3},{\"ii\":4}]");

	}

	// Writer test - float places object
	{
		JsonWriterStatic<256> jw;

		jw.startObject();

		jw.setFloatPlaces(2);
		jw.insertKeyValue("a", 12.3333);

		jw.setFloatPlaces(0);
		jw.insertKeyValue("b", (double)12.77777777);

		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":12.33,\"b\":13}");

	}

	// Writer test - int array
	{
		JsonWriterStatic<256> jw;

		int array[3];
		array[0] = 1;
		array[1] = 2;
		array[2] = 3;

		jw.startObject();

		jw.setFloatPlaces(2);
		jw.insertKeyValue("a", "test");

		jw.insertKeyArray("b", array, sizeof(array)/sizeof(array[0]));
		
		// This closes the outer object
		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":\"test\",\"b\":[1,2,3]}");
	}

	// Writer test - float vector
	{
		JsonWriterStatic<256> jw;

		std::vector<float> vector;
		vector.push_back(1.1);
		vector.push_back(2.2);
		vector.push_back(3.333);

		jw.startObject();

		jw.setFloatPlaces(2);
		jw.insertKeyValue("a", "test");
		jw.insertKeyVector("b", vector);

		// This closes the outer object
		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":\"test\",\"b\":[1.10,2.20,3.33]}");

	}
 
	// Writer test - float vector #2
	{
		JsonWriterStatic<256> jw;

		std::vector<float> vector;
		vector.push_back(1.1);
		vector.push_back(2.2);
		vector.push_back(3.333);
		if (vector.size() > 2) {
			// Remove the first (oldest) element
			vector.erase(vector.begin());
		}

		jw.startObject();

		jw.setFloatPlaces(2);
		jw.insertKeyValue("a", "test");
		jw.insertKeyVector("b", vector);

		
		// This closes the outer object
		jw.finishObjectOrArray();

		assertJsonWriterBuffer(jw, "{\"a\":\"test\",\"b\":[2.20,3.33]}");

	}

	// Writer Test
	// https://community.particle.io/t/json-parser-and-generator-in-device-os/56147/7
	{
		JsonWriterStatic<256> jw;

		jw.startObject(); // Start outer object

		jw.setFloatPlaces(2);

		jw.insertKeyObject("f"); // Start inner object 
			jw.insertKeyValue("g", "Call me \"John\"");
			jw.insertKeyValue("h", -.78);
		jw.finishObjectOrArray(); // End inner object

		jw.finishObjectOrArray(); // End outer object

		// printf("%s\n", jw.getBuffer());	
		assertJsonWriterBuffer(jw, "{\"f\":{\"g\":\"Call me \\\"John\\\"\",\"h\":-0.78}}");
	}

/*
{
   "f":
       {
           "g":"Call me \"John\"",
           "h":-0.78
       }
}
*/
	// Modifier test - make string longer
	{
		JsonParserStatic<512, 32> jp;

		char *data2b = readTestData("test2b.json");
		//printf("%s", data2b);

		jp.addData(data2b, strlen(data2b));
		free(data2b);

		jp.parse();

		// printTokens(jp);
		const JsonParserGeneratorRK::jsmntok_t *key, *value;

		bool bResult = jp.getValueTokenByKey(jp.getOuterToken(), "t1", value);
		assert(bResult);

		JsonModifier mod(jp);
		bResult = mod.startModify(value);
		assert(bResult);

		mod.insertString("this is a test");
		mod.finish();

		// printf("after modify\n");
		// printTokens(jp);

		String s;
		int intValue;
		float floatValue;
		double doubleValue;
		bool boolValue;

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 0));
		//printf("key: start=%d end=%d\n", key->start, key->end);
		//printf("value: start=%d end=%d\n", value->start, value->end);
		assert(key->start == 5);
		assert(key->end == 7);
		assert(jp.getTokenValue(key, s));
		assert(s == "t1");

		assert(value->start == 10);
		assert(value->end == 24);
		assert(jp.getTokenValue(value, s));
		assert(s == "this is a test");

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 1));
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getTokenValue(key, s));
		assert(s== "t2");

		assert(jp.getValueByKey(jp.getOuterObject(), "t2", intValue));
		assert(intValue == 1234);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 2));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", value));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 3));
		assert(jp.getTokenValue(key, s));
		assert(s == "t4");
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t4", value));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 4));
		assert(jp.getTokenValue(key, s));
		assert(s == "t5");
		assert(jp.getTokenValue(value, boolValue));
		assert(!boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 6));
		assert(jp.getTokenValue(key, s));
		assert(s == "t7");

		assert(jp.getTokenValue(value, s));
		assert(s == "\"quoted\"");
	}


	// Modifier test - make string shorter
	{
		JsonParserStatic<512, 32> jp;

		char *data2b = readTestData("test2b.json");
		//printf("%s", data2b);

		jp.addData(data2b, strlen(data2b));
		free(data2b);

		jp.parse();

		// printTokens(jp);
		const JsonParserGeneratorRK::jsmntok_t *key, *value;

		bool bResult = jp.getValueTokenByKey(jp.getOuterToken(), "t1", value);
		assert(bResult);

		JsonModifier mod(jp);
		bResult = mod.startModify(value);
		assert(bResult);

		mod.insertString("x");
		mod.finish();

		// printf("after modify\n");
		// printTokens(jp);

		String s;
		int intValue;
		float floatValue;
		double doubleValue;
		bool boolValue;

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 0));
		//printf("key: start=%d end=%d\n", key->start, key->end);
		//printf("value: start=%d end=%d\n", value->start, value->end);
		assert(key->start == 5);
		assert(key->end == 7);
		assert(jp.getTokenValue(key, s));
		assert(s == "t1");

		assert(value->start == 10);
		assert(value->end == 11);
		assert(jp.getTokenValue(value, s));
		assert(s == "x");

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 1));
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getTokenValue(key, s));
		assert(s== "t2");

		assert(jp.getValueByKey(jp.getOuterObject(), "t2", intValue));
		assert(intValue == 1234);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 2));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", value));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 3));
		assert(jp.getTokenValue(key, s));
		assert(s == "t4");
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t4", value));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 4));
		assert(jp.getTokenValue(key, s));
		assert(s == "t5");
		assert(jp.getTokenValue(value, boolValue));
		assert(!boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 6));
		assert(jp.getTokenValue(key, s));
		assert(s == "t7");

		assert(jp.getTokenValue(value, s));
		assert(s == "\"quoted\"");
	}

	// Modifier test - change integer (same size)
	{
		JsonParserStatic<512, 32> jp;

		char *data2b = readTestData("test2b.json");
		//printf("%s", data2b);

		jp.addData(data2b, strlen(data2b));
		free(data2b);

		jp.parse();

		// printTokens(jp);
		const JsonParserGeneratorRK::jsmntok_t *key, *value;

		bool bResult = jp.getValueTokenByKey(jp.getOuterToken(), "t2", value);
		assert(bResult);

		JsonModifier mod(jp);
		bResult = mod.startModify(value);
		assert(bResult);

		mod.insertValue((int)9999);
		mod.finish();

		// printf("after modify\n");
		// printTokens(jp);

		String s;
		int intValue;
		float floatValue;
		double doubleValue;
		bool boolValue;

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 0));
		//printf("key: start=%d end=%d\n", key->start, key->end);
		//printf("value: start=%d end=%d\n", value->start, value->end);
		assert(jp.getTokenValue(key, s));
		assert(s == "t1");

		assert(jp.getTokenValue(value, s));
		assert(s == "abc");

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 1));
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 9999);

		assert(jp.getTokenValue(key, s));
		assert(s== "t2");

		assert(jp.getValueByKey(jp.getOuterObject(), "t2", intValue));
		assert(intValue == 9999);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 2));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", value));
		assert(jp.getTokenValue(value, floatValue));
		assert(floatValue == 1234.5);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 3));
		assert(jp.getTokenValue(key, s));
		assert(s == "t4");
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t4", value));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 4));
		assert(jp.getTokenValue(key, s));
		assert(s == "t5");
		assert(jp.getTokenValue(value, boolValue));
		assert(!boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 6));
		assert(jp.getTokenValue(key, s));
		assert(s == "t7");

		assert(jp.getTokenValue(value, s));
		assert(s == "\"quoted\"");
	}

	// Modifier test - change double
	{
		JsonParserStatic<512, 32> jp;

		char *data2b = readTestData("test2b.json");
		//printf("%s", data2b);

		jp.addData(data2b, strlen(data2b));
		free(data2b);

		jp.parse();

		// printTokens(jp);
		const JsonParserGeneratorRK::jsmntok_t *key, *value;

		bool bResult = jp.getValueTokenByKey(jp.getOuterToken(), "t3", value);
		assert(bResult);

		JsonModifier mod(jp);
		bResult = mod.startModify(value);
		assert(bResult);

		mod.insertValue((double)12345.6);
		mod.finish();

		// printf("after modify\n");
		// printTokens(jp);

		String s;
		int intValue;
		float floatValue;
		double doubleValue;
		bool boolValue;

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 0));
		//printf("key: start=%d end=%d\n", key->start, key->end);
		//printf("value: start=%d end=%d\n", value->start, value->end);
		assert(jp.getTokenValue(key, s));
		assert(s == "t1");

		assert(jp.getTokenValue(value, s));
		assert(s == "abc");

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 1));
		assert(jp.getTokenValue(value, intValue));
		assert(intValue == 1234);

		assert(jp.getTokenValue(key, s));
		assert(s== "t2");

		assert(jp.getValueByKey(jp.getOuterObject(), "t2", intValue));
		assert(intValue == 1234);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 2));
		assert(jp.getTokenValue(value, doubleValue));
		assert(doubleValue == 12345.6);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t3", value));
		assert(jp.getTokenValue(value, doubleValue));
		assert(doubleValue == 12345.6);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 3));
		assert(jp.getTokenValue(key, s));
		assert(s == "t4");
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getValueTokenByKey(jp.getOuterObject(), "t4", value));
		assert(jp.getTokenValue(value, boolValue));
		assert(boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 4));
		assert(jp.getTokenValue(key, s));
		assert(s == "t5");
		assert(jp.getTokenValue(value, boolValue));
		assert(!boolValue);

		assert(jp.getKeyValueTokenByIndex(jp.getOuterObject(), key, value, 6));
		assert(jp.getTokenValue(key, s));
		assert(s == "t7");

		assert(jp.getTokenValue(value, s));
		assert(s == "\"quoted\"");
	}

	// Array test low-level findLeftCommand. findRightComma
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("[1, 2 , 3]");

		jp.parse();

		//printTokens(jp);

		JsonModifier mod(jp);

		const JsonParserGeneratorRK::jsmntok_t *arrayToken = jp.getOuterArray();

		const JsonParserGeneratorRK::jsmntok_t *tok;

		tok = jp.getTokenByIndex(arrayToken, 0);
		assert(mod.findLeftComma(tok) == -1);
		assert(mod.findRightComma(tok) == 2);

		tok = jp.getTokenByIndex(arrayToken, 1);
		assert(mod.findLeftComma(tok) == 2);
		assert(mod.findRightComma(tok) == 6);

		tok = jp.getTokenByIndex(arrayToken, 2);
		assert(mod.findLeftComma(tok) == 6);
		assert(mod.findRightComma(tok) == -1);

		mod.removeArrayIndex(arrayToken, 0);
		// printTokens(jp);

		const char *expected = "[ 2 , 3]";
		assertJsonParserBuffer(jp, expected);

		mod.removeArrayIndex(arrayToken, 1);
		// printTokens(jp);

		expected = "[ 2 ]";
		assertJsonParserBuffer(jp, expected);

		mod.removeArrayIndex(arrayToken, 0);

		expected = "[  ]";
		assertJsonParserBuffer(jp, expected);

		arrayToken = jp.getOuterArray();
		mod.startAppend(arrayToken);

		mod.insertCheckSeparator();
		mod.insertValue(4);

		mod.finish();
		//printTokens(jp);

		expected = "[  4]";
		assertJsonParserBuffer(jp, expected);

		mod.startAppend(arrayToken);

		mod.insertCheckSeparator();
		mod.insertValue(5);

		mod.finish();
		//printTokens(jp);

		expected = "[  4,5]";
		assertJsonParserBuffer(jp, expected);

	}

	// Array test low-level findLeftCommand. findRightComma with strings
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("[\"a\", \"b\" ,\"c\" ]");

		jp.parse();

		//printTokens(jp);

		JsonModifier mod(jp);

		const JsonParserGeneratorRK::jsmntok_t *arrayToken = jp.getOuterArray();

		const JsonParserGeneratorRK::jsmntok_t *tok;

		tok = jp.getTokenByIndex(arrayToken, 0);
		assert(mod.findLeftComma(tok) == -1);
		assert(mod.findRightComma(tok) == 4);

		tok = jp.getTokenByIndex(arrayToken, 1);
		assert(mod.findLeftComma(tok) == 4);
		assert(mod.findRightComma(tok) == 10);

		tok = jp.getTokenByIndex(arrayToken, 2);
		assert(mod.findLeftComma(tok) == 10);
		assert(mod.findRightComma(tok) == -1);

		mod.removeArrayIndex(arrayToken, 0);
		// printTokens(jp);

		const char *expected = "[ \"b\" ,\"c\" ]";
		assertJsonParserBuffer(jp, expected);

		mod.removeArrayIndex(arrayToken, 0);
		expected = "[ \"c\" ]";
		assertJsonParserBuffer(jp, expected);

		mod.removeArrayIndex(arrayToken, 0);
		expected = "[  ]";
		assertJsonParserBuffer(jp, expected);
		// printTokens(jp);

		arrayToken = jp.getOuterArray();
		mod.startAppend(arrayToken);

		//mod.insertCheckSeparator();
		mod.insertArrayValue("d");

		mod.finish();
		//printTokens(jp);

		expected = "[  \"d\"]";
		assertJsonParserBuffer(jp, expected);

		mod.startAppend(arrayToken);

		mod.insertArrayValue("e");

		mod.finish();
		// printTokens(jp);

		expected = "[  \"d\",\"e\"]";
		assertJsonParserBuffer(jp, expected);

	}

	// Object test removeKeyValue insertKeyValue
	// Note: uses low-level API, probably best to use insertOrUpdateKeyValue in most code
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("{\"a\":1, \"b\":\"x\" ,\"c\":3 }");

		jp.parse();

		//printTokens(jp);

		JsonModifier mod(jp);

		mod.removeKeyValue(jp.getOuterObject(), "a");

		//printTokens(jp);
		const char *expected = "{ \"b\":\"x\" ,\"c\":3 }";
		assertJsonParserBuffer(jp, expected);

		mod.removeKeyValue(jp.getOuterObject(), "c");

		//printTokens(jp);
		expected = "{ \"b\":\"x\"  }";
		assertJsonParserBuffer(jp, expected);

		mod.removeKeyValue(jp.getOuterObject(), "b");

		//printTokens(jp);
		expected = "{   }";
		assertJsonParserBuffer(jp, expected);


		mod.startAppend(jp.getOuterObject());

		mod.insertKeyValue("d", (int)4);

		mod.finish();
		//printTokens(jp);
		expected = "{   \"d\":4}";
		assertJsonParserBuffer(jp, expected);


		mod.startAppend(jp.getOuterObject());

		mod.insertKeyValue("e", "test");

		mod.finish();
		//printTokens(jp);
		expected = "{   \"d\":4,\"e\":\"test\"}";
		assertJsonParserBuffer(jp, expected);
	}

	// High level insertOrUpdateKeyValue API test
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("{}");

		jp.parse();

		//printTokens(jp);

		JsonModifier mod(jp);

		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "a", (int)1);

		// printTokens(jp);

		const char *expected = "{\"a\":1}";
		assertJsonParserBuffer(jp, expected);

		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "b", "x");

		// printTokens(jp);

		expected = "{\"a\":1,\"b\":\"x\"}";
		assertJsonParserBuffer(jp, expected);

		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "b", "xxx");

		// printTokens(jp);

		expected = "{\"a\":1,\"b\":\"xxx\"}";
		assertJsonParserBuffer(jp, expected);

		// Updating a value will reorder the keys in the object
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "a", (int)999);

		expected = "{\"b\":\"xxx\",\"a\":999}";
		assertJsonParserBuffer(jp, expected);

		// String to number
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "b", (int)123);

		expected = "{\"a\":999,\"b\":123}";
		assertJsonParserBuffer(jp, expected);

		// Number to string
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "b", "x");

		expected = "{\"a\":999,\"b\":\"x\"}";
		assertJsonParserBuffer(jp, expected);

		// bool
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "c", true);

		expected = "{\"a\":999,\"b\":\"x\",\"c\":true}";
		assertJsonParserBuffer(jp, expected);

		// float
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "d", 3.5);

		expected = "{\"a\":999,\"b\":\"x\",\"c\":true,\"d\":3.500000}";
		assertJsonParserBuffer(jp, expected);

	}

	// High level appendArrayValue API test
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("[]");

		jp.parse();

		// printTokens(jp);

		JsonModifier mod(jp);

		mod.appendArrayValue(jp.getOuterArray(), (int)1);

		// printTokens(jp);

		const char *expected = "[1]";
		assertJsonParserBuffer(jp, expected);


		mod.appendArrayValue(jp.getOuterArray(), (int)2);

		//printTokens(jp);

		expected = "[1,2]";
		assertJsonParserBuffer(jp, expected);

		mod.appendArrayValue(jp.getOuterArray(), (float)3.5);

		//printTokens(jp);

		expected = "[1,2,3.500000]";
		assertJsonParserBuffer(jp, expected);

		mod.appendArrayValue(jp.getOuterArray(), (bool)true);

		//printTokens(jp);

		expected = "[1,2,3.500000,true]";
		assertJsonParserBuffer(jp, expected);

		mod.appendArrayValue(jp.getOuterArray(), "xxx");

		// printTokens(jp);

		expected = "[1,2,3.500000,true,\"xxx\"]";
		assertJsonParserBuffer(jp, expected);

	}

	// Array in object test
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("{\"t2\":{\"a\":\"foo\"},\"t3\":[1, 2, 3]}");

		jp.parse();

		JsonModifier mod(jp);

		const JsonParserGeneratorRK::jsmntok_t *arrayToken = 0;
		const char *expected;

		bool bResult = jp.getValueTokenByKey(jp.getOuterObject(), "t3", arrayToken);
		assert(bResult);
		assert(arrayToken);

		mod.appendArrayValue(arrayToken, 4);

		expected = "{\"t2\":{\"a\":\"foo\"},\"t3\":[1, 2, 3,4]}";
		assertJsonParserBuffer(jp, expected);
	}

	// Object in object test
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("{\"t2\":{\"a\":\"foo\"},\"t3\":[1, 2, 3]}");

		jp.parse();

		JsonModifier mod(jp);

		const JsonParserGeneratorRK::jsmntok_t *t2Token = 0;
		const char *expected;

		bool bResult = jp.getValueTokenByKey(jp.getOuterObject(), "t2", t2Token);
		assert(bResult);
		assert(t2Token);

		mod.insertOrUpdateKeyValue(t2Token, "b", "xxx");

		expected = "{\"t2\":{\"a\":\"foo\",\"b\":\"xxx\"},\"t3\":[1, 2, 3]}";
		assertJsonParserBuffer(jp, expected);


		jp.getValueTokenByKey(jp.getOuterObject(), "t2", t2Token);
		mod.insertOrUpdateKeyValue(t2Token, "b", "x");

		expected = "{\"t2\":{\"a\":\"foo\",\"b\":\"x\"},\"t3\":[1, 2, 3]}";
		assertJsonParserBuffer(jp, expected);

		jp.getValueTokenByKey(jp.getOuterObject(), "t2", t2Token);
		mod.insertOrUpdateKeyValue(t2Token, "a", (int)5);

		expected = "{\"t2\":{\"b\":\"x\",\"a\":5},\"t3\":[1, 2, 3]}";
		assertJsonParserBuffer(jp, expected);
	}

	// Append an object to an array
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("[]");

		jp.parse();

		JsonModifier mod(jp);

		//printTokens(jp);

		mod.startAppend(jp.getOuterArray());

		mod.startObject();

		mod.insertKeyValue("a", (int)1);
		mod.insertKeyValue("b", (bool)false);
		mod.insertKeyValue("c", "x");

		mod.finishObjectOrArray();

		mod.finish();

		//printTokens(jp);

		const char *expected;
		expected = "[{\"a\":1,\"b\":false,\"c\":\"x\"}]";
		assertJsonParserBuffer(jp, expected);

		// Add another item to the array
		mod.startAppend(jp.getOuterArray());

		mod.startObject();

		mod.insertKeyValue("a", (int)999);

		mod.finishObjectOrArray();

		mod.finish();

		//printTokens(jp);

		expected = "[{\"a\":1,\"b\":false,\"c\":\"x\"},{\"a\":999}]";
		assertJsonParserBuffer(jp, expected);

	}

	// Append an array to an object
	{
		JsonParserStatic<512, 32> jp;

		jp.addString("{}");

		jp.parse();

		JsonModifier mod(jp);

		mod.startAppend(jp.getOuterObject());

		mod.insertKeyArray("a");

		mod.insertArrayValue(1);
		mod.insertArrayValue(2);
		mod.insertArrayValue(3);

		mod.finishObjectOrArray();

		mod.finish();

		//printTokens(jp);
		assertJsonParserBuffer(jp, "{\"a\":[1,2,3]}");

		// Add another object to the array

		mod.startAppend(jp.getOuterObject());

		mod.insertKeyArray("b");

		mod.insertArrayValue("test");

		mod.finishObjectOrArray();

		mod.finish();

		//printTokens(jp);

		assertJsonParserBuffer(jp, "{\"a\":[1,2,3],\"b\":[\"test\"]}");

		// Add a simple value to the object
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "c", "xxx");

		assertJsonParserBuffer(jp, "{\"a\":[1,2,3],\"b\":[\"test\"],\"c\":\"xxx\"}");

		//printTokens(jp);

		// Change an array value to a simple value
		mod.insertOrUpdateKeyValue(jp.getOuterObject(), "b", (int)99);

		assertJsonParserBuffer(jp, "{\"a\":[1,2,3],\"c\":\"xxx\",\"b\":99}");

	}

}

// Function to dump the token table. Used while debugging the JsonModify code.
void printTokens(JsonParser &jp) {
	JsonParserGeneratorRK::jsmntok_t *tokensEnd = jp.getTokensEnd();

	for(JsonParserGeneratorRK::jsmntok_t *tok = jp.getTokens(); tok < tokensEnd; tok++) {
		printToken(jp, tok);
	}

}

void printToken(JsonParser &jp, const JsonParserGeneratorRK::jsmntok_t *tok) {
	char tempBuf[1024];

	const char *typeName = "UNKNOWN";
	switch(tok->type) {
	case JsonParserGeneratorRK::JSMN_UNDEFINED:
		typeName = "UNDEFINED";
		break;

	case JsonParserGeneratorRK::JSMN_OBJECT:
		typeName = "OBJECT";
		break;

	case JsonParserGeneratorRK::JSMN_ARRAY:
		typeName = "ARRAY";
		break;

	case JsonParserGeneratorRK::JSMN_STRING:
		typeName = "STRING";
		break;

	case JsonParserGeneratorRK::JSMN_PRIMITIVE:
		typeName = "PRIMITIVE";
		break;
	}

	memcpy(tempBuf, jp.getBuffer() + tok->start, tok->end - tok->start);
	tempBuf[tok->end - tok->start] = 0;

	printf("type=%s start=%d end=%d size=%d %s\n", typeName, tok->start, tok->end, tok->size, tempBuf);
}



void printIndent(size_t indent) {
	for(size_t ii = 0; ii < 2 * indent; ii++) {
		printf(" ");
	}
}

void printString(const char *str) {
	printf("\"");

	for(size_t ii = 0; str[ii]; ii++) {
		if (str[ii] == '"') {
			printf("\\\"");
		}
		else
		if (str[ii] == '\\') {
			printf("\\\\");
		}
		else
		if (str[ii] >= 32 && str[ii] < 127) {
			printf("%c", str[ii]);
		}
		else {
			printf("\\x%02x", str[ii]);
		}
	}
	printf("\"");
}

void printJsonInner(JsonParser &jp, const JsonParserGeneratorRK::jsmntok_t *container, size_t indent) {

	switch(container->type) {
	case JsonParserGeneratorRK::JSMN_OBJECT: {
		printIndent(indent);
		printf("{\n");

		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *keyToken;
			const JsonParserGeneratorRK::jsmntok_t *valueToken;

			if (!jp.getKeyValueTokenByIndex(container, keyToken, valueToken, ii)) {
				break;
			}
			if (ii > 0) {
				printf(",\n");
			}

			String keyName;
			jp.getTokenValue(keyToken, keyName);

			printIndent(indent + 1);
			printString(keyName);
			printf(":");
			printJsonInner(jp, valueToken, indent + 1);
		}
		printf("\n");
		printIndent(indent);
		printf("}\n");
		break;
	}
	case JsonParserGeneratorRK::JSMN_ARRAY: {
		printIndent(indent);
		printf("[\n");

		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *valueToken;

			if (!jp.getValueTokenByIndex(container, ii, valueToken)) {
				break;
			}
			if (ii > 0) {
				printf(",\n");
			}
			printIndent(indent + 1);
			printJsonInner(jp, valueToken, indent + 1);
		}
		printf("\n");
		printIndent(indent);
		printf("]\n");
		break;
	}
	case JsonParserGeneratorRK::JSMN_STRING: {
		printf("\"");
		for(int ii = container->start; ii < container->end; ii++) {
			printf("%c", jp.getBuffer()[ii]);
		}
		printf("\"");
		break;
	}
	case JsonParserGeneratorRK::JSMN_PRIMITIVE: {
		for(int ii = container->start; ii < container->end; ii++) {
			printf("%c", jp.getBuffer()[ii]);
		}
		break;
	}
	case JsonParserGeneratorRK::JSMN_UNDEFINED:
	default: {
		break;
	}
	}

}

void printJson(JsonParser &jp) {
	printJsonInner(jp, jp.getOuterToken(), 0);
}



