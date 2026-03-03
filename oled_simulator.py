#!/usr/bin/env python3
"""
OLED Display Simulator for Smart Water Bottle
Simulates SSD1306 128x64 OLED display with Flask web interface
"""

from flask import Flask, render_template, jsonify
import datetime

app = Flask(__name__)

# Simulated sensor data
sensor_data = {
    'temperature': 25.3,
    'tds_value': 120,
    'water_level': 2,  # 0=Empty, 1=Low, 2=Medium, 3=Full
    'battery_capacity': 85,
    'time': datetime.datetime.now().strftime("%H:%M:%S"),
    'display_mode': 'normal'  # normal, unlock_digit, unlock_prompt, alarm, locked
}

water_level_str = ['Empty', 'Low', 'Medium', 'Full']

@app.route('/')
def index():
    return render_template('oled_display.html')

@app.route('/api/display_data')
def get_display_data():
    """Return current display data based on mode"""
    mode = sensor_data['display_mode']

    if mode == 'normal':
        return jsonify({
            'mode': 'normal',
            'battery_capacity': sensor_data['battery_capacity'],
            'lines': [
                {
                    'text': f"Temp: {int(sensor_data['temperature'])}.{int((sensor_data['temperature'] - int(sensor_data['temperature'])) * 10)} C",
                    'x': 0,
                    'y': 0,
                    'size': 12
                },
                {
                    'text': f"TDS:  {sensor_data['tds_value']} ppm",
                    'x': 0,
                    'y': 16,
                    'size': 12
                },
                {
                    'text': f"Water: {water_level_str[sensor_data['water_level']]}",
                    'x': 0,
                    'y': 32,
                    'size': 12
                },
                {
                    'text': sensor_data['time'],
                    'x': 0,
                    'y': 48,
                    'size': 16
                }
            ]
        })

    elif mode == 'unlock_digit':
        return jsonify({
            'mode': 'unlock_digit',
            'battery_capacity': sensor_data['battery_capacity'],
            'lines': [
                {
                    'text': 'UNLOCK MODE',
                    'x': 16,
                    'y': 0,
                    'size': 16
                },
                {
                    'text': '7',  # Example digit
                    'x': 56,
                    'y': 24,
                    'size': 16
                }
            ]
        })

    elif mode == 'unlock_prompt':
        return jsonify({
            'mode': 'unlock_prompt',
            'battery_capacity': sensor_data['battery_capacity'],
            'lines': [
                {
                    'text': 'UNLOCK MODE',
                    'x': 16,
                    'y': 0,
                    'size': 16
                },
                {
                    'text': 'Step 2/3',
                    'x': 28,
                    'y': 24,
                    'size': 16
                },
                {
                    'text': 'Press at correct #',
                    'x': 8,
                    'y': 48,
                    'size': 12
                }
            ]
        })

    elif mode == 'alarm':
        return jsonify({
            'mode': 'alarm',
            'battery_capacity': sensor_data['battery_capacity'],
            'lines': [
                {
                    'text': '!! ALARM !!',
                    'x': 16,
                    'y': 8,
                    'size': 16
                },
                {
                    'text': 'LOCKED - 3 FAILS',
                    'x': 4,
                    'y': 32,
                    'size': 12
                },
                {
                    'text': 'Hold KEY2 to reset',
                    'x': 4,
                    'y': 48,
                    'size': 12
                }
            ]
        })

    elif mode == 'locked':
        temp_int = int(sensor_data['temperature'])
        temp_dec = int((sensor_data['temperature'] - temp_int) * 10)
        return jsonify({
            'mode': 'locked',
            'battery_capacity': sensor_data['battery_capacity'],
            'lines': [
                {
                    'text': 'LOCKED',
                    'x': 32,
                    'y': 0,
                    'size': 16
                },
                {
                    'text': f"T:{temp_int}.{temp_dec}  TDS:{sensor_data['tds_value']}",
                    'x': 0,
                    'y': 24,
                    'size': 12
                },
                {
                    'text': sensor_data['time'],
                    'x': 28,
                    'y': 48,
                    'size': 16
                }
            ]
        })

@app.route('/api/set_mode/<mode>')
def set_mode(mode):
    """Switch display mode"""
    if mode in ['normal', 'unlock_digit', 'unlock_prompt', 'alarm', 'locked']:
        sensor_data['display_mode'] = mode
        return jsonify({'success': True, 'mode': mode})
    return jsonify({'success': False, 'error': 'Invalid mode'})

@app.route('/api/update_time')
def update_time():
    """Update current time"""
    sensor_data['time'] = datetime.datetime.now().strftime("%H:%M:%S")
    return jsonify({'time': sensor_data['time']})

if __name__ == '__main__':
    print("=" * 60)
    print("OLED Display Simulator for Smart Water Bottle")
    print("=" * 60)
    print("Open browser and navigate to: http://localhost:5000")
    print("Press Ctrl+C to stop the server")
    print("=" * 60)
    app.run(debug=True, host='0.0.0.0', port=5000)
