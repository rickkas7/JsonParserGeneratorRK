# JSON Parser and Generator

There are a number of JSON parsers and generators for Particle products including the popular [SparkJson](https://github.com/menan/sparkjson) library and [JSMNSpark](https://github.com/menan/jsmnspark).

I created yet another library because I wanted something lightweight. SparkJson creates piles of objects that are copies of the original data during parsing. [JSMN](https://github.com/zserge/jsmn) is very lightweight, but is kind of a pain to use.

What I did was wrap JSMN with an easier to use C++ API, along with adding easy value accessors. 

I also added a JSON generator that's nearly as efficient as using sprintf, but much easier to use. It takes care of escaping quotes and special characters, and converts UTF-8 to JSON UTF-16 entities.

The parser and generator are separated internally so if you only need one or the other the linker will remove the unnecessary code automatically to save space.

The [full API documentation can be found here](http://rickkas7.github.io/JsonParserGeneratorRK/).

## JSON Parser

The parser can be used in many situations, but it's particularly well-suited for handing responses from webhooks, including multi-part responses. 

The parser can be used in two different ways: static allocation, where almost all of the memory location is done in advance, or dynamically.

To do it dynamically, just construct the [JsonParser](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_parser.html) object as a global or local variable:

```
JsonParser parser;
```

To do it statically, you need to guess the maximum size of the data you want to receive and the maximum number of tokens it will have. Each object is one token, plus two tokens for each key/value pair. Each array is one token, plus one token for each value in the array.

This [JsonParserStatic](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_parser_static.html) example creates a static parser to parse up to 1024 bytes of data and 50 tokens:

```
JsonParserStatic<1024, 50> parser;
```

You then typically add the data to parse using the [addData](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_buffer.html#a760cb5be42ed2d2ca9306b1109e76af3) or [addString](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_buffer.html#a61bf30ac6e1bd460f1e809d02a7d5ba4) method. If you're getting the data from a subscribe handler, you'll probably use addString.

```
parser.addString(data);
```

If you have a pointer and length, the addData method can be used instead.

Then, once all of the data has been added, call [parse](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_parser.html#ad528213e8600cbad4d85910b62fc033a). This is handy for webhooks where you may get a multipart response. Example 3 demonstrates this:

```
void subscriptionHandler(const char *event, const char *data) {
	int responseIndex = 0;

	const char *slashOffset = strrchr(event, '/');
	if (slashOffset) {
		responseIndex = atoi(slashOffset + 1);
	}

	if (responseIndex == 0) {
		jsonParser.clear();
	}
	jsonParser.addString(data);

	if (jsonParser.parse()) {
		// Looks valid (we received all parts)

		// This printing thing is just for testing purposes, you should use the commands to
		// process data
		printJson(jsonParser);
	}
}
```

Say you have this object:

```
{
  "t1":"abc",
  "t2":1234,
  "t3":1234.5,
  "t4":true,
  "t5":false,
  "t6":null,
  "t7":"\"quoted\""
}
```

You could read the value of t1 by using [getOuterValueByKey](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_parser.html#a38858994342cd2735b716b117bf8afdf) and this code:

```
String strValue;
parser1.getOuterValueByKey("t1", strValue);
```

This also works for other data types:

```
int intValue;
parser1.getOuterValueByKey("t2", intValue)

float floatValue;
parser1.getOuterValueByKey("t3", floatValue);

bool boolValue;
parser1.getOuterValueByKey("t4", boolValue);
```

There's also a fluent-style API that can make reading complex JSON easier. For example, given this fragment of JSON:

```
{
	"response": {
		"version": "0.1",
		"termsofService": "http://www.wunderground.com/weather/api/d/terms.html",
		"features": {
			"forecast": 1
		}
	},
	"forecast": {
		"txt_forecast": {
			"date": "12:25 PM EST",
			"forecastday": {
				"period": 7,
				"icon": "nt_partlycloudy",
				"icon_url": "http://icons.wxug.com/i/c/k/nt_partlycloudy.gif",
				"title": "Saturday Night",
				"fcttext": "Partly cloudy early with increasing clouds overnight. Low 29F. Winds NW at 15 to 25 mph.",
				"fcttext_metric": "Partly cloudy early with increasing clouds overnight. Low -2C. Winds NW at 25 to 40 km/h.",
				"pop": "20"
			}
		},
```

```	
String s = parser.getReference().key("response").key("version").valueString();
// s == "0.1"

s = parser.getReference().key("forecast").key("txt_forecast").key("date").valueString();
// s = "12:25 PM EST"

int value = parser.getReference().key("forecast").key("txt_forecast").key("forecastday").key("period").valueInt();
// value == 7
```

If you have a complicated JSON file to decode, using the [JSON Parser Tool](http://rickkas7.github.io/jsonparser/) makes it easy. You paste in your JSON and it formats it nicely. Click on a row and will generate the fluent accessor to get that value!


## JSON Generator

The JSON Generator is used to build valid JSON strings. While you can build JSON using sprintf, the JSON generator is able to double-quote escape strings, and escape double quotes within strings. It can also generate correct JSON unicode characters.

The most common use is to construct a static buffer to hold the JSON data for Particle.publish. Since this data is limited to 256 bytes, this is a reasonable approach using [JsonWriterStatic](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_writer_static.html):

```
JsonWriterStatic<256> jw;
```

You can also dynamically allocate a buffer using the plain [JsonWriter](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_writer.html).

The JsonWriter handles nested objects and arrays, but does so without creating temporary copies of the objects. Because of this, it's necessary to use startObject(), startArray(), and finishObjectOrArray() so the objects are balanced properly.

To make this easier, the [JsonWriterAutoObject](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_writer_auto_object.html) can be instantiated on the stack. When the object goes out of scope, it will automatically close the object. You use it like this:

```
	{
		JsonWriterAutoObject obj(&jw);

		// Add various types of data
		jw.insertKeyValue("a", true);
		jw.insertKeyValue("b", 1234);
		jw.insertKeyValue("c", "test");
	}
```

This will output the JSON data:

```
{\"a\":true,\"b\":1234,\"c\":\"test\"}
```

If you are sending float or double values you may want to limit the number of decimal places to send. This is done using [setFloatPlaces](http://rickkas7.github.io/JsonParserGeneratorRK/class_json_writer.html#aecd4d984a49fe59b0c4d892fe6d1e791).

## JsonModifier

The JsonModifier class (added in version 0.1.0) makes it possible to modify an existing object that has been parsed with JsonParser.

You will typically process a JSON object using a `JsonParser` object, `addString()` or `addData()` method, then `parse()`.

Assuming your `JsonParser` is in the variable `jp` you then construct a temporary modifier object on the stack like this:

```
JsonModifier mod(jp);
```

The most common thing to do is have a JSON object and you want to update the value, or insert the value if it does not exist:

```
mod.insertOrUpdateKeyValue(jp.getOuterObject(), "a", (int)1);
```

If the input JSON was empty, it would then be:

```
{"a":1}
```

You can add int, long, float, double, bool, and const char * objects this way.

```
mod.insertOrUpdateKeyValue(jp.getOuterObject(), "b", "testing");
```

This would change the object to:

```
{"a":1,"b":"testing"}
```

Updating an object will remove it from its current location and add it at the end of the object.

Another common function is `appendArrayValue()` which appends to an array.

You can also use `removeKeyValue()` and `removeArrayIndex()` to remove keys or array entries.


## Examples

There are three Particle devices examples.

### 1 - Parser

The parser example is a standalone test of parsing some JSON data. The data is built into the code, so just just run it and monitor the serial output to make sure the test passes.

It also demonstrates how to read simple values out of the JSON data.

### 2 - Generator

The generator example is a standalone test of generating some JSON data. The data is built into the code, so just just run it and monitor the serial output to make sure the test passes.

It also demonstrates how to write JSON data.

### 3 - Subscription

This example creates a subscription on the event jsonParserTest, so you can send it JSON data, and it will parse and print it to the debuggging serial. For example, if you published these three events:

```
particle publish jsonParserTest '{"a":1234}' --private
particle publish jsonParserTest '{"a":1234,"b":"test"}' --private
particle publish jsonParserTest '{"a":1234,"b":"test":"c":[1,2,3]}' --private
```

You'd get these three objects printed to debugging serial.

```
{
  "a":1234
}
{
  "a":1234,
  "b":"test"
}
{
  "a":1234,
  "b":"test",
  "c":  [
    1,
    2,
    3
  ]

}

```

It also demonstrates how to handle multi-part webhook responses.


## Test code

The github repository also has code in the test directory. It can run an automated test of several sample data files to verify operation. It's run by doing something like:

```
cd test
make
```

On Linux only, if you have valgrind installed, it can also do a build with valgrind checking to check for memory leaks and buffer overruns. It's run by doing:

```
cd test
make check
```

The test code is also a reference of various ways you can call the API.

## Version History

### 0.1.4 (2020-12-23)

- Added addChunkedData() method to support subscribing to multi-part webhook response events.

### 0.1.3 (2020-09-22)

- Added JsonWriter methods insertKeyArray() and insertKeyVector() to make it easier to add arrays.
- Added JsonWriter methods insertArray() and insertVector() to make it easier to add arrays.

### 0.1.1 (2020-05-14)

Fixed a bug where calling parse() on an empty buffer returns true. It should return false. See issue #7.

### 0.1.0 (2019-09-18)

Added support for JsonModifier, a class to modify an existing JSON object in place, without making a copy of it.

### 0.0.7 (2019-08-30)

Fixed a bug in the 3-subscription example. The check for the part number should use strrchr, not strchr, because it needs to 
find the last slash before the part number for webhook multi-part responses.


