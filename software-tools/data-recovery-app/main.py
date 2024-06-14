"""
Author: Edwin Mwiti Maingi
Email: emwiti658@gmail.com
Date: 14th June 2024, 5.20pm

This program is a tool for recovering flight data from external SPI Flash memory
via UART

See the FlightSoftware README for more details on usage

"""

import tkinter as tk
import customtkinter as ctk
import sys
import glob
import serial

# version #
version = "1.0"

# default theme
ctk.set_appearance_mode("system");

# baud rates
bauds = [
    "4800",
    "9600",
    "19200",
    "38400",
    "57600",
    "115200",
    "230400",
    "460800",
    "921600"]

class App(ctk.CTk):
    def __init__(self, *args, **kwargs):
        """
        Init class
        """
        super().__init__(*args, **kwargs)
        self.title("Nakuja Flight Data Recovery Tool V"+version)
        self.geometry('450x400')
        self.resizable(False, False)

        # scan the device for serial ports
        self.ports = self.scanSerial()

        self.createWidgets()

    def scanSerial(self):
        """
        scans the system for available com ports and returns a list of the ports 
        """
        if sys.platform.startswith('win'):
            ports = ['COM%s' % (i+1) for i in range(256)]
        elif sys.platform.startswith('linux') or sys.platform.startswith('cygwin'):
            # exclude the current terminal 
            ports = glob.glob('/dev/tty[A-Za-z]*')
        elif sys.platform.startswitch('darwin'):
            ports = glob.glob('/dev/tty.*')
        else:
            raise EnvironmentError('Unsupported platform')

        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass
        
        return result

    def selectFile(self):
        """
        Allow user to select output csv file
        """
        filename = ctk.filedialog.askopenfilename()
        self.output_file_entry.insert(0, filename)
        print(filename)

    def dump(self):
        """
        Copy data from flash memory to the output file
        """
        # check for user selections
        self.port = self.port_select.get()
        self.baud = self.baud_select.get()
        self.output_file = self.output_file_entry.get()

        if self.output_file == "":
            self.output_file = "flight.csv"

        print(self.port);
        print(self.baud)
        print(self.output_file)

        # connect to serial
        try:
            self.ser = serial.Serial(self.port)
            if self.ser:
                print("Connected to " + self.port)
                self.serial_connection_label.configure(text = "Connected to " + self.port)
            else: 
                print("Could not connect to " + self.port)
        except serial.SerialException:
            print("Could not connect to serial port")
        

    def createWidgets(self):
        """
        Create window widgets
        """
        # port label
        self.port_label = ctk.CTkLabel(self, text = 'Port:');
        self.port_label.grid(row = 0, column = 0,
                             padx = 20, pady = 20,
                             sticky = "ew")

        # port selection combo box
        self.port_select = ctk.CTkOptionMenu(self, values = self.ports);
        self.port_select.grid(row = 0, column = 1,
                             padx = 20, pady = 20,
                             sticky = "ew")

        # baud rate label
        self.baud_label = ctk.CTkLabel(self, text = 'Baud rate:');
        self.baud_label.grid(row = 1, column = 0,
                             padx = 20, pady = 20,
                             sticky = "ew")

        # baud rate selection combo box
        self.baud_select = ctk.CTkOptionMenu(self, values = bauds)
        self.baud_select.grid(row = 1, column = 1,
                             padx = 20, pady = 20,
                            sticky = "ew")

        # output file selection
        self.output_file_label = ctk.CTkLabel(self, text = "Output file:")
        self.output_file_label.grid(row = 2, column = 0,
                                    padx = 20, pady = 20)

        # output file selection entry box                 
        self.output_file_entry = ctk.CTkEntry(self, placeholder_text="flight.csv")
        self.output_file_entry.grid(row = 2, column = 1,
                                     padx = 20, pady = 20,
                                    sticky = "ew")
        self.output_select_button = ctk.CTkButton(self, text = "Open", command=self.selectFile)
        self.output_select_button.grid(row = 2, column = 2,
                                       pady = 20, ipadx=1,
                                        sticky = "ew")

        # dump button
        self.mybutton = ctk.CTkButton(self, text = 'DUMP', font=("Inter", 14), command = self.dump)
        self.mybutton.grid(row = 4, column = 1,
                           padx = 10, pady = 10)

        # status labels 
        self.serial_connection_label = ctk.CTkLabel(self, text="")
        self.serial_connection_label.grid(row = 5, column = 0, columnspan = 3, 
                                          pady = 20, padx = 20)


if __name__ == "__main__":
    window = App()
    window.mainloop()
