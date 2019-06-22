// Example usage for timezone library by Jelmer Tiete<jelmer@tiete.be>.

#include "timezone.h"

SerialLogHandler logHandler;

// Initialize the Timezone lib
Timezone timezone;

void setup() {

	// Make sure you call timezone.begin() in your setup() function
    // Here you can define a custom event name
    // Make sure to change the event name in your webhook and server too
    timezone.withEventName("test/custom_timezone_event").begin();

}

void loop() {

    // If we don't have our timezone
	if (!timezone.isValid())
	{
		Log.info("Requesting local UTC offset.");

		// This is the only function you need to call to update the timezone
		timezone.request();
	}

    // If we synced the time and we have a valid timezone
    if (Time.isValid() && timezone.isValid()){

    	Log.info("The current time is: %s", Time.format(TIME_FORMAT_ISO8601_FULL).c_str());

		Log.info("Raw UTC offset: %.1f", timezone.rawOffset);
    	Log.info("Daylight savings time offset: %.1f", timezone.dstOffset);

    	if (Time.isDST())
    		Log.info("DST is in effect!");
    	else
    		Log.info("DST is not in effect.");

	    Log.info("UTC offset: %.1f", timezone.utcOffset); //rawOffset + dstOffset
	    Log.info("------------------------------");

    } else {

    	Log.info("Waiting for time fix or timezone update...");

    }

    delay(30000);
}
