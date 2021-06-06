# Maestro scripts

Theses python scripts are used to simulate what should a beat saber mod do at the end of this projet.
No optimization has been made and they only exist for testing purpose (=pretty dirty code)

The serial one was just a fun test and most likely won't be updated

## Usage

Before using this script you must create a file `creditentials.py` that contains the following informations:

```python
password="Here is my wonderfull but secret password"
username="Your MQTT username"
host="MQTT hostname"
```

You'll also need the library simpleaudio: `pip3 install simpleaudio`

Then run the command `python3 bs-ligthshow-mqtt <path to the map folder>`
