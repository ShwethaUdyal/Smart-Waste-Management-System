from flask import Flask, jsonify
from database import create_table, get_connection
import paho.mqtt.client as mqtt
import json
import threading

app = Flask(__name__)

MQTT_BROKER = "YOUR_MQTT_BROKER"
MQTT_PORT = 1883
MQTT_TOPIC = "smartwaste/bin"


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
        client.subscribe(MQTT_TOPIC)
    else:
        print("MQTT connection failed:", rc)


def on_message(client, userdata, message):
    try:
        data = json.loads(message.payload.decode())

        bin_id = data["bin_id"]
        fill_level = data["fill_level"]
        distance = data["distance"]

        connection = get_connection()
        cursor = connection.cursor()

        cursor.execute(
            """
            INSERT INTO waste_bins
            (bin_id, fill_level, distance)
            VALUES (%s, %s, %s)
            """,
            (bin_id, fill_level, distance)
        )

        connection.commit()

        cursor.close()
        connection.close()

        print("Waste-bin data stored:", data)

    except Exception as e:
        print("Error processing MQTT message:", e)


def start_mqtt():

    client = mqtt.Client()

    client.on_connect = on_connect
    client.on_message = on_message

    client.connect(MQTT_BROKER, MQTT_PORT, 60)

    client.loop_forever()


@app.route("/")
def home():
    return jsonify({
        "message": "Smart Waste Management API is running",
        "status": "success"
    })


@app.route("/health")
def health():
    return jsonify({
        "status": "healthy"
    })


if __name__ == "__main__":

    try:
        create_table()
        print("PostgreSQL table ready.")
    except Exception as e:
        print("Database connection failed:", e)

    # mqtt_thread = threading.Thread(
    #     target=start_mqtt,
    #     daemon=True
    # )

    # mqtt_thread.start()

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=True
    )
