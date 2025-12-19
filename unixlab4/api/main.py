from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from kafka import KafkaProducer
import json
import os

app = FastAPI()

KAFKA_BOOTSTRAP_SERVERS = os.getenv('KAFKA_BOOTSTRAP_SERVERS', 'localhost:9092')
TOPIC_NAME = 'safe_tasks'

producer = KafkaProducer(
    bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
    value_serializer=lambda v: json.dumps(v).encode('utf-8'),
    acks='all',
    retries=5
)


class Task(BaseModel):
    url: str


@app.post("/check")
async def check_url(task: Task):
    try:
        future = producer.send(TOPIC_NAME, {'url': task.url})
        result = future.get(timeout=10)

        return {
            "status": "queued",
            "url": task.url,
            "partition": result.partition,
            "offset": result.offset
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=f"Kafka Error: {str(e)}")