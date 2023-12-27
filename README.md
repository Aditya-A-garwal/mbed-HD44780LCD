# mbed-HD44780LCD

![GitHub License](https://img.shields.io/github/license/Aditya-A-garwal/mbed-HD44780LCD)
![GitHub forks](https://img.shields.io/github/forks/Aditya-A-garwal/mbed-HD44780LCD?style=flat-square&color=blue)
![GitHub Repo stars](https://img.shields.io/github/stars/Aditya-A-garwal/mbed-HD44780LCD?style=flat-square&color=blue)
![GitHub issues](https://img.shields.io/github/issues-raw/Aditya-A-garwal/mbed-HD44780LCD?style=flat-square&color=indianred)
![GitHub closed issues](https://img.shields.io/github/issues-closed-raw/Aditya-A-garwal/mbed-HD44780LCD?style=flat-square)
![GitHub pull requests](https://img.shields.io/github/issues-pr/Aditya-A-garwal/mbed-HD44780LCD?style=flat-square&color=indianred)
![GitHub closed pull requests](https://img.shields.io/github/issues-pr-closed/Aditya-A-garwal/mbed-HD44780LCD?style=flat-square)

## Overview

This repository contains a simple library to use a **16x2 character LCD** with MBed OS. A 16x2 character LCD is a useful tool for printing debug messages and displaying information about a system. The library provides a complete interface for the display, that includes controlling the cursor movement/position and style, scrolling the display and using power saving methods such as disabling the screen. The library also enables the **printf family of functions** to be used with the LCD directly, allowing **formatted output**.

## Usage

The process to use the library in your own *MBed CLI/MBed CE* projects consists of a few simple steps. This section assumes that CMake is being used as the build system. For other build systems/IDEs, use the source and header files directly and refer to their manuals.

The steps to use the library in your own projects are shown below -

1. Create a file named ```external/CMakeLists.txt``` in your project root with the following content. This will fetch the repository and add its targets to the project.

    ```diff
    FetchContent_Declare(mbed-HD44780LCD
        GIT_REPOSITORY
            https://github.com/Aditya-A-garwal/mbed-HD44780LCD.git
        GIT_TAG
            latest
    )

    FetchContent_MakeAvailable(mbed-HD44780LCD)
    ```

    ```latest``` after ```GIT_TAG``` uses the latest commit to the main branch of this repository. Replace it with the hash of the commit or the tag of the release that needs to be used in your project.

    More information about the FetchContent set of commands can be found [in the official CMake Docs](https://cmake.org/cmake/help/latest/module/FetchContent.html).

2. Add the following line to to the ```CMakeLists.txt``` in your project root. This will add the ```external``` directory to your project. Make sure to insert it before creating the ```APP_TARGET``` executable and after including the MBed OS directory.

    ```diff
    add_subdirectory(${MBED_PATH})

    + add_subdirectory(external)

    add_executable(${APP_TARGET}
        main.cpp
    )
    ```

3. Link the library with APP_TARGET (or other targets as required) by adding updating the following line -

    ```diff
    - target_link_libraries(${APP_TARGET} mbed-os)
    + target_link_libraries(${APP_TARGET} mbed-os mbed-HD44780LCD)
    ```

    This also updates the include path for the linked targets.

4. Configure and build the project by running the following commands in the root-directory of the project. This fetches the repository and makes the code available to the intellisense and autocomplete features in most IDEs.

    ```bash
    # Configure (fetches the repository and prepares the build-system)
    mbed-tools configure --toolchain <TOOLCHAIN> --mbed-target <TARGET_NAME> --profile <PROFILE>
    cmake -S . -B cmake_build/<TARGET_NAME>/<PROFILE>/<TOOLCHAIN>

    # Builds the code
    cmake --build cmake_build/<TARGET_NAME>/<PROFILE>/<TOOLCHAIN>
    ```

    Make sure to replace ```<TOOLCHAIN>```, ```<TARGET_NAME>``` and ```<PROFILE>``` with the appropriate
    values.

    **To change the version of the library being used, update the ```GIT_TAG``` parameter in ```external/CMakeLists.txt``` and re-run the configure and build commands. Re-building the project is not enough as the ```FetchContent_Declare``` command fetches the library while configuring the project only, and not while building.**

5. Include the header files in ```main.cpp``` (and other files as required) -

    ```cpp
    #include "HD44780LCD.h"
    ```

## Organization of the Library

The library contains a single header file - ```HD44780LCD.h``` and its corresponding source file - ```HD44780LCD.cpp```. The library provides a single class - ```HD44780LCD``` to use the display.

The steps to use the display are as follows -

1. Instantiate the ```HD44780LCD``` class.
2. Initialize the object by calling the ```initialize()``` method.
3. Enable the backlight by calling the ```enable_backlight()``` method.
3. Send data to the display by calling the ```send_data(byte)```, ```send_buffer(len```, buf) and ```printf(fmt_string, *args)``` methods.

For the complete list of methods provided by the class, navigate to the ```HD44780LCD.h``` header file. To override the default stream for printf, add the following code before the ```main``` function. This function is called automatically by MBed OS before starting your application.

```cpp
HD44780LCD lcd(sda_pin, scl_pin);

FileHandle *
mbed::mbed_override_console(int fd) {

    // The get_stream method automatically initializes the LCD and returns a pointer to the underlying stream

    // If the LCD object is declared within this function, then the static lifetime specifier must be used to prevent it from being destructed
    // static HD44780LCD lcd(sda_pin, scl_pin);

    if (fd == STDOUT_FILENO) {
        return lcd.get_stream();
    }
}
```

Detailed information is available as inline documentation within the header files.

## Known Limitations

The following limitations exist within the library, with no known timeline to fix it -

- The methods for getting/setting the cursor position only work in a defined manner as long as the display is not scrolled.
- The Library presently only supports 16x2 character LCDs. 20x4 LCDs use the same driver (and are compatible) but are not presently not supported.
- The library presently only supports interfacing with the display via I2C, using the PC8574 I2C driver. Directly interfacing with the HD44780 Bus in half-bus or full-bus mode is presently not supported.

## Documentation

The ```.h``` header files contain inline documentation for all classes, structs, functions and enums within it. This repository uses the Doxygen standard for inline-documentation. Regular comments explaining implementation details can be found in the ```.cpp``` source files.
