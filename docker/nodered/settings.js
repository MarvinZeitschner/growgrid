module.exports = {
  uiPort: process.env.PORT || 1880,
  mqttReconnectTime: 15000,
  serialReconnectTime: 15000,
  debugMaxLength: 1000,

  functionGlobalContext: {
    influxdb_host: process.env.INFLUXDB_HOST || 'influxdb2',
    influxdb_token: process.env.INFLUXDB_TOKEN,
    influxdb_org: process.env.INFLUXDB_ORG || 'growgrid',
    influxdb_bucket: process.env.INFLUXDB_BUCKET || 'sensors',
    mqtt_broker: process.env.MQTT_BROKER || 'mosquitto',
    mqtt_port: process.env.MQTT_PORT || 1883,
    mqtt_username: process.env.MQTT_USERNAME || '',
    mqtt_password: process.env.MQTT_PASSWORD || ''
  },

  autoInstallModules: true,

  logging: {
    console: {
      level: "info",
      metrics: false,
      audit: false
    }
  }
};
