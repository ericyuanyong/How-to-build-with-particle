from flask import Flask, render_template, jsonify
import random, datetime as dt

app = Flask(__name__)

# --- mock datasource ------------------
def demo_last_30_days():
    today = dt.date.today()
    return [
        {"day": (today - dt.timedelta(days=i)).strftime("%Y‑%m‑%d"),
         "kWh": round(random.uniform(2.0, 7.0), 2)}
        for i in reversed(range(30))
    ]

# --- routes ---------------------------
@app.route("/")
def dashboard():
    return render_template("dashboard.html")

@app.route("/api/v1/stats/daily")
def stats_daily():
    return jsonify(demo_last_30_days())

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
