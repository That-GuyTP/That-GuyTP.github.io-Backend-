# That-GuyTP.github.io Backend

This repository contains backend APIs for modules showcased on:
https://that-guytp.github.io

## Stack

- Java 21
- Spring Boot 3
- Maven
- Docker (Render deployment)

## Current APIs

### Health

- `GET /health`
- `GET /api/health`

### SortSorter

- `POST /api/sortsorter/sort`

### LoveLearningLangs

- `GET /api/content/bootstrap`
- `POST /api/auth/register`
- `POST /api/auth/login`
- `GET /api/auth/me`
- `POST /api/progress/add-language`
- `POST /api/progress/complete-exercise`
- `GET /api/progress/review`
- `POST /api/exercise/start`
- `POST /api/exercise/submit`

LLL data files are loaded from:
- `data/lovelearninglangs/Words.json`
- `data/lovelearninglangs/Phrases.json`
- `data/lovelearninglangs/Users.json`
- `data/lovelearninglangs/IpSaveHistory.json`

## Environment variables

- `PORT` (provided by Render)
- `CORS_ALLOWED_ORIGINS` (comma-separated)
- `LLL_TOKEN_SECRET` (required in production)
- `LLL_USER_RETENTION_MS` (optional)
- `LLL_SAVE_LIMIT_MAX` (optional)
- `LLL_SAVE_LIMIT_WINDOW_MS` (optional)
- `LLL_DATA_DIR` (optional override for LLL data path)

## Local run

```bash
mvn spring-boot:run
```

Default URL: `http://localhost:8080`

## Render deployment

- Service type: `Web Service`
- Runtime: `Docker`
- Health check path: `/health`
- Recommended vars:
  - `CORS_ALLOWED_ORIGINS=https://that-guytp.github.io`
  - `LLL_TOKEN_SECRET=<secure-random-value>`

## Frontend connection

In the frontend repository, set:

```bash
VITE_LLL_API_BASE_URL=https://<your-render-backend>/api
```
