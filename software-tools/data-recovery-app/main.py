"""
Author: Edwin Mwiti Maingi
Email: emwiti658@gmail.com
Date: 14th June 2024, 5.20pm

This program is a tool for recovering flight data from external SPI Flash memory
via UART

See the FlightSoftware README for more details on usage

"""

from tkinter import *
import customtkinter as ctk

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
    "921600"
    ]

class App(ctk.CTk):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.title("Nakuja Flight Data Recovery App V"+version)
        self.geometry('600x300')

        self.createWidgets()

    def createWidgets(self):
        # port label
        self.port_label = ctk.CTkLabel(self, text = 'Port:');
        self.port_label.grid(row = 0, column = 0,
                             padx = 20, pady = 20,
                             sticky = "ew")

        # port selection combo box
        self.port_select = ctk.CTkOptionMenu(self, values = ['COM1', 'COM5']);
        self.port_select.grid(row = 0, column = 1,
                             padx = 20, pady = 20,
                             sticky = "ew")

        # baud rate label
        self.baud_label = ctk.CTkLabel(self, text = 'Baud rate:');
        self.baud_label.grid(row = 1, column = 0,
                             padx = 20, pady = 20,
                             sticky = "ew")

        # baud rate selection combo box
        self.baud_select = ctk.CTkOptionMenu(self, values = bauds);
        self.baud_select.grid(row = 1, column = 1,
                             padx = 20, pady = 20,
                             sticky = "ew")

        
        
        self.mybutton = ctk.CTkButton(self, text = 'DUMP', font=("Inter", 14))
        self.mybutton.grid(row = 2, column = 1,
                           padx = 10, pady = 10)


if __name__ == "__main__":
    window = App()
    window.mainloop()
