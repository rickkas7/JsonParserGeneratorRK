#include "Particle.h"

#include "JsonParserGeneratorRK.h"

const unsigned long TEST_RUN_PERIOD_MS = 10000;
unsigned long lastRun = 0;

void runTest();

const char * const test2 = "{\"t1\":\"abc\",\"t2\":1234,\"t3\":1234.5,\"t4\":true,\"t5\":false,\"t6\":null, \"t7\" : \"\\\"quoted\\\"\" } ";

// Global parser that supports up to 256 bytes of data and 20 tokens
JsonParserStatic<256, 20> parser1;

void setup() {
	Serial.begin(9600);
}

void loop() {
	if (millis() - lastRun >= TEST_RUN_PERIOD_MS) {
		lastRun = millis();
		runTest();
	}
}

void runTest() {
	// Clear the parser state, add the string test2, and parse it
	parser1.clear();
	parser1.addString(test2);
	if (!parser1.parse()) {
		Serial.println("parsing failed test2");
		return;
	}

	String strValue;
	if (!parser1.getOuterValueByKey("t1", strValue)) {
		Serial.println("failed to get test2 t1");
		return;
	}
	if (strValue != "abc") {
		Serial.printlnf("wrong value test2 t1 was %s", strValue.c_str());
		return;
	}

	String keyName;
	if (!parser1.getOuterKeyValueByIndex(0, keyName, strValue)) {
		Serial.println("failed to get test2 t1 by index");
		return;
	}
	if (keyName != "t1") {
		Serial.printlnf("wrong key name test2 t1 was %s by index", keyName.c_str());
		return;
	}
	if (strValue != "abc") {
		Serial.printlnf("wrong value test2 t1 was %s by index", strValue.c_str());
		return;
	}

	int intValue;
	if (!parser1.getOuterValueByKey("t2", intValue)) {
		Serial.println("failed to get test2 t2");
		return;
	}
	if (intValue != 1234) {
		Serial.printlnf("wrong value test2 t2 was %d", intValue);
		return;
	}
	intValue = -1;

	if (!parser1.getOuterKeyValueByIndex(1, keyName, intValue)) {
		Serial.println("failed to get test2 t2 by index");
		return;
	}
	if (keyName != "t2") {
		Serial.printlnf("wrong key name test2 t2 was %s by index", keyName.c_str());
		return;
	}
	if (intValue != 1234) {
		Serial.printlnf("wrong value test2 t2 was %d by index", intValue);
		return;
	}


	float floatValue;
	if (!parser1.getOuterValueByKey("t3", floatValue)) {
		Serial.println("failed to get test2 t3");
		return;
	}
	if (floatValue != 1234.5) {
		Serial.printlnf("wrong value test2 t3 was %f", floatValue);
		return;
	}

	bool boolValue;
	if (!parser1.getOuterValueByKey("t4", boolValue)) {
		Serial.println("failed to get test2 t4");
		return;
	}
	if (boolValue != true) {
		Serial.printlnf("wrong value test2 t4 was %d", boolValue);
		return;
	}

	if (!parser1.getOuterValueByKey("t5", boolValue)) {
		Serial.println("failed to get test2 t5");
		return;
	}
	if (boolValue != false) {
		Serial.printlnf("wrong value test2 t5 was %d", boolValue);
		return;
	}


	if (!parser1.getOuterValueByKey("t7", strValue)) {
		Serial.println("failed to get test2 t7");
		return;
	}
	if (strValue != "\"quoted\"") {
		Serial.printlnf("wrong value test2 75 was %s", strValue.c_str());
		return;
	}

	Serial.println("test passed!");
}
