import threading
import time
import datetime
import json
import requests
import sseclient
from flask import Flask, render_template, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import func

# ────────────────────────────────────────────────────────────────
# Flask & DB setup
# ────────────────────────────────────────────────────────────────
app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///energy.db"
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False
db = SQLAlchemy(app)

# ────────────────────────────────────────────────────────────────
# Model: one row per hourly reading
# ────────────────────────────────────────────────────────────────
class Reading(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp = db.Column(db.DateTime, index=True, nullable=False)
    energy_kwh = db.Column(db.Float, nullable=False)

with app.app_context():
    db.create_all()

# ────────────────────────────────────────────────────────────────
# Particle credentials & config
# ────────────────────────────────────────────────────────────────
PARTICLE_DEVICE_ID = "0a10aced202194944a04b1a0"
PARTICLE_TOKEN = "ee88804db6c92ced674dc76a81df670f597141d6"
EVENT_NAME = "energy_hourly"  # the Particle event name your device publishes
COST_PER_KWH = 0.12

# ────────────────────────────────────────────────────────────────
# Particle event stream listener
# ────────────────────────────────────────────────────────────────


def listen_particle_events():
    print("Starting Particle event listener...")

    url = f"https://api.particle.io/v1/devices/events/energy_hourly?access_token={PARTICLE_TOKEN}"
    
    try:
        with requests.get(url, stream=True) as response:
            if response.status_code != 200:
                print(f"Failed to connect: {response.status_code}")
                return

            for line in response.iter_lines():
                if line:
                    decoded_line = line.decode("utf-8")
                    if decoded_line.startswith("data:"):
                        event_data = decoded_line[5:].strip()
                        try:
                            data_json = json.loads(event_data)
                            raw_data = data_json.get("data", "{}")
                            parsed_data = json.loads(raw_data)
                            energy_wh = float(parsed_data.get("energy_wh", 0))
                            energy_kwh = energy_wh / 1000.0
                            published_at = data_json.get("published_at")

                            print(f"Received event energy_hourly: {energy_kwh:.4f} kWh at {published_at}")

                            timestamp = datetime.datetime.utcnow()
                            with app.app_context():
                                reading = Reading(timestamp=timestamp, energy_kwh=energy_kwh)
                                db.session.add(reading)
                                db.session.commit()

                            #print(f"Received event {event.event}: {energy_kwh} kWh at {timestamp}")

                        except Exception as e:
                            print("Error parsing event data:", e)
                            print("event data was:", event_data)

    except Exception as e:
        print("Particle event stream connection error:", e)

# ────────────────────────────────────────────────────────────────
# API endpoints
# ────────────────────────────────────────────────────────────────
@app.route("/api/v1/energy/total")
def energy_total():
    row = Reading.query.order_by(Reading.timestamp.desc()).first()
    if not row:
        return jsonify({"timestamp": None, "energy_total": 0.0})
    return jsonify({"timestamp": row.timestamp.isoformat(),
                    "energy_total": row.energy_kwh})

@app.route('/api/v1/stats/summary')
def stats_summary():
    try:
        now = datetime.datetime.utcnow()
        today0 = now.replace(hour=0, minute=0, second=0, microsecond=0)
        wnd30  = now - datetime.timedelta(days=30)

        today_kwh = (db.session.query(func.coalesce(func.sum(Reading.energy_kwh), 0))
                            .filter(Reading.timestamp >= today0)
                            .scalar())

        usage_30d = (db.session.query(func.coalesce(func.sum(Reading.energy_kwh), 0))
                            .filter(Reading.timestamp >= wnd30)
                            .scalar())

        total_energy = (db.session.query(func.coalesce(func.sum(Reading.energy_kwh), 0))
                                .scalar())

        cost_today = today_kwh * COST_PER_KWH

        return jsonify({
            "today_kwh": round(today_kwh, 2),
            "cost_today": round(cost_today, 2),
            "usage_30d": round(usage_30d, 2),
            "total_energy": round(total_energy, 2)
        })

    except Exception as e:
        print(f"Error fetching summary stats: {e}")
        return jsonify({"error": str(e)}), 500

@app.route('/api/v1/stats/daily')
def stats_daily():
    wnd30 = datetime.datetime.utcnow() - datetime.timedelta(days=30)
    rows = (db.session.query(func.date(Reading.timestamp).label('day'),
                             func.sum(Reading.energy_kwh).label('kWh'))
                     .filter(Reading.timestamp >= wnd30)
                     .group_by('day')
                     .order_by('day')
                     .all())
    data = [{'day': str(day), 'kWh': float(kwh)} for day, kwh in rows]
    return jsonify(data)

@app.route("/")
def dashboard():
    return render_template("dashboard.html")

# ────────────────────────────────────────────────────────────────
# Main: start listener thread and run Flask
# ────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    # Make sure DB tables exist
    with app.app_context():
        db.create_all()

    # Start Particle event listener thread
    listener_thread = threading.Thread(target=listen_particle_events, daemon=True)
    listener_thread.start()

    # Run Flask app
    app.run(debug=True, use_reloader=False, host="0.0.0.0",port=5050)
