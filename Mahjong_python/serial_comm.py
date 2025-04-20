import serial

def send_to_serial(data, port='COM4', baudrate=115200):
    """
    Sends data to a serial port.

    Parameters:
    data (str): The data to send.
    port (str): The serial port to use (default is 'COM3', plz change it depending on your actual port label).
    baudrate (int): The baud rate for the serial communication (default is 115200).
    """
    try:
        ser = serial.Serial(port, baudrate)
        ser.write(data.encode())
        ser.close()
    except serial.SerialException as e:
        print(f"Error opening or writing to serial port: {e}")

