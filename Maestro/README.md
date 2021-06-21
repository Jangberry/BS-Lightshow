# Maestro scripts

This python script is used to simulate what should a beat saber mod do at the end of this projet.
No optimization has been made and it only exists for testing purpose (=pretty dirty code)

## Usage

Before using this script you must create a file `creditentials.py` that contains the following informations:

```python
password="Here is my wonderfull but secret password"
username="Your MQTT username"
host="MQTT hostname"
```

You'll also need the libraries [simpleaudio](https://pypi.org/project/simpleaudio/) and [paho-mqtt](https://pypi.org/project/paho-mqtt/): `pip3 install simpleaudio paho-mqtt`

Then run the command `python3 bs-ligthshow-mqtt <path to the map folder>`

Depending on how your MQTT server is configured you may need to disable TLS: [check paho-mqtt doc](https://pypi.org/project/paho-mqtt/#tls-set)
