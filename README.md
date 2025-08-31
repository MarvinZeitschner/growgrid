# GrowGrid Project

This project is an IoT system for automated plant watering and monitoring.

## First-Time Setup

To run this project on a new machine, you need Docker, Docker Compose, and OpenSSL installed.

All setup commands should be run from within the `docker/` directory.

1.  **Navigate to the Docker Directory**
    ```bash
    cd docker
    ```

2.  **Prepare Environment File**
    Copy the example environment file. Then, edit `.env.dev` to add your secrets and set `SERVER_IP` to your host machine's local IP address.
    ```bash
    cp .env.dev.example .env.dev
    ```

3.  **Generate TLS Certificates**
    Load the environment variables from your `.env.dev` file and then run the generation script.
    ```bash
    source .env.dev
    sh mosquitto/generate-certs.sh
    ```

4.  **Start Docker Services**
    Build and start the Docker containers in detached mode.
    ```bash
    docker compose -f compose.dev.yml up -d --build
    ```

## Firmware Setup (ESP32)

The `generate-certs.sh` script automatically copies the necessary certificates to the correct firmware component directory.

After running the script from the `docker` directory, you can navigate back to the project root and build the firmware:
```bash
cd ..
idf.py fullclean
idf.py build
```