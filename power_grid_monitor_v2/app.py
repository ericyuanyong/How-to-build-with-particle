# app.py  — Energy‑Monitor mini‑backend
import threading, time, datetime as dt, requests
import datetime
from flask import Flask, render_template, jsonify, request
from flask_sqlalchemy import SQLAlchemy
from sqlalchemy import func

# ────────────────────────────────────────────────────────────────────
# 1.  Flask & DB setup
# ────────────────────────────────────────────────────────────────────
app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = "sqlite:///energy.db"
app.config["SQLALCHEMY_TRACK_MODIFICATIONS"] = False
db = SQLAlchemy(app)

# ────────────────────────────────────────────────────────────────────
# 2.  Model: one row per hourly reading
# ────────────────────────────────────────────────────────────────────
# ────────────────────────────────────────────────────────────────────
# 2.  Model: one row per hourly reading
# ────────────────────────────────────────────────────────────────────
class Reading(db.Model):
    id          = db.Column(db.Integer, primary_key=True)
    timestamp   = db.Column(db.DateTime, index=True, nullable=False)
    energy_kwh  = db.Column(db.Float,  nullable=False)

with app.app_context():
    db.create_all()


# ────────────────────────────────────────────────────────────────────
# 3.  Particle credentials
# ────────────────────────────────────────────────────────────────────
PARTICLE_DEVICE_ID = "0a10aced202194944a04b1a0"
PARTICLE_FUNCTION  = "energy_total"          # endpoint name
PARTICLE_TOKEN     = "ee88804db6c92ced674dc76a81df670f597141d6"
PARTICLE_ARG       = "energy"                # the arg your device expects
POLL_SECONDS       = 3600                    # 1 hour
COST_PER_KWH       = 0.12                    # adjust or move to env var

# ────────────────────────────────────────────────────────────────────
# 4.  Background polling thread
# ────────────────────────────────────────────────────────────────────
def poll_particle_energy():
    """Poll the Particle API every hour to fetch energy usage"""
    while True:
        with app.app_context():  # Ensure you're in the app's context
            try:
                response = requests.post(
                    f"https://api.particle.io/v1/devices/{PARTICLE_DEVICE_ID}/energy_total",
                    data={
                        "arg": "energy",
                        "access_token": PARTICLE_TOKEN
                    }
                )
                data = response.json()
                print(f"Particle API response: {data}")
                
                # Save data to the DB if valid
                if "return_value" in data:
                    energy = data["return_value"]
                    print(f"Energy received: {energy} kWh")
                    
                    reading = Reading(energy_kwh=energy, timestamp=datetime.datetime.now())  # Use datetime
                    db.session.add(reading)
                    db.session.commit()  # Save to DB
                    print(f"Saved energy: {energy} kWh")
                
            except Exception as e:
                print(f"Error: {e}")

        time.sleep(60)  # Wait for 1 hour before next call




# ────────────────────────────────────────────────────────────────────
# 5.  API endpoints
# ────────────────────────────────────────────────────────────────────
@app.route("/api/v1/energy/total")
def energy_total():
    row = Reading.query.order_by(Reading.timestamp.desc()).first()
    if not row:
        return jsonify({"timestamp": None, "energy_total": 0.0})
    return jsonify({"timestamp": row.timestamp.isoformat(),
                    "energy_total": row.energy_kwh})

@app.route('/api/v1/stats/summary')
def stats_summary():
    """Return the summary of energy stats like today, cost, and total usage."""
    try:
        now = datetime.datetime.utcnow()
        today0 = now.replace(hour=0, minute=0, second=0, microsecond=0)
        wnd30  = now - datetime.timedelta(days=30)

        # Aggregate in SQL for speed
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


# ────────────────────────────────────────────────────────────────────
# 6.  Sensor ingest endpoint  (optional if you post manually)
# ────────────────────────────────────────────────────────────────────
@app.post("/api/v1/measurements")
def ingest():
    """
    Allow sensor to POST {"energy_kwh":123.45, "timestamp":"ISO8601"}
    If timestamp omitted, server uses utcnow().
    """
    data  = request.get_json(force=True)
    kwh   = float(data["energy_kwh"])
    ts    = dt.datetime.fromisoformat(data.get("timestamp")) if "timestamp" in data else dt.datetime.utcnow()
    db.session.add(Reading(timestamp=ts, energy_kwh=kwh))
    db.session.commit()
    return {"status": "ok"}, 201


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


# ────────────────────────────────────────────────────────────────────
# 7.  Dashboard route
# ────────────────────────────────────────────────────────────────────
@app.route("/")
def dashboard():
    return render_template("dashboard.html")   # updated template lives in templates/

# ────────────────────────────────────────────────────────────────────
# 8.  Run
# ────────────────────────────────────────────────────────────────────
if __name__ == "__main__":
    with app.app_context():
        db.create_all()

    # Prevent multiple threads: only start once
    import threading
    polling_thread = threading.Thread(target=poll_particle_energy, daemon=True)
    polling_thread.start()

    app.run(debug=True, use_reloader=False, host="0.0.0.0")
