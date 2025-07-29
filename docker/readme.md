### Build and run docker with environment variables
docker compose -f compose.dev.yml --env-file .env.dev up -d --build --force-recreate

### Monitor all docker services continuously
docker compose -f compose.dev.yml --env-file .env.dev logs -f

### Nuke all volumes
docker compose -f compose.dev.yml --env-file .env.dev down -v
