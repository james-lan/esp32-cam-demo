#ifndef __REG_REGS_H__
#define __REG_REGS_H__

//Read Only: Default 0x8431
#define CHIP_VERSION 0x00

//Start row, defaults to row 12 (1st light pixel row)
#define ROW_START 0x01
//Start cloumn, defaults to column 20 (1st light pixel cloumn, must be even)
#define COLUMN_START 0x02
//Row size - 1 (Default 1023)
#define ROW_SIZE 0x03 
//Column size -1 (Default 1279)
#define COLUMN_SIZE 0x04

//Blanking time (See datasheet)
#define HORIZONTAL_BLANKING 0x05
#define VERTICAL_BLANKING 0x06

//Output Control:
//Modifiable: 0, 1, 6 
//All modifiable = 0 by default
//0: Do not update, until set to 0 (Also 0xF1, bit 1)
//1: Chip Enable (Also 0xF1, bit 0)
//6: Use Test Data (Test data will be output)
#define OUTPUT_CONTROL 0x07
#define OUPTUT_CONTROL_SUSPEND 0x01
#define OUPTUT_CONTROL_ENABLE 0x02
#define OUPTUT_CONTROL_TEST_DATA 0x06


//Timing: Number of rows of integration
//INT = Reg0x09 x row time - overhead time - reset delay, where:
//Row time = ((Reg0x04 + 1) + 244 + Reg0x05 - 19) pixel clock periods
//Overhead time = 180 pixel clock periods
// Reset Delay = Reset delay = 4 x Reg0x0C (SHUTTER_DELAY) pixel clock periods

#define SHUTTER_WIDTH 0x09
#define SHUTTER_DELAY 0X0C


#define RESET 0X0D

//Setting bit 0 to 1 will cause the sensor to restart from the 0,0 (Upper left position) 
#define RESTART 0x0B

//Bits:
//Modifiable: 2, 3, 4, 5, 8, 9, 10, 11
// 2: Column skip 4
// 3: Row skip 4
// 4: Column skip 8
// 5: Row skip 8 
// 8: Snapshot mode: Wait for Trigger pin, or 1 in bit 0 of RESTART Register
// 9: Strobe Enable (Default: No strobe signal) 
//10: Strobe signal on when rows integrating
//11: Strobe Override (set it manually here, bit 9 should be 0) 
#define READ_OPTION_1 0X1E

//Bits:
//Modifiable: 0, 3, 4, 7, 9,10,15
//Default on all modifiable bits is 0
//15: Mirror rows (flip bottom to top)
//10: Continuous line valid xor frame valid
// 9: Continuous line valid (on during VERTICAL_BLANKING) 
// 7: Flip rows (Useless for Monochrome, read row+1, row then row+3, +2, etc. Good for Bayer filtered)
// 4: Row skip (Read 2, skip 2) 
// 3: Column skip (Read 2, skip 2)
// 0: Output frames after change
#define READ_OPTION_2 0X20
#define READ_OPTION_2_FLIP 0x8000
#define READ_OPTION_2_SKIP_2 0x0018

//Black adjustment: Just read the sheet. 
#define CAL_THRESHOLD 0x5F

//Black Level Control 
//16bit: settable 0-2, 11, 12, 15
//All settable: 0 normal
//15: No rapid black sweep on new gain
//12: Manual black level sweep 
//11: Do not reset black level after calculation
//2-1: 00: Normal, black level when ADC running
//2-1: 01: Continuously applied
//2-1: x1: Black level offset
//0: Override black level correction
#define CAL_CTRL 0x62

// 1= Normal Operation, 0=Stop
//Also 0x07 Bit 1
#define CHIP_ENABLE 0xF1

//Gain Settings: 
//Gain default 0x100 = 8 = 1x gain
//If Gain =< 8 Gain = (bit[6] + 1) x (bit[5-0] x 0.125)
//If Gain > 8 Gain = 8.0 + bit[2-0]
#define GLOBAL_GAIN 0x35
//Setting GLOBAL_GAIN sets all 4 registers below:
#define EVEN_ROW_EVEN_COLUMN 0x2B
#define ODD_ROW_EVEN_COLUMN 0x2C
#define EVEN_ROW_ODD_COLUMN 0x2D
#define ODD_ROW_ODD_COLUMN 0x2E
//Analog offset corrections (Black adjustment) 
#define EVEN_ROW_EVEN_COLUMN_2 0x60
#define ODD_ROW_EVEN_COLUMN_2 0x61
#define EVEN_ROW_ODD_COLUMN_2 0x63
#define ODD_ROW_ODD_COLUMN_2 0x64

#endif //__REG_REGS_H__
