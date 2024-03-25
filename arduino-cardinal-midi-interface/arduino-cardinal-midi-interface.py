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
    serial_output = data.split(',')
    if len(serial_output) == 2 and serial_output[0] != '' and serial_output[1] != '':
        cut_data = int(serial_output[1]) - 950
        scaled_data = cut_data * (1023 / (1023 - 950))
        previous_data = scaled_data
        voltage = (scaled_data / 1023) * 5

        if voltage >= 4.9:
            voltage = 4.9
        if voltage <= 0.1:
            voltage = 0.1

        processed_data = {}
        processed_data['pin'] = serial_output[0]
        processed_data['voltage'] = voltage

        return processed_data
    else:
        return 0

def convert_to_MIDI_and_send(data):
    data['voltage'] = int(data['voltage'] * 127 / 5)

    match data['pin']:
        case '0':
            MIDI_message = mido.Message('control_change', control=1, value=data['voltage'])
        case '1':
            MIDI_message = mido.Message('control_change', control=2, value=data['voltage'])
        case '2':
            MIDI_message = mido.Message('control_change', control=3, value=data['voltage'])
        case '3':
            MIDI_message = mido.Message('control_change', control=4, value=data['voltage'])
        case other:
            print("No match found")

    outport.send(MIDI_message)
    

if __name__ == "__main__":
    while True:
        serial_output = read_serial_data()

        if serial_output:
            processed_data = process_data(serial_output)
            if processed_data:
                convert_to_MIDI_and_send(processed_data)