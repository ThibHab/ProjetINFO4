import serial
import mido

com_port = '/dev/ttyACM0'
baud_rate = 9600

arduino = serial.Serial(com_port, baud_rate, timeout=1)
outport = mido.open_output('Python MIDI outport', virtual=True)

previous_data = 0

def read_serial_data():
    return arduino.readline().decode('utf8', errors='ignore').strip()

def process_data(data):
    try:
        cut_data = int(data) - 950
        adjusted_data = cut_data * (1023 / (1023 - 950))
        previous_data = adjusted_data
        voltage = (adjusted_data / 1023) * 5

        if voltage >= 4.9:
            voltage = 4.9
        if voltage <= 0.1:
            voltage = 0.1

        print("Raw voltage value : ", voltage)

        return voltage
    except ValueError:
        print("Invalid data")
        return previous_data

def convert_to_MIDI(data):
    if data:
        print("display data for error : ", data)
        data = int(data * 127 / 5)
        print("MIDI converted voltage : ", data)
    MIDI_message = mido.Message('control_change', control=3, value=data)
    outport.send(MIDI_message)
    print("Sent MIDI message : ", MIDI_message)
    

if __name__ == "__main__":
    while True:
        data = read_serial_data()

        if data:
            processed_data = process_data(data)
            convert_to_MIDI(processed_data)
