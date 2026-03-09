# That-GuyTP.github.io Backend

This repository contains backend APIs for modules showcased on https://that-guytp.github.io.

## Stack

- Java 21
- Spring Boot 3
- Maven
- Docker (for Render deployment)

## Current API

### Health check

- `GET /health`

Returns:

```json
{ "status": "ok" }
```

### SortSorter API

- `POST /api/sortsorter/sort`

Request body:

```json
{
  "words": ["sort", "assorted", "banana", "SortSorter"]
}
```

Response body:

```json
{
  "sortedWords": ["banana", "sort", "assorted", "SortSorter"],
  "sortedEntries": [
    { "word": "banana", "sortCount": 0 },
    { "word": "sort", "sortCount": 1 },
    { "word": "assorted", "sortCount": 1 },
    { "word": "SortSorter", "sortCount": 2 }
  ],
  "inputSize": 4
}
```

## Local run

```bash
mvn spring-boot:run
```

Default URL: `http://localhost:8080`

## Render deployment

- Service type: `Web Service`
- Runtime: `Docker`
- Plan: `Free` (upgrade to `Starter` for no spin-down)
- Health check path: `/health`
- Required env var:
  - `CORS_ALLOWED_ORIGINS=https://that-guytp.github.io`

You can deploy using `render.yaml` in this repository.
