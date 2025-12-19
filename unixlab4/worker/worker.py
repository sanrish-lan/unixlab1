import pika
import os
import json
import requests
import time
import socket

BROKER_URL = os.getenv('BROKER_URL', 'amqp://guest:guest@localhost:5672/')
WORKER_ID = socket.gethostname()


def check_website(url):
    print(f"[{WORKER_ID}] Начало проверки: {url}")
    try:
        time.sleep(2)
        response = requests.get(url, timeout=5)
        print(f"[{WORKER_ID}] Успех: {url} -> Status {response.status_code}")
    except Exception as e:
        print(f"[{WORKER_ID}] Ошибка при проверке {url}: {e}")


def callback(ch, method, properties, body):
    data = json.loads(body)
    url = data.get('url')

    check_website(url)

    ch.basic_ack(delivery_tag=method.delivery_tag)


def main():
    print(f"[{WORKER_ID}] Запуск воркера. Ожидание подключения к брокеру...")
    connection = None

    while connection is None:
        try:
            connection = pika.BlockingConnection(pika.URLParameters(BROKER_URL))
        except pika.exceptions.AMQPConnectionError:
            print(f"[{WORKER_ID}] Брокер недоступен, повторная попытка через 5 сек...")
            time.sleep(5)

    channel = connection.channel()
    channel.queue_declare(queue='url_check_queue', durable=True)

    channel.basic_qos(prefetch_count=1)

    channel.basic_consume(queue='url_check_queue', on_message_callback=callback)

    print(f"[{WORKER_ID}] Ожидание задач...")
    channel.start_consuming()


if __name__ == '__main__':
    main()