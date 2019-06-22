#pragma once

/* Particle-timezone library by Jelmer Tiete<jelmer@tiete.be>
 */

// This will load the definition for common Particle variable types
#include "Particle.h"

// This is your main class that users will import into their application
class Timezone
{
public:
  float utcOffset;
  float rawOffset;
  float dstOffset;

  /**
   * Constructor
   */
  Timezone();

  /**
   * The default event name is "timezone". Use this method to change it. Note that this
   * name is also configured in your server and must be changed in both
   * locations or timezone requests will not work
   */
  Timezone &withEventName(const char *name);

  /**
   * Example method
   */
  void begin();

  /**
   * Example method
   */
  bool request();

  /**
   * Example method
   */
  bool isValid();

protected:
  bool publishTimezoneLocation();
  const char *scanWifiNetworks();
  void subscriptionHandler(const char *event, const char *data);

  bool timezoneSet;
  String eventName;
};
