from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
import pika
import os
import json

app = FastAPI()

BROKER_URL = os.getenv('BROKER_URL', 'amqp://guest:guest@localhost:5672/')


class Task(BaseModel):
    url: str


def publish_message(url: str):
    try:
        connection = pika.BlockingConnection(pika.URLParameters(BROKER_URL))
        channel = connection.channel()
        channel.queue_declare(queue='url_check_queue', durable=True)

        message = json.dumps({'url': url})
        channel.basic_publish(
            exchange='',
            routing_key='url_check_queue',
            body=message,
            properties=pika.BasicProperties(delivery_mode=2, ))
        connection.close()
    except Exception as e:
        print(f"Error publishing to RabbitMQ: {e}")
        raise e


@app.post("/check")
async def check_url(task: Task):
    try:
        publish_message(task.url)
        return {"status": "queued", "url": task.url, "message": "Задача отправлена работникам"}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
