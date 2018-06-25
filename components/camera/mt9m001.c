#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "sccb.h"
#include "mt9m001.h"
#include "mt9m001_regs.h"
#include <stdio.h>
#include "esp_log.h"

static const char* TAG = "mt9m001";


static int reset(sensor_t *sensor)
{
	const TickType_t xDelay = 5 / portTICK_PERIOD_MS;
	int ret = 0;
	//Bit 0 needs to be set to 1, then 0 to reset
	uint8_t reg[2];
	uint8_t data = 0;
	if (SCCB_Read16(sensor->slv_addr, RESET, reg) != 0)
	{
		ESP_LOGE(TAG, "Register Read (0x20, RESET) Failed");
	} else {
		reg[0] = reg[0] | 0x00;
		reg[1] = reg[1] | 0x01;
		ret |= SCCB_Write16(sensor->slv_addr, RESET, reg);
		vTaskDelay( xDelay );
// 	}
// 	if(SCCB_Read16(sensor->slv_addr, READ_OPTION_2, reg) != 0)
// 	{
// 		ESP_LOGE(TAG, "Register Read (0x20, READ_OPTION_2) Failed (Reset to ready)");
// 	} else {
		//reg = reg & 0xFFFE;
		SCCB_Read16(sensor->slv_addr, RESET, reg);
		reg[0] = reg[0] & 0xFF;
		reg[1] = reg[1] & 0xFE;
		ret |= SCCB_Write16(sensor->slv_addr, RESET, reg);
	}
	vTaskDelay( xDelay );
	if(SCCB_Read16(sensor->slv_addr, OUTPUT_CONTROL, reg) != 0)
	{
		ESP_LOGE(TAG, "Register Read (0x07, OUTPUT_CONTROL) Failed");
		vTaskDelay( xDelay );
		if(SCCB_Read16(sensor->slv_addr, CHIP_ENABLE, reg) != 0)
		{
			ESP_LOGE(TAG, "Register Read (0xF1, CHIP_ENABLE) Retry Failed");
		} else {
			reg[0] = reg[0] | 0x00;
			reg[1] = reg[1] | 0x01;
			//reg = reg | 0x0001;
			ret |= SCCB_Write16(sensor->slv_addr, CHIP_ENABLE, reg);
		}
	} else {
		reg[0] = reg[0] | 0x00;
		reg[1] = reg[1] | OUPTUT_CONTROL_ENABLE;
		//reg = reg | 0x0001;
		ret |= SCCB_Write16(sensor->slv_addr, OUTPUT_CONTROL, reg);
	}
	vTaskDelay( xDelay );
  	//data = SCCB_Read16(sensor->slv_addr, READ_OPTION_2, reg); //Clear Skip 2
	if(SCCB_Read16(sensor->slv_addr, READ_OPTION_2, reg) != 0)
	{
		ESP_LOGE(TAG, "Register Read (0x20, READ_OPTION_2) Failed");
	} else {
		reg[0] = reg[0] & 0xFF;
		reg[1] = reg[1] | 0x18;
		ret |= SCCB_Write16(sensor->slv_addr, READ_OPTION_2, reg);//*/
	}
	if(SCCB_Read16(sensor->slv_addr, READ_OPTION_2, reg) != 0)
	{
		ESP_LOGE(TAG, "Register Read (0x20, READ_OPTION_2) Failed");
	} else {
		reg[0] = reg[0] & 0xFF;
		reg[1] = reg[1] & 0xE7;
		ret |= SCCB_Write16(sensor->slv_addr, READ_OPTION_2, reg);//*/
	}
	if(SCCB_Read16(sensor->slv_addr, READ_OPTION_1, reg) != 0)
	{
		ESP_LOGE(TAG, "Register Read (0X1E, READ_OPTION_1) Failed");
	} else {
		reg[0] = reg[0] & 0xFF; //Make damned sure column/row skips are disabled. 
		reg[1] = reg[1] & 0xC9;
		ret |= SCCB_Write16(sensor->slv_addr, READ_OPTION_1, reg);//*/
	}
	
	data = SCCB_Read16(sensor->slv_addr, GLOBAL_GAIN, reg);
	//reg = reg & 0xFFFE;
	reg[0] = 0;
	reg[1] = 0x7F;
	ret |= SCCB_Write16(sensor->slv_addr, GLOBAL_GAIN, reg);
	
	return ret;
}

static int set_pixformat(sensor_t *sensor, pixformat_t pixformat)
{
	//Not implemented
	switch (pixformat) {
		case PIXFORMAT_GRAYSCALE:
		case PIXFORMAT_GRAYSCALE10:
			break;
		case PIXFORMAT_RGB565:
		case PIXFORMAT_YUV422:
		default:
			return -1;
	}
	return 0;
}

static int set_framesize(sensor_t *sensor, framesize_t framesize)
{
	unsigned char reg[2];
	int ret =0;
	int data;
	uint16_t w = resolution[framesize][0]-1;
	uint16_t h = resolution[framesize][1]-1;
	if (w > 1280) w = 1280-1;
	if (h > 1024) h = 1024-1;
	w=1280;
	h=1024;
	data = SCCB_Read16(sensor->slv_addr, ROW_SIZE, reg);
	//reg = reg & 0xFFFE;
	reg[0] = w >> 8 & 0xFF;
	reg[1] = w & 0xFF;
	ret |= SCCB_Write16(sensor->slv_addr, ROW_SIZE, reg);
	data = SCCB_Read16(sensor->slv_addr, COLUMN_SIZE, reg);
	//reg = reg & 0xFFFE;
	reg[0] = h >> 8 & 0xFF;
	reg[1] = h & 0xFF;
	ret |= SCCB_Write16(sensor->slv_addr, COLUMN_SIZE, reg);
	//Not implemented

	//Bit 0 needs to be set to 1, then 0 to reset
//	uint8_t reg = SCCB_Read16(sensor->slv_addr, READ_OPTION_2, data);
	return 0;
}

static int set_colorbar(sensor_t *sensor, int enable)
{
	//Not implemented
	return 0;
}

static int set_whitebal(sensor_t *sensor, int enable)
{
	//Not implemented
	return 0;
}

static int set_gain_ctrl(sensor_t *sensor, int enable)
{
	//Not implemented
	unsigned char reg[2];
	int ret = 0;
	int data;
	data = SCCB_Read16(sensor->slv_addr, GLOBAL_GAIN, reg);
	//reg = reg & 0xFFFE;
		reg[0] = 0;
		reg[1] = 0x7F;
		ret |= SCCB_Write16(sensor->slv_addr, GLOBAL_GAIN, reg);
	return 0;
}

static int set_exposure_ctrl(sensor_t *sensor, int enable)
{
//Not implemented
	return 0;
}

static int set_hmirror(sensor_t *sensor, int enable)
{
//Not implemented
	return 0;
}

static int set_vflip(sensor_t *sensor, int enable)
{
	uint8_t reg[2];
	uint8_t data = SCCB_Read16(sensor->slv_addr, READ_OPTION_2, reg);

	// Set mirror on/off
	if (enable) {
		reg[0] = reg[0] | 0x80;
		reg[1] = reg[1] | 0x00;
		//reg = reg | 0x8000;
	}else {
		//reg = reg && 0x7FFF;
		reg[0] = reg[0] & 0x7F;
		reg[1] = reg[1] & 0xFF;
	}
	// Write back register READ_OPTION_2
	return SCCB_Write16(sensor->slv_addr, READ_OPTION_2, reg);
}
int mt9m001_init(sensor_t *sensor)
{
	uint8_t identity[2];
	// Set function pointers
	sensor->reset = reset;
	sensor->set_pixformat = set_pixformat;
	sensor->set_framesize = set_framesize;
	sensor->set_colorbar = set_colorbar;
	sensor->set_whitebal = set_whitebal;
	sensor->set_gain_ctrl = set_gain_ctrl;
	sensor->set_exposure_ctrl = set_exposure_ctrl;
	sensor->set_hmirror = set_hmirror;
	sensor->set_vflip = set_vflip;
	
	// Retrieve sensor's signature
//	sensor->id.MIDH = SCCB_Read(sensor->slv_addr, REG_MIDH);
//	sensor->id.MIDL = SCCB_Read(sensor->slv_addr, REG_MIDL);
//	sensor->id.PID = SCCB_Read(sensor->slv_addr, REG_PID);
//	sensor->id.VER = SCCB_Read(sensor->slv_addr, REG_VER);
	// The only equivilent that the MT9M001 has is Reg0.
	SCCB_Read16(sensor->slv_addr, CHIP_VERSION, identity); //Should be 0x8431
	if (identity[0] != 0x84 || identity[1] != 0x31) {
		printf("Bad return %02x %02x Should be: 0x8431\n", identity[0], identity[1]);
		//return -1;
	} else {
		printf("Good return %02x %02x Should be: 0x8431\n", identity[0], identity[1]);
	}
	sensor->id.MIDH = 0xBA;
	sensor->id.MIDL = 0xBA;
	sensor->id.PID = 0xBA;
	sensor->id.VER = 0xBA;
	
	// Set sensor flags
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_VSYNC, 1);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_HSYNC, 1);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_PIXCK, 1);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_FSYNC, 1);
	SENSOR_HW_FLAGS_SET(sensor, SENSOR_HW_FLAGS_JPEGE, 0);
	
	return 0;
}
