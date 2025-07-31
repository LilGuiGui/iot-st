# iot-st

Compile, via arduino-cli
arduino-cli compile --fqbn esp32:esp32:esp32 iot-st.ino     

Upload, via arduino-cli
note: /dev/ttyUSB0 diganti dengan yang punya USB Slot Win, Linux: /dev/ttyUSB0, Windows: COMx (e.g., COM3, COM4, etc.) < tanya GPT
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 iot-st.ino                                                       

Monitor, via arduino-cli
arduino-cli monitor -p /dev/ttyUSB0  
BaudRATE = 57600

