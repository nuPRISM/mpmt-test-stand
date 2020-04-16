# mPMT Test Stand Software Repository

This repository contains all of the software used in the mPMT Test Stand. For detailed information on the project software architecture please see the [mPMT Test Stand - Software Reference Manual](https://www.hyperk.ca/mpmt/pmt-test-stand/capstone-mpmt-hand-off-documentation/mpmt-test-stand-software-manual.pdf/@@download/file/mPMT%20Test%20Stand%20-%20Software%20Manual.pdf).

This repository includes the software meant to execute on a Host PC running Linux, firmware for the Arduino Due, as well as stand-alone applications for testing.


## Project Structure

This repository contains the following directories:



*   **MessageTerminal**: Command-line application for testing and debugging the Arduino firmware and serial communication software
*   **feArduino: **MIDAS frontend application for managing communication with the Arduino
*   **feScan**: MIDAS frontend application for running/monitoring a scan
*   **firmware**: Arduino Due firmware
*   **PseudoGantry**: Arduino project that emulates the behavior of the gantry (motor drivers, limit switches and encoders)
*   **shared**: Software that is used by both Host PC applications and Arduino firmware
*   **shared_linux**: Software that is used by multiple Host PC applications (but not Arduino firmware)
*   **tools**: Scripts to help with development and build environment setup


## Development Environment Setup


### Operating System

**Host PC Software Development**

The Host PC software must be run under Linux; however, you can still develop on Windows using a virtual machine (not recommended) or the [Windows Subsystem for Linux](https://docs.microsoft.com/en-us/windows/wsl/faq) - WSL (recommended). See the official [installation instructions for WSL](https://docs.microsoft.com/en-us/windows/wsl/install-win10).

**Arduino Firmware Development**

The Arduino Due firmware is organized as a [PlatformIO](https://platformio.org/) project and can be built and flashed from any operating system.


### IDE

We recommend using [VS Code](https://code.visualstudio.com/) as an IDE for both the Host PC software and the Arduino firmware as there is a convenient plugin for PlatformIO that simplifies the process of building and flashing the Arduino firmware. If you cannot or choose not to use VS Code, you can use any text editor to modify the code.


### Build Tools

**Host PC Software**

The Host PC software requires the standard set of C++ build tools for Linux (g++ compiler, make, etc.). These can be installed from your OS’s package repository. For example:

On Ubuntu, run:


```
sudo apt install build-essential
```


On CentOS, run:


```
sudo yum groupinstall 'Development Tools'
```


**Arduino Firmware**

The Arduino firmware must be built using PlatformIO. If you have VS Code installed (recommended), install the [PlatformIO IDE extension for VS Code](https://platformio.org/platformio-ide). If you do not want to use VS Code, you can install the PlatformIO tools independently. Follow the [PlatformIO Core installation instructions](https://docs.platformio.org/en/latest/core/installation.html) or for convenience, you can just run the pio_install.sh script from the tools directory:


```
./tools/pio_install.sh 0
```


You will have to log out and log back in for PATH changes to take effect.

The script will print out its progress to your terminal. If it fails at a particular step, you can re-run the script starting at that step by changing the number passed to the script. Run the script without any arguments to see a list of the different steps.

**CAUTION**: Each installation step can have side-effects and running a step multiple times may have unintended consequences such as adding the same line to your .bashrc multiple times.


### MIDAS

You must have the [MIDAS](https://midas.triumf.ca/MidasWiki/index.php/Main_Page) framework installed in order to build and run either of the MIDAS frontends (feArduino and feScan). If you are purely testing serial communication or the Arduino firmware, you can use the MessageTerminal program which does not require MIDAS to be installed.

**Production / TRIUMF Install**

To install MIDAS on a TRIUMF computer or for production use, follow the [installation instructions in the MIDAS wiki](https://midas.triumf.ca/MidasWiki/index.php/Installation).

**Personal Development Install**

If you are installing MIDAS just for development or testing, you can run the midas_install.sh script from the tools directory (this should work on a real Linux system or within WSL):


```
./tools/midas_install.sh 0
```


You will have to log out and log back in for PATH changes to take effect.

The script will print out its progress to your terminal. If it fails at a particular step, you can re-run the script starting at that step by changing the number passed to the script. Run the script without any arguments to see a list of the different steps.

**CAUTION**: Each installation step can have side-effects and running a step multiple times may have unintended consequences such as adding the same line to your .bashrc multiple times.

**NOTE**: This does not install all of the MIDAS components (in particular ROOT is not installed) although everything required to run the mPMT Test Stand frontends will be installed.

**WARNING**: This produces a completely insecure instance of MIDAS running on your computer. If your computer is publicly accessible on the internet in any way, you should not do this and follow the Production / TRIUMF installation instructions instead.

Once MIDAS is installed, run the web-server in the background using the following command:


```
mhttpd -D
```


You can also start the logger by running:


```
mlogger -D
```


Running the logger is not required and you can save disk space by not running it.

Navigate to [https://localhost:8443](https://localhost:8443) to verify you can access MIDAS. You will have to acknowledge and bypass a warning about an untrusted certificate (since we are using self-signed certificates). The credentials for logging in are:


```
Username = "midas"
Password = "midas"
```



## Building / Flashing / Running

If you haven’t already, clone this repository:


```
git clone git@github.com:nuPRISM/mpmt-test-stand.git ~/online/mpmt-test-stand
```


The directory you clone into is up to you, but a common practice is to clone into the “online” MIDAS directory in your user home directory.

There are 3 steps to building, flashing and running the required software to operate the test stand:



1. Build / Flash the Arduino Firmware
2. Build / Run feArduino
3. Build / Run feScan


### Build / Flash the Arduino Firmware



1. Connect the Arduino Due “Programming” port (they are labelled on the bottom of the board) to your Host PC with a micro-USB cable
2. Determine which serial port the Arduino is connected to:
    1. On Windows:
        1. Right click on Start > Device Manager
        2. The Arduino Due should show up under “Ports (COM & LPT)” as `Arduino Due Programming Port (COM#)`
        3. Take note of the COM number
    2. On Linux:
        4. Run `dmesg` in a terminal
        5. The most recent logs should indicate which serial port device the Arduino Due is attached to (e.g. `/dev/ttyACM0`)
        6. If you can’t tell from the dmesg logs, you can disconnect the Arduino, run `dmesg -w`, reconnect the Arduino and look at the new lines that appear in the logs
3. Open a PlatformIO terminal
    3. In VS Code, hit CTRL+SHIFT+P and search for “PlatformIO: New Terminal”
    4. If you are not using VS Code and have installed PlatformIO Core, just open a regular terminal
4. Run the following command:


```
pio run -t upload --upload-port <port>
```


 where `<port>` is the serial port you noted down earlier (e.g. COM3 on Windows or /dev/ttyACM0 on Linux))


### Build / Run feArduino



1. Open a regular terminal (or a WSL terminal if you are developing on Windows) and navigate to the `feArduino` directory
2. Run `make`
3. Assuming the build was successful, run the following command:


    ```
    ./feArduino.exe <port>
    ```


    where `<port>` is the serial port you noted down earlier



*   On Windows, if the serial port was `COMx`, you should use `/dev/ttySx` here since this is being run from within WSL
4. You should see an output similar to the following:

    ```
    Frontend name          :     feArduino
    Event buffer size      :     2186000
    User max event size    :     1088000
    User max frag. size    :     5242880
    # of events per buffer :     2

    Connect to experiment mpmttest...
    OK
    Init hardware..../feArduino.exe
    /dev/ttyS3
    Waiting for Arduino...Connected!
    Verifying link...SUCCESS
    OK
    ```



    The `Waiting for Arduino...Connected!` and `Verifying link...SUCCESS` messages indicate the feArduino frontend has successfully connected to the Arduino Due over serial.

5. If you are running from within WSL, you must hit `CTRL+D` at this point, otherwise the frontend will fail to communicate with the rest of MIDAS


### Build / Run feScan



1. Open a regular terminal (or a WSL terminal if you are developing on Windows) and navigate to the `feScan` directory
2. Run `make`
3. Assuming the build was successful, run the following command:


```
	./feScan.exe

```



4. You should see an output similar to the following:

    ```
    Frontend name          :     feScan
    Event buffer size      :     2186000
    User max event size    :     1088000
    User max frag. size    :     5242880
    # of events per buffer :     2

    Connect to experiment mpmttest...
    OK
    Init hardware...OK
    ```


5. If you are running from within WSL, you must hit `CTRL+D` at this point, otherwise the frontend will fail to communicate with the rest of MIDAS


## Usage

Once the Arduino is flashed and both MIDAS frontends are running, you can interact with the test stand via the MIDAS web interface.


### MIDAS Custom Pages

The primary user interface is through the scan.html webpage in the feScan directory. See the instructions on the [Custom Pages article in the MIDAS wiki](https://midas.triumf.ca/MidasWiki/index.php/Custom_Page#Access_a_Custom_Page_from_the_Regular_MIDAS_pages) for how to register this webpage with MIDAS so it appears on the sidebar.


### Running a Scan



1. Navigate to the custom Scan page within MIDAS
2. Configure the various parameters of the scan in the “Scan Setup” table
3. Click “Start” and then “Start” again on the new page that loads (optionally adjusting the run number)
4. Navigate back to the “Scan” page
5. You will notice a couple of changes:
    1. The Scan Setup fields are now read-only
    2. The Start button will be disabled and the Stop button will be enabled
    3. The Scan Status table will now report the current progress / state of the scan
    4. The Gantry Position graphic will illustrate the current position of the gantry
6. The gantry will start moving to each position on the grid (generated according to the Scan Setup parameters) and waiting at each position for the configured time
7. The scan will end once all of the points on the grid have been visited
8. You can stop the scan at any time by clicking the Stop button


### Manually Moving the Gantry



1. Navigate the ODB Browser to /Equipment/ARDUINO/Settings
2. Set the Destination to the X and Y coordinates of the destination in mm’s
3. Set the Velocity to the X and Y velocities in mm/s
4. Set MoveRequest to “y”
5. Refresh the page
6. MoveResponse[0] will be “y” and MoveResponse[1] will indicate whether the move request succeeded
7. The current position of the gantry can still be monitored on the Scan page or in the ODB Browser under /Equipment/ARDUINO/Variables/GANT, where GANT[0] is the X coordinate in mm and GANT[1] is the Y coordinate in mm


### Checking Temperature Data



1. Navigate the ODB Browser to /Equipment/ARDUINO/Variables/TEMP
2. The latest temperature readings will be reported here


## Troubleshooting / Debugging


### Building

Unexpected build issues can sometimes be fixed by cleaning the project you’re trying to build and then building again.

The Arduino firmware project can be cleaned by running the following command in a PlatformIO terminal:


```
pio run -t clean
```


The Host PC software can be cleaned by running the following command from the component’s directory:


```
make clean
```



### Flashing

If the Arduino Due fails to flash, make sure that you do not have any serial monitors open on the serial port that the Arduino Due uses. Similarly make sure neither the MessageTerminal nor the Arduino Frontend are running since they also use the serial port.


### Serial Connection Failures

It is possible for the Arduino and Host PC application (either the MessageTerminal or Arduino Frontend) to fail to establish a serial connection if the Arduino has just been connected to the PC and has not been flashed since being disconnected (despite the fact that the firmware is stored in non-volatile memory). The cause for this is unknown but it can be fixed by flashing the firmware after connecting to the computer even if the firmware hasn’t changed.


### MessageTerminal

The MessageTerminal application can be used to test and debug the Arduino firmware and/or the serial communication software without requiring MIDAS. To build the MessageTerminal, navigate to the MessageTerminal directory in a terminal and run `make`. You can run `make clean` to delete all the binary outputs.

To start the program, run the following command:


```
./build/MessageTerminal <port>
```


You can type ? and hit enter to see a list of available commands.

**NOTE: **You must exit the MessageTerminal before trying to flash new firmware to the Arduino since only one program can communicate with the serial port at a time.


### Debugging

Debug messages from the Arduino Firmware can be monitored by connecting a USB-to-serial adapter between the `Serial2` port of the Arduino Due (pins 16 and 17) and your Host PC.

The firmware must be built in `DEBUG` mode to enable the debug messages which can be done by running the following command from a PlatformIO terminal:


```
pio run -e DEBUG -t upload --upload-port <port>
```


By default, the firmware will dump state information once per second to Serial2. This frequency is determined by the number on the first line of mPMTTestStand::execute():


```
PERIODIC(this->debug_dump(), 1000); // Dump state information every 1000 ms
```


If you are modifying the firmware and want to send additional debug messages, you can use the following macros available in the Debug.h header:


```
DEBUG_PRINT(_s)
DEBUG_PRINTLN(_s)
DEBUG_PRINT_VAL(_name, _val)
PROFILE(_name, _thresh_us, _x)
PERIODIC(_x, _t)
```


See Debug.h for more detailed usage information.
