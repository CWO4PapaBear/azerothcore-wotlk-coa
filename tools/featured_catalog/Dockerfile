FROM python:3.12-slim-bookworm
RUN apt-get update && apt-get install -y --no-install-recommends default-mysql-client gh ca-certificates \
    && rm -rf /var/lib/apt/lists/*
WORKDIR /opt/featured-catalog
COPY . .
ENV PYTHONDONTWRITEBYTECODE=1 PYTHONUNBUFFERED=1
USER 1000:1000
ENTRYPOINT ["python3", "worker.py", "--config", "/config/worker.json"]
