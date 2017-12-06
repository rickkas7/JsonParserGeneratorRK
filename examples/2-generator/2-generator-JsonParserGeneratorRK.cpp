#include "Particle.h"

#include "JsonParserGeneratorRK.h"

const unsigned long TEST_RUN_PERIOD_MS = 10000;
unsigned long lastRun = 0;

void runTest();


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
	// This creates a buffer to hold up to 256 bytes of JSON data (good for Particle.publish)
	JsonWriterStatic<256> jw;

	// Creating a scope like this in {} with a JsonWriterAutoObject in it creates an object,
	// and automatically closes the object when leaving the scope. This is necessary because
	// all JSON values must be in either an object or an array to be valid, and JsonWriter
	// requires all startObject to be balanced with a finishObjectOrArray and JsonWriterAutoObject
	// takes care of doing that automatically.
	{
		JsonWriterAutoObject obj(&jw);

		// Add various types of data
		jw.insertKeyValue("a", true);
		jw.insertKeyValue("b", 1234);
		jw.insertKeyValue("c", "test");
	}

	// Verify the results
	if (strcmp(jw.getBuffer(), "{\"a\":true,\"b\":1234,\"c\":\"test\"}")) {
		Serial.printlnf("test mismatch got %s", jw.getBuffer());
		return;
	}


	Serial.println("test passed!");
}
