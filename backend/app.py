from flask import Flask, jsonify
from database import create_table
app = Flask(__name__)
#Create databse table when the application starts
try:
    create_table()
    print("PostgreSQL database connected succesfully.")
except Exception as e:
    print("Database connection failed:",e)

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
    app.run(host="0.0.0.0", port=5000, debug=True)
