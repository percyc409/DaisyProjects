#pragma once

#include <stdint.h>
#include "daisy_pad.h"

using namespace daisy;

#define MENU_BUTTON_DURATION 2000
#define CONFIRM_DURATION 2000

struct MenuItem {

    const uint32_t label;

    enum class ValueType {
        INT,
        BOOL,
        DISP //Display option. No value to edit
    };

    ValueType type;

    union {
        int32_t int_value;
        bool bool_value;
    };

    int32_t int_min, int_max, int_step;

    // Callback to sync value back to your variable
    std::function<void()> on_commit;
};


MenuItem g_menu_items[] = {
  { 0x555c545c, MenuItem::ValueType::BOOL, .bool_value = false }, //Mono - (Mono Output)
  { 0x55b03974, MenuItem::ValueType::INT, .int_value = 0, .int_min = 0, .int_max = 16, .int_step = 1 }, //MI.Ch - (Midi Channel)
  { 0x7C777878, MenuItem::ValueType::DISP, .bool_value = false } //BAtt - (Display Battery Voltage)
};

constexpr uint8_t MENU_ITEM_COUNT = sizeof(g_menu_items) / sizeof(MenuItem);


class PadMenuSystem {

  public: 
    PadMenuSystem(DaisyPad* pad) {hw = pad;}
    ~PadMenuSystem() {}
    
    enum State {
        IDLE,
        BROWSE,
        EDIT_VALUE,
        CONFIRM
    };

    State m_state = IDLE;

    void update() {

        if (ctrlUpdate) {
            switch(m_state) {
                case IDLE :
                    detectStartCon();
                    break;
                case BROWSE : 
                    handleBrowseInputs();
                    break;
                case EDIT_VALUE:
                    handleEditInputs();
                    break;
                case CONFIRM:
                    handleConfirm();
            }

            ctrlUpdate = false;
        }
    }

    inline void ControlsUpdated() {
        ctrlUpdate = true;
    }

  private: 

    void detectStartCon() {
        // Long press encoder button enters menu
        static uint32_t buttontime = 0;
        static bool pressed = false;
        if (!pressed && hw->encoder.Pressed()) {
            pressed = true;
            buttontime = System::GetNow();
        } else if (pressed && hw->encoder.Pressed()) {
            uint32_t now = System::GetNow();
            if (now - buttontime > MENU_BUTTON_DURATION) {
                pressed = false;
                initdispcode = hw->Get7SegCode();
                m_state = BROWSE;
            }
        } else if (!hw->encoder.Pressed()) {
            pressed = false;
        }
    }
    
    void handleBrowseInputs() {
        if (hw->encoder.Increment() != 0) {
            // Cycle through items
            m_selected_index += (MENU_ITEM_COUNT + hw->encoder.Increment());
            m_selected_index %= MENU_ITEM_COUNT;
            m_current_item = &g_menu_items[m_selected_index];
        }
        
        if (hw->encoder.RisingEdge()) {
            m_state = EDIT_VALUE;
        }

        hw->Set7SegCode(m_current_item->label);
    }
    
    void handleEditInputs() {
        uint32_t code;

        if (hw->encoder.RisingEdge()) {
            // Confirm and exit
            if (m_current_item && m_current_item->on_commit) {
                m_current_item->on_commit();
            }
            m_state = CONFIRM;
        }
        
        // Adjust value based on item type*
        int direction = hw->encoder.Increment();
        
        if (direction != 0) {
            //Adjust different Value types

            switch (m_current_item->type) {
                case MenuItem::ValueType::INT :
                    m_current_item->int_value += direction*m_current_item->int_step;
                    m_current_item->int_value = (m_current_item->int_value < m_current_item->int_min) ? m_current_item->int_min : (m_current_item->int_value > m_current_item->int_max) ? m_current_item->int_max : m_current_item->int_value;
                    break;
                case MenuItem::ValueType::BOOL :
                    m_current_item->bool_value = !m_current_item->bool_value;
                    break;
                case MenuItem::ValueType::DISP : 
                    break;
            }
        }

        switch (m_current_item->type) {
            case MenuItem::ValueType::INT :
                code = number7seg(m_current_item->int_value / 10);
                code = code << 8;
                code += number7seg(m_current_item->int_value % 10);
                hw->Set7SegCode(code);
                break;
            case MenuItem::ValueType::BOOL :
                if (m_current_item->bool_value) {
                    hw->Set7SegCode(0x00005C37); //On
                } else {
                    hw->Set7SegCode(0x005C7171); //Off
                }
                break;
            case MenuItem::ValueType::DISP :
                // Confirm and exit
                if (m_current_item && m_current_item->on_commit) {
                    m_current_item->on_commit();
                }
                m_state = CONFIRM;
                break;
        }
    }

    void handleConfirm() {
        static bool confirmed = false;
        static int confirmTime = 0;

        if (!confirmed) {
            confirmed = true;
            confirmTime = System::GetNow();
            if (m_current_item->type != MenuItem::ValueType::DISP) {
                hw->Set7SegCode(initdispcode);
                hw->SetTemp7SegCode(0x5E5C5479); //donE
            }
        } else {
            uint32_t now = System::GetNow();
            if (now - confirmTime > CONFIRM_DURATION) {
                confirmed = false;
                m_state =  IDLE;
            }
        }
    }

    uint8_t number7seg (int num, bool point = false) {

    uint8_t segcode;

    switch (num) {
        case 0 : segcode = 0b00111111; break;
        case 1 : segcode = 0b00000110; break;
        case 2 : segcode = 0b01011011; break;
        case 3 : segcode = 0b01001111; break;
        case 4 : segcode = 0b01100110; break;
        case 5 : segcode = 0b01101101; break;
        case 6 : segcode = 0b01111101; break;
        case 7 : segcode = 0b00000111; break;
        case 8 : segcode = 0b01111111; break;
        case 9 : segcode = 0b01101111; break;
        default : segcode = 0b00000000;  
    }

    if (point) {
        segcode |= 0x80;
    }

    return segcode;
}

    DaisyPad* hw;
    MenuItem* m_current_item = g_menu_items;
    uint8_t m_selected_index = 0;
    uint32_t initdispcode;
    bool ctrlUpdate = false;
};