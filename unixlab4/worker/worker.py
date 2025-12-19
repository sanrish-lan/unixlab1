from kafka import KafkaConsumer
import os
import json
import requests
import time
import socket

KAFKA_BOOTSTRAP_SERVERS = os.getenv('KAFKA_BOOTSTRAP_SERVERS', 'localhost:9092')
TOPIC_NAME = 'url_checks'
GROUP_ID = 'url_worker_group'
WORKER_ID = socket.gethostname()


def check_website(url):
    print(f"[{WORKER_ID}] Проверяю: {url}")
    try:
        time.sleep(2)  # Имитация нагрузки
        response = requests.get(url, timeout=5)
        print(f"[{WORKER_ID}] Готово: {url} -> Status {response.status_code}")
    except Exception as e:
        print(f"[{WORKER_ID}] Ошибка: {url} -> {e}")


def main():
    print(f"[{WORKER_ID}] Запуск Consumer. Подключение к Kafka...")

    while True:
        try:
            consumer = KafkaConsumer(
                TOPIC_NAME,
                bootstrap_servers=KAFKA_BOOTSTRAP_SERVERS,
                group_id=GROUP_ID,
                auto_offset_reset='earliest',
                enable_auto_commit=True,
                value_deserializer=lambda x: json.loads(x.decode('utf-8'))
            )
            break
        except Exception as e:
            print(f"[{WORKER_ID}] Kafka недоступна ({e}). Рестарт через 5 сек...")
            time.sleep(5)

    print(f"[{WORKER_ID}] Подключено к топику '{TOPIC_NAME}'. Жду задач...")

    for message in consumer:
        data = message.value
        url = data.get('url')
        check_website(url)


if __name__ == '__main__':
    main()