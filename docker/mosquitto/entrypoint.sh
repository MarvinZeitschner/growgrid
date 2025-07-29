#!/bin/sh
set -e

mkdir -p /mosquitto/data /mosquitto/log

if [ ! -f /mosquitto/data/passwd ] && [ -n "$MQTT_USERNAME" ] && [ -n "$MQTT_PASSWORD" ]; then
  mosquitto_passwd -b -c /mosquitto/data/passwd "$MQTT_USERNAME" "$MQTT_PASSWORD"
  chown mosquitto:mosquitto /mosquitto/data/passwd
  chmod 600 /mosquitto/data/passwd
fi

exec mosquitto -c /mosquitto/config/mosquitto.conf
