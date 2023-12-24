#ifndef __HD44780LCD_H__
#define __HD44780LCD_H__

#include "mbed.h"

class HD44780LCD
        : public Stream
{

    /** the default address of the I2C Peripheral that controls the LCD */
    static constexpr uint8_t DEFAULT_I2C_ADDR	= (0x27<<1);

    class I2CInterface {

        I2C         con;

        uint8_t     addr;
        uint8_t     backlightMask;

    public:

        I2CInterface(PinName I2cSda, PinName I2cScl, uint8_t addr = DEFAULT_I2C_ADDR);

        void    send_byte(uint8_t byte, uint8_t isData);

        void    send_nibble(uint8_t nibble, uint8_t isData);

        void    enable_backlight();

        void    disable_backlight();

        void    toggle_backlight();

        bool    is_backlight_on() const;
    };

    I2CInterface    con;

    uint32_t        cursorLoc;

    uint16_t        displayState {0};
    uint16_t        cursorMovement {0};

public:

    enum EntryMode {

        UNINITIALIZED,
        CURSOR_INC,
        CURSOR_DEC,
        DISPLAY_DEC,
        DISPLAY_INC
    };

    enum CursorMode {

        // the mode is a composition of the two values

        BLINKING_CURSOR = 0x01,
        UNDERLINE_CURSOR = 0x02,
    };

    HD44780LCD() = delete;
    HD44780LCD(const HD44780LCD &) = delete;

    HD44780LCD(PinName i2c_sda, PinName i2c_scl);

    void            initialize();

    // methods to display characters on the LCD

    void            send_data(const uint8_t byte);
    void            send_buffer(const uint8_t *buf, const uint32_t len);
    void            create_custom_char(const uint32_t loc, const uint8_t glyph[8]);

    // method to manage backlight

    void            enable_backlight();
    void            disable_backlight();
    void            toggle_backlight();
    bool            is_backlight_on() const;

    // methods to manage entry mode

    void            set_cursor_auto_dec();
    void            set_cursor_auto_inc();

    void            set_display_auto_dec();
    void            set_display_auto_inc();

    EntryMode       get_entry_mode() const;

    // methods to manage display

    void            clear_display();

    void            enable_display();
    void            disable_display();
    void            toggle_display();
    bool            is_display_enabled() const;

    // methods to manage cursor position

    void            set_cursor_home();
    void            move_cursor_left();
    void            move_cursor_right();

    void            set_cursor_pos(const uint32_t r, const uint32_t c);
    uint32_t        get_cursor_row() const;
    uint32_t        get_cursor_col() const;

    // methods to manage cursor aesthetic

    void            enable_cursor_display();
    void            disable_cursor_display();
    void            toggle_cursor_display();
    bool            is_cursor_displayed() const;

    void            enable_blinking_cursor();
    void            disable_blinking_cursor();
    void            toggle_blinking_cursor();
    bool            is_blinking_cursor_displayed() const;

    // methods to move display

    void            scroll_display_left();
    void            scroll_display_right();

    // methods for getting stream

    HD44780LCD*     get_stream();

    // methods inherited from Stream

    int             _putc(int c) override;
    int             _getc() override;

private:

    void            update_display_cursor_pos();

    void            inc_cursor_loc();
    void            dec_cursor_loc();
};

#endif //__HD44780LCD_H__
