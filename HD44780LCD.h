/**
 * @file                    HD44780LCD.h
 * @author                  Aditya Agarwal (aditya.agarwal@dumblebots.com)
 *
 * @brief                   Simple Library to use a 16x2 LCD display driven by the HD44780 driver with MBed OS
 *
 * @copyright               Copyright (c) 2023
 *
 */

#ifndef __HD44780LCD_H__
#define __HD44780LCD_H__

#include "mbed.h"

/**
 * @brief                   Class that provides a simple interface to use a 16x2 LCD driven with the HD44780 driver
 *
 */
class HD44780LCD
        : public Stream
{

    /** the default address of the I2C Peripheral that controls the LCD */
    static constexpr uint8_t DEFAULT_I2C_ADDR	= (0x27<<1);

    /**
     * @brief               Class that provides an interface to use the display via the PC8574 I2C-driven chip
     *
     */
    class I2CInterface {

        /** I2C bus being used to connect to the PC8574 chip */
        I2C         con;

        /** Address of the PC8574 chip on the I2C Bus */
        uint8_t     addr;
        /** Bitmask holding the status of the backlight */
        uint8_t     backlightMask;

    public:

        /**
         * @brief           Construct a new I2CInterface object
         *
         * @param I2cSda    Microcontroller Pin to which the SDA (Serial Data) Pin of the LCD is connected (must be an SDA Pin)
         * @param I2cScl    Microcontroller Pin to which the SCL (Serial Clock) Pin of the LCD is connected (must be an SCL Pin)
         * @param addr      Address of the PC8574 chip on the I2C Bus
         */
        I2CInterface(PinName I2cSda, PinName I2cScl, uint8_t addr = DEFAULT_I2C_ADDR);

        /**
         * @brief           Send a byte (data or instruction) to the LCD (two nibbles in half-bus mode)
         *
         * @param byte      Byte to send (data or instruction indicated by ```isData```)
         * @param isData    Whether the byte was data or an instruction
         */
        void    send_byte(uint8_t byte, uint8_t isData);

        /**
         * @brief           Send a nibble (data or instruction) to the LCD
         *
         * @param nibble    Nibble to send (data or instruction indicated by ```isData```)
         * @param isData    Whether the byte was data or an instruction
         */
        void    send_nibble(uint8_t nibble, uint8_t isData);

        /**
         * @brief           Enable the LCD's backlight
         *
         */
        void    enable_backlight();

        /**
         * @brief           Disable the LCD's backlight
         *
         */
        void    disable_backlight();

        /**
         * @brief           Toggle the LCD's backlight
         *
         */
        void    toggle_backlight();

        /**
         * @brief           Check the state of the LCD's backlight
         *
         * @return          true if the backlight is switched on, false otherwise
         */
        bool    is_backlight_on() const;
    };

    /** Interface to communicate with the LCD */
    I2CInterface    con;

    /** Location of the cursor in the DDRAM of the LCD */
    uint32_t        cursorLoc;

    /** Bitmask containing the enabled/disabled states of the display, underline cursor and blinking cursor */
    uint16_t        displayState {0};
    /** Bitmask containing the entry mode of the LCD */
    uint16_t        cursorMovement {0};

public:

    /**
     * @brief               Different types of movements the cursor/display can perform after printing a character
     *
     */
    enum EntryMode {

        UNINITIALIZED,
        CURSOR_INC,
        CURSOR_DEC,
        DISPLAY_DEC,
        DISPLAY_INC
    };

    /**
     * @brief               Different types of cursors that can be displayed
     *
     */
    enum CursorMode {

        // the mode is a composition of the two values

        BLINKING_CURSOR = 0x01,
        UNDERLINE_CURSOR = 0x02,
    };

    HD44780LCD() = delete;

    HD44780LCD(const HD44780LCD &) = delete;

    /**
     * @brief               Construct a new HD44780LCD object
     *
     * @param i2c_sda       Microcontroller Pin to which the SDA (Serial Data) Pin of the LCD is connected (must be an SDA Pin)
     * @param i2c_scl       Microcontroller Pin to which the SCL (Serial Clock) Pin of the LCD is connected (must be an SCL Pin)
     *
     */
    HD44780LCD(PinName i2c_sda, PinName i2c_scl);

    /**
     * @brief               Initializes the LCD by running its initialization sequence
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            initialize();

    // methods to display characters on the LCD

    /**
     * @brief               Send a single byte of data (character) to the LCD to display
     *
     * @remark              This method alters the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     * @param byte          Character to send to be displayed
     *
     */
    void            send_data(const uint8_t byte);

    /**
     * @brief               Send an array of bytes (characters) to the LCD to display
     *
     * @remark              This method alters the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     * @param buf           Pointer to array of characters
     * @param len           Number of characters to pick from the buffer
     */
    void            send_buffer(const uint8_t *buf, const uint32_t len);

    /**
     * @brief               Create a glyph for a custom character in the LCD's memory
     *                      See Example section for usage
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              The location of the character must be between 0 and 8 (exclusive), values outside
     *                      this range will cause undefined behaviour
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     * @example             To create a smiley face and display it
     * @code
     * uint8_t smiley[8] = {
     *             0b00000,
     *             0b01010,
     *             0b01010,
     *             0b01010,
     *             0b00000,
     *             0b10001,
     *             0b01110,
     *             0b00000
     * };
     *
     * lcd.create_custom_char(7, smiley);
     * lcd.send_data(7);
     * lcd.send_buffer("\x07\x07\x07", 3); // prints 3 more smiley faces
     * @endcode
     *
     * @param loc           Location (between 0 and 8 exclusive) in CGRAM to store glyph
     * @param glyph         Pattern of glyph
     *
     */
    void            create_custom_char(const uint32_t loc, const uint8_t glyph[8]);

    // method to manage backlight

    /**
     * @brief               Enable the LCD's backlight
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            enable_backlight();

    /**
     * @brief               Disable the LCD's backlight
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            disable_backlight();

    /**
     * @brief               Toggle the LCD's backlight
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            toggle_backlight();

    /**
     * @brief               Check the state of the LCD's backlight
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return              true if the backlight is switched on, false otherwise
     */
    bool            is_backlight_on() const;

    // methods to manage entry mode

    /**
     * @brief               Set the cursor to automatically decrement after printing a character
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            set_cursor_auto_dec();

    /**
     * @brief               Set the cursor to automatically increment after printing a character
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            set_cursor_auto_inc();

    /**
     * @brief               Set the display to automatically scroll left after printing a character
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     * @warning             Setters/Getters for cursor position do not work in a well-defined way after calling this method
     *
     */
    void            set_display_auto_dec();

    /**
     * @brief               Set the display to automatically scroll right after printing a character
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     * @warning             Setters/Getters for cursor position do not work in a well-defined way after calling this method
     *
     */
    void            set_display_auto_inc();

    /**
     * @brief               Get the entry mode of the LCD
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return EntryMode    The direction the cursor moves/display scrolls in after a character is printed
     *                      as set using either of the following methods
     *                      ```HD44780LCD::set_cursor_auto_dec()```
     *                      ```HD44780LCD::set_cursor_auto_inc()```
     *                      ```HD44780LCD::set_display_auto_dec()```
     *                      ```HD44780LCD::set_display_auto_inc()```
     */
    EntryMode       get_entry_mode() const;

    // methods to manage display

    /**
     * @brief               Clear the display
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            clear_display();

    /**
     * @brief               Enable the display (starts displaying characters without mutating memory)
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::disable_display()```
     * @remark              See also ```HD44780LCD::toggle_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            enable_display();

    /**
     * @brief               Disable the display (hides characters without mutating memory)
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::enable_display()```
     * @remark              See also ```HD44780LCD::toggle_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            disable_display();

    /**
     * @brief               Toggle the display (show or hide characters being shown without mutating memory)
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::enable_display()```
     * @remark              See also ```HD44780LCD::disable_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            toggle_display();

    /**
     * @brief               Check whether the display is enabled or not
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return              true if the display is enabled, false otherwise
     */
    bool            is_display_enabled() const;

    // methods to manage cursor position

    /**
     * @brief               Set the cursor to the home position
     *
     * @remark              This method alters the cursor position
     */
    void            set_cursor_home();

    /**
     * @brief               Move the cursor one position to the left (wrap-around to previous line if reached start-of-line)
     *
     * @remark              This method alters the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            move_cursor_left();

    /**
     * @brief               Move the cursor one position to the right (wrap-around to next line if reached end-of-line)
     *
     * @remark              This method alters the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            move_cursor_right();

    /**
     * @brief               Set the position of the cursor on the display
     *
     * @remark              This method alters the cursor position
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     * @param r             Row to which the cursor should be moved
     * @param c             Column to which the cursor should be moved
     */
    void            set_cursor_pos(const uint32_t r, const uint32_t c);

    /**
     * @brief               Get the row in which the cursor is situated
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return uint32_t     Row in which the cursor is situated
     */
    uint32_t        get_cursor_row() const;

    /**
     * @brief               Get the column in which the cursor is situated
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return uint32_t     Column in which the cursor is situated
     */
    uint32_t        get_cursor_col() const;

    // methods to manage cursor aesthetic

    /**
     * @brief               Enable displaying the underline cursor
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::disable_cursor_display()```
     * @remark              See also ```HD44780LCD::toggle_cursor_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            enable_cursor_display();

    /**
     * @brief               Disable displaying the underline cursor
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::enable_cursor_display()```
     * @remark              See also ```HD44780LCD::toggle_cursor_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            disable_cursor_display();

    /**
     * @brief               Toggle displaying the underline cursor
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::enable_cursor_display()```
     * @remark              See also ```HD44780LCD::disable_cursor_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            toggle_cursor_display();

    /**
     * @brief               Check if displaying the underline cursor is enabled
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return              true if displaying the underline cursor is enabled, false otherwise
     */
    bool            is_cursor_displayed() const;

    /**
     * @brief               Enable displaying the blinking cursor
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::disable_blinking_display()```
     * @remark              See also ```HD44780LCD::toggle_blinking_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            enable_blinking_cursor();

    /**
     * @brief               Disable displaying the blinking cursor
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::enable_blinking_display()```
     * @remark              See also ```HD44780LCD::toggle_blinking_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            disable_blinking_cursor();

    /**
     * @brief               Toggle blinking the blinking cursor
     *
     * @remark              This method does not alter the cursor position
     *
     * @remark              See also ```HD44780LCD::enable_blinking_display()```
     * @remark              See also ```HD44780LCD::disable_blinking_display()```
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            toggle_blinking_cursor();

    /**
     * @brief               Check if displaying the blinking cursor is enabled
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           This method can be called from ISR context
     *
     * @return              true if displaying the blinking cursor is enabled, false otherwise
     */
    bool            is_blinking_cursor_displayed() const;

    // methods to move display

    /**
     * @brief               Scroll the contents of display one position to the left
     *
     * @warning             This method scrolls the display contents to the left without moving the cursor's location,
     *                      but the position changes as the cursor is also moved to the left
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            scroll_display_left();

    /**
     * @brief               Scroll the contents of the display one position to the right
     *
     * @warning             This method scrolls the display contents to the right without moving the cursor's location,
     *                      but the position changes as the cursor is also moved to the right
     *
     * @attention           Can not call this method from ISR context
     * @attention           It is unsafe to call this method from multiple threads concurently
     *
     */
    void            scroll_display_right();

    // methods for getting stream

    /**
     * @brief               Get a pointer to the underlying stream for using with file stream APIs
     *
     * @remark              This method does not alter the cursor position
     *
     * @attention           It is safe to call this method from ISR context
     *
     * @return HD44780LCD*  Pointer to underlying stream
     */
    HD44780LCD*     get_stream();

    // methods inherited from Stream

    int             _putc(int c) override;
    int             _getc() override;

private:

    /**
     * @brief               Update the position of the cursor in the display to match the stored value
     *
     * @attention           Can not call this method from ISR context
     */
    void            update_display_cursor_pos();

    /**
     * @brief               Increment the position of the cursor while handling wrap-around
     *
     */
    void            inc_cursor_loc();

    /**
     * @brief               Decrement the position of the cursor while handling wrap-around
     *
     */
    void            dec_cursor_loc();
};

#endif //__HD44780LCD_H__
