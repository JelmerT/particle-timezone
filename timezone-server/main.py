#
# Particle-timezone server by Jelmer Tiete<jelmer@tiete.be>
#

import logging
import json
import time
import googlemaps
from flask import Flask, request, make_response

import config

EVENT_NAME = "timezone"

logging.basicConfig(format='[%(asctime)s] [%(levelname)s] %(name)s %(message)s',
                            level=logging.INFO)
logging.getLogger().setLevel(logging.INFO)

app = Flask(__name__)


def get_timezone(json_data):
    wifiAccessPoints = []

    # look for access points in json
    for access_point in json_data['data']['a']:
        ap = dict(macAddress=access_point['m'],
                  signalStrength=access_point['s'],
                  channel= access_point['c'])
        wifiAccessPoints.append(ap)
        # app.logger.info(ap)

    gmaps = googlemaps.Client(key=config.maps_api_key)

    location = gmaps.geolocate(consider_ip=False,
                               wifi_access_points=wifiAccessPoints)

    # app.logger.info('%s', json.dumps(location))

    timezone = gmaps.timezone(location=location['location'])

    app.logger.info('%s', json.dumps(timezone))

    #TODO check if response is an OK

    json_tz_response = dict(rawOffset=timezone['rawOffset'],
                            dstOffset=(timezone['dstOffset']))

    return json_tz_response


@app.route('/')
def root():
    return 'Timezone server is running!'


@app.route('/v1/timezone', methods=['POST'])
def post_timezone():

    # check for correct content-type (json)
    if request.is_json:
        json_data = request.get_json()
        app.logger.info('%s', json.dumps(json_data))

        try:
            # Check for particle test-event and reply 200
            if json_data['data'] == "test-event":
                return make_response(json.dumps(json_data), 200)

            elif json_data['event'].endswith(EVENT_NAME):
                json_tz_response = get_timezone(json_data)
                return make_response(json.dumps(json_tz_response),200)

            else:
                return make_response("Malformed request", 400)

        except KeyError:
            return make_response("Malformed request", 400)

    return make_response("Unsupported Content-Type", 415)


@app.errorhandler(500)
def server_error(e):
    app.logger.exception('An error occurred during a request.')
    return """
    An internal error occurred: <pre>{}</pre>
    See logs for full stacktrace.
    """.format(e), 500


@app.errorhandler(404)
def not_found_error(error):
    return make_response("Page not found", 404)


if __name__ == '__main__':
    # This is used when running locally. Gunicorn is used to run the
    # application on Google App Engine. See entrypoint in app.yaml.
    app.run(host='127.0.0.1', port=8080, debug=True)
