#include "Particle.h"

#include "JsonParserGeneratorRK.h"

void subscriptionHandler(const char *event, const char *data);
void printJson(JsonParser &jp);

// Create a parser to handle 2K of data and 100 tokens
JsonParserStatic<2048, 100> jsonParser;

// Test commands:
// particle publish jsonParserTest '{"a":1234}' --private
// particle publish jsonParserTest '{"a":1234,"b":"test"}' --private
// particle publish jsonParserTest '{"a":1234,"b":"test":"c":[1,2,3]}' --private


void setup() {
	Serial.begin(9600);
	Particle.subscribe("jsonParserTest", subscriptionHandler, MY_DEVICES);
}

void loop() {
}

void subscriptionHandler(const char *event, const char *data) {
	
	jsonParser.addChunkedData(event, data);

	if (jsonParser.parse()) {
		// Looks valid (we received all parts)

		// This printing thing is just for testing purposes, you should use the commands to
		// process data and extract the parts you need
		printJson(jsonParser);

		// After parsing be sure to clear the data so the next set of responses will start
		// fresh with no data saved.
		jsonParser.clear();
	}
}

void printIndent(size_t indent) {
	for(size_t ii = 0; ii < 2 * indent; ii++) {
		Serial.printf(" ");
	}
}

void printString(const char *str) {
	Serial.printf("\"");

	for(size_t ii = 0; str[ii]; ii++) {
		if (str[ii] == '"') {
			Serial.printf("\\\"");
		}
		else
		if (str[ii] == '\\') {
			Serial.printf("\\\\");
		}
		else
		if (str[ii] >= 32 && str[ii] < 127) {
			Serial.printf("%c", str[ii]);
		}
		else {
			Serial.printf("\\x%02x", str[ii]);
		}
	}
	Serial.printf("\"");
}

void printJsonInner(JsonParser &jp, const JsonParserGeneratorRK::jsmntok_t *container, size_t indent) {

	switch(container->type) {
	case JsonParserGeneratorRK::JSMN_OBJECT: {
		printIndent(indent);
		Serial.printf("{\n");

		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *keyToken;
			const JsonParserGeneratorRK::jsmntok_t *valueToken;

			if (!jp.getKeyValueTokenByIndex(container, keyToken, valueToken, ii)) {
				break;
			}
			if (ii > 0) {
				Serial.printf(",\n");
			}

			String keyName;
			jp.getTokenValue(keyToken, keyName);

			printIndent(indent + 1);
			printString(keyName);
			Serial.printf(":");
			printJsonInner(jp, valueToken, indent + 1);
		}
		Serial.printf("\n");
		printIndent(indent);
		Serial.printf("}\n");
		break;
	}
	case JsonParserGeneratorRK::JSMN_ARRAY: {
		printIndent(indent);
		Serial.printf("[\n");

		for(size_t ii = 0; ; ii++) {
			const JsonParserGeneratorRK::jsmntok_t *valueToken;

			if (!jp.getValueTokenByIndex(container, ii, valueToken)) {
				break;
			}
			if (ii > 0) {
				Serial.printf(",\n");
			}
			printIndent(indent + 1);
			printJsonInner(jp, valueToken, indent + 1);
		}
		Serial.printf("\n");
		printIndent(indent);
		Serial.printf("]\n");
		break;
	}
	case JsonParserGeneratorRK::JSMN_STRING: {
		Serial.printf("\"");
		for(int ii = container->start; ii < container->end; ii++) {
			Serial.printf("%c", jp.getBuffer()[ii]);
		}
		Serial.printf("\"");
		break;
	}
	case JsonParserGeneratorRK::JSMN_PRIMITIVE: {
		for(int ii = container->start; ii < container->end; ii++) {
			Serial.printf("%c", jp.getBuffer()[ii]);
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
