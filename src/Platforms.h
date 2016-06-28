#pragma once

#if defined(ESP8266)
//==============================================================================
// ESP8266
//==============================================================================
#include "Arduino.h"
#include <ESP8266WiFi.h>
#define HAS_REMOTE_IP 1
#define SERVER_BEGIN_BOOL 0
//------------------------------------------------------------------------------

#elif defined(PARTICLE) || defined(SPARK)
//==============================================================================
// Particle
//==============================================================================
#ifndef PARTICLE
#define PARTICLE
#endif

#define WiFiClient TCPClient
#define WiFiServer TCPServer

#include "application.h"
#define HAS_REMOTE_IP 1
#define SERVER_BEGIN_BOOL 1
//------------------------------------------------------------------------------

#elif defined(ARDUINO)
//==============================================================================
// Zero
//==============================================================================
#ifndef ZERO
#define ZERO
#endif

#include "Arduino.h"
#include <WiFi101.h>
#define HAS_REMOTE_IP 0
#define SERVER_BEGIN_BOOL 0
//------------------------------------------------------------------------------

#endif
