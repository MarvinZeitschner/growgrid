#!/bin/sh
set -e

if [ -n "$MQTT_USERNAME" ] && [ -n "$MQTT_PASSWORD" ]; then
  mosquitto_passwd -c -b /mosquitto/config/passwd "$MQTT_USERNAME" "$MQTT_PASSWORD"
fi

exec mosquitto -c /mosquitto/config/mosquitto.conf
