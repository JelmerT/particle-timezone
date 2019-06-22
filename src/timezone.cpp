/* Particle-timezone library by Jelmer Tiete<jelmer@tiete.be>
 */

#include "timezone.h"
#include "JsonParserGeneratorRK.h"

static char requestBuf[256];
static char *requestCur;
static int numAdded = 0;

/**
 * Constructor.
 */
Timezone::Timezone(): utcOffset(0), rawOffset(0), dstOffset(0),
	timezoneSet(false), eventName("timezone"){

}

Timezone &Timezone::withEventName(const char *name) {
	this->eventName = name;
	return *this;
}

/**
 * Timezone constructor, must be run first
 */
void Timezone::begin()
{
  	snprintf(requestBuf, sizeof(requestBuf), "hook-response/%s/%s",
  		eventName.c_str(), System.deviceID().c_str());
  	Particle.subscribe(requestBuf, &Timezone::subscriptionHandler,
  		this, MY_DEVICES);
}

/**
 * Request a timezone update
 */
bool Timezone::request()
{
    return publishTimezoneLocation();
}

/**
 * return true if timezone was set
 */
bool Timezone::isValid()
{
	return this->timezoneSet;
}

/**
* Example private method
*/
bool Timezone::publishTimezoneLocation() {

	const char *scanData = scanWifiNetworks();

	// Serial.printlnf("scanData=%s", scanData);

	if (scanData[0]) {

		if (Particle.connected()) {
			Log.info("Getting timezone...");
			return Particle.publish(eventName, scanData, PRIVATE);
		} else {
			Log.info("Getting timezone failed: Not connected to Particle cloud");
			return false;
		}
	} else {
		return false;
	}
}

/**
* Example private method
*/
static void wifiScanCallback(WiFiAccessPoint* wap, void* data) {
	// The - 3 factor here to leave room for the closing JSON array ] object }} and the trailing null
	size_t spaceLeft = &requestBuf[sizeof(requestBuf) - 3] - requestCur;

	size_t sizeNeeded = snprintf(requestCur, spaceLeft,
			"%s{\"m\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"s\":%d,\"c\":%d}",
			(requestCur[-1] == '[' ? "" : ","),
			wap->bssid[0], wap->bssid[1], wap->bssid[2], wap->bssid[3],
			wap->bssid[4], wap->bssid[5], wap->rssi, wap->channel);
	if (sizeNeeded <= spaceLeft) {
		// There is enough space to store the whole entry, so save it
		requestCur += sizeNeeded;
		numAdded++;
	}
}

/**
* Example private method
*/
const char *Timezone::scanWifiNetworks() {

	requestCur = requestBuf;
	numAdded = 0;

	// Building JSON request payload
	requestCur += sprintf(requestCur, "{\"a\":");
	*requestCur++ = '[';

	WiFi.scan(wifiScanCallback);

	*requestCur++ = ']';
	*requestCur++ = '}';
	*requestCur++ = 0;

	if (numAdded == 0) {
		requestBuf[0] = 0;
	}

	return requestBuf;
}

/**
* Example private method
*/
void Timezone::subscriptionHandler(const char *event, const char *data) {
	JsonParserStatic<256, 10> jsonParser;
	int dstOffsetValue, rawOffsetValue = 0;
	String dataString = "";

	Log.info("Timezone received");
  	// Log.info(event);
  	// Log.info(data);

	jsonParser.clear();
	jsonParser.addString(data);

	if (!jsonParser.parse()) {
		Log.error("Timezone callback parsing failed");
	}
	if (jsonParser.getOuterValueByKey("data", dataString)) {
		Log.info("Parsed data: %s", dataString.c_str());
		if (dataString == "test-event")
			Log.info("Webhook and server are working!");
	}
	if (jsonParser.getOuterValueByKey("rawOffset", rawOffsetValue)) {
		Log.info("parsed rawOffset in sec: %d", rawOffsetValue);
		this->rawOffset = float(rawOffsetValue/3600);
	}
	if (jsonParser.getOuterValueByKey("dstOffset", dstOffsetValue)) {
		Log.info("parsed dstOffset in sec: %d", dstOffsetValue);
		this->dstOffset = float(dstOffsetValue/3600);
	}

	this->utcOffset = (this->rawOffset + this->dstOffset);

  	// Log.info("Got new UTC offset: %d", this->utcOffset);
  	// Log.info("Got new DST offset: %d", this->dstOffset);

  	Time.zone(this->rawOffset);
	Time.setDSTOffset(this->dstOffset);

	if (this->dstOffset == 0)
		Time.endDST();
	else
		Time.beginDST();

  	this->timezoneSet = true;

  	Log.info("UTC offset: %.1f", this->utcOffset);

}
