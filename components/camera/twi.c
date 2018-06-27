/*
  si2c.c - Software I2C library for ESP31B

  Copyright (c) 2015 Hristo Gochkov. All rights reserved.
  This file is part of the ESP31B core for Arduino environment.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <stdint.h>
#include <stdbool.h>
#include "twi.h"
#include "soc/gpio_reg.h"
#include "wiring.h"
#include <stdio.h>


#define USE_NATIVE_I2C

#ifdef USE_NATIVE_I2C
#include "driver/i2c.h"

#define I2C_FREQ 170000
#define I2C_PORT 1
#define ACK_CHECK_EN 0x1
#define ACK_VAL 0x0
#define NACK_VAL 0x1 

void twi_init(unsigned char sda, unsigned char scl){
	int i2c_master_port = I2C_PORT;
	i2c_config_t conf;
	conf.mode = I2C_MODE_MASTER;
	conf.sda_io_num = sda;
	conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
	conf.scl_io_num = scl;
	conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
	conf.master.clk_speed = I2C_FREQ;
	i2c_param_config(i2c_master_port, &conf);
	i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
	
}
void twi_stop(void) {
// 	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
// 	i2c_master_stop(cmd);
// 	esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
// 	i2c_cmd_link_delete(cmd);
	return;
	//i2c_master_stop ? 
}
void twi_setClock(unsigned int freq){
	return;
	//Going to ignore this for now, as it's set as a define, at initilization
}
uint8_t twi_writeTo(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop) {
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( address /*<< 1*/ ) | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write(cmd, buf, len, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}
uint8_t twi_readFrom(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop)
{
	if (len == 0) {
		return ESP_OK;
	}
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, ( address /*<< 1*/ ) | I2C_MASTER_READ, ACK_CHECK_EN);
	if (len > 1) {
		i2c_master_read(cmd, buf, len - 1, ACK_VAL);
	}
	i2c_master_read_byte(cmd, buf + len - 1, NACK_VAL);
	i2c_master_stop(cmd);
	esp_err_t ret = i2c_master_cmd_begin(I2C_PORT, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return ret;
}


#else

unsigned char twi_dcount = 18;
static unsigned char twi_sda, twi_scl;


static inline void SDA_LOW() {
  //Enable SDA (becomes output and since GPO is 0 for the pin, 
  // it will pull the line low)
  if (twi_sda < 32) {
    REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(twi_sda));
  }
  else {
    REG_WRITE(GPIO_ENABLE1_W1TS_REG, BIT(twi_sda - 32));
  }
}

static inline void SDA_HIGH() {
  //Disable SDA (becomes input and since it has pullup it will go high)
  if (twi_sda < 32) {
    REG_WRITE(GPIO_ENABLE_W1TC_REG, BIT(twi_sda));
  }
  else {
    REG_WRITE(GPIO_ENABLE1_W1TC_REG, BIT(twi_sda - 32));
  }
}

static inline uint32_t SDA_READ() { 
  if (twi_sda < 32) {
    return (REG_READ(GPIO_IN_REG) & BIT(twi_sda)) != 0;
  }
  else {
    return (REG_READ(GPIO_IN1_REG) & BIT(twi_sda - 32)) != 0;
  }
}

static void SCL_LOW() {
  if (twi_scl < 32) {
    REG_WRITE(GPIO_ENABLE_W1TS_REG, BIT(twi_scl));
  }
  else {
    REG_WRITE(GPIO_ENABLE1_W1TS_REG, BIT(twi_scl - 32));
  }
}

static void SCL_HIGH() {
  if (twi_scl < 32) {
    REG_WRITE(GPIO_ENABLE_W1TC_REG, BIT(twi_scl));
  }
  else {
    REG_WRITE(GPIO_ENABLE1_W1TC_REG, BIT(twi_scl - 32));
  }
}

static uint32_t SCL_READ() {
  if (twi_scl < 32) {
    return (REG_READ(GPIO_IN_REG) & BIT(twi_scl)) != 0;
  }
  else {
    return (REG_READ(GPIO_IN1_REG) & BIT(twi_scl - 32)) != 0;
  }
}


#ifndef FCPU80
#define FCPU80 80000000L
#endif

#if F_CPU == FCPU80
#define TWI_CLOCK_STRETCH 800
#else
#define TWI_CLOCK_STRETCH 1600
#endif

void twi_setClock(unsigned int freq){
#if F_CPU == FCPU80
  if(freq <= 100000) twi_dcount = 19;//about 100KHz
  else if(freq <= 200000) twi_dcount = 8;//about 200KHz
  else if(freq <= 300000) twi_dcount = 3;//about 300KHz
  else if(freq <= 400000) twi_dcount = 1;//about 400KHz
  else twi_dcount = 1;//about 400KHz
#else
  if(freq <= 100000) twi_dcount = 32;//about 100KHz
  else if(freq <= 200000) twi_dcount = 14;//about 200KHz
  else if(freq <= 300000) twi_dcount = 8;//about 300KHz
  else if(freq <= 400000) twi_dcount = 5;//about 400KHz
  else if(freq <= 500000) twi_dcount = 3;//about 500KHz
  else if(freq <= 600000) twi_dcount = 2;//about 600KHz
  else twi_dcount = 1;//about 700KHz
#endif
}

void twi_init(unsigned char sda, unsigned char scl){
  twi_sda = sda;
  twi_scl = scl;
  pinMode(twi_sda, OUTPUT);
  pinMode(twi_scl, OUTPUT);

  digitalWrite(twi_sda, 0);
  digitalWrite(twi_scl, 0);

  pinMode(twi_sda, INPUT_PULLUP);
  pinMode(twi_scl, INPUT_PULLUP);
  twi_setClock(100000);
}

void twi_stop(void){
  pinMode(twi_sda, INPUT);
  pinMode(twi_scl, INPUT);
}

static void twi_delay(unsigned char v){
  unsigned int i;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
  unsigned int reg;
  for(i=0;i<v;i++) reg = REG_READ(GPIO_IN_REG);
#pragma GCC diagnostic pop
}

static bool twi_write_start(void) {
  SCL_HIGH();
  SDA_HIGH();
  if (SDA_READ() == 0) return false;
  twi_delay(twi_dcount);
  SDA_LOW();
  twi_delay(twi_dcount);
  return true;
}

static bool twi_write_stop(void){
  unsigned int i = 0;
  SCL_LOW();
  SDA_LOW();
  twi_delay(twi_dcount);
  SCL_HIGH();
  while (SCL_READ() == 0 && (i++) < TWI_CLOCK_STRETCH);// Clock stretching (up to 100us)
  twi_delay(twi_dcount);
  SDA_HIGH();
  twi_delay(twi_dcount);

  return true;
}

bool do_log = false;
static bool twi_write_bit(bool bit) {
  unsigned int i = 0;
  SCL_LOW();
  if (bit) {SDA_HIGH(); if (do_log) {twi_delay(twi_dcount+1);}}
  else {SDA_LOW(); if (do_log) {} }
  twi_delay(twi_dcount+1);
  SCL_HIGH();
  while (SCL_READ() == 0 && (i++) < TWI_CLOCK_STRETCH);// Clock stretching (up to 100us)
  twi_delay(twi_dcount);
  return true;
}

static bool twi_read_bit(void) {
  unsigned int i = 0;
  SCL_LOW();
  SDA_HIGH();
  twi_delay(twi_dcount+2);
  SCL_HIGH();
  while (SCL_READ() == 0 && (i++) < TWI_CLOCK_STRETCH);// Clock stretching (up to 100us)
  bool bit = SDA_READ();
  twi_delay(twi_dcount);
  return bit;
}

static bool twi_write_byte(unsigned char byte) {
  
  if (byte == 0x43) {
    // printf("TWB %02x ", (uint32_t) byte);
    // do_log = true;
  }
  unsigned char bit;
  for (bit = 0; bit < 8; bit++) {
    twi_write_bit((byte & 0x80) != 0);
    byte <<= 1;
  }
  if (do_log) {
    printf("\n");
    do_log = false;
  }
  return !twi_read_bit();//NACK/ACK
}

static unsigned char twi_read_byte(bool nack) {
  unsigned char byte = 0;
  unsigned char bit;
  for (bit = 0; bit < 8; bit++) byte = (byte << 1) | twi_read_bit();
  twi_write_bit(nack);
  return byte;
}

unsigned char twi_writeTo(unsigned char address, unsigned char * buf, unsigned int len, unsigned char sendStop){
  unsigned int i;
  if(!twi_write_start()) return 4;//line busy
  if(!twi_write_byte(((address << 1) | 0) & 0xFF)) {
    if (sendStop) twi_write_stop();
    return 2; //received NACK on transmit of address
  }
  for(i=0; i<len; i++) {
    if(!twi_write_byte(buf[i])) {
      if (sendStop) twi_write_stop();
      return 3;//received NACK on transmit of data
    }
  }
  if(sendStop) twi_write_stop();
  i = 0;
  while(SDA_READ() == 0 && (i++) < 10){
    SCL_LOW();
    twi_delay(twi_dcount);
    SCL_HIGH();
    twi_delay(twi_dcount);
  }
  return 0;
}

unsigned char twi_readFrom(unsigned char address, unsigned char* buf, unsigned int len, unsigned char sendStop){
  unsigned int i;
  if(!twi_write_start()) return 4;//line busy
  if(!twi_write_byte(((address << 1) | 1) & 0xFF)) {
    if (sendStop) twi_write_stop();
    return 2;//received NACK on transmit of address
  }
  for(i=0; i<(len-1); i++) buf[i] = twi_read_byte(false);
  buf[len-1] = twi_read_byte(true);
  if(sendStop) twi_write_stop();
  i = 0;
  while(SDA_READ() == 0 && (i++) < 10){
    SCL_LOW();
    twi_delay(twi_dcount);
    SCL_HIGH();
    twi_delay(twi_dcount);
  }
  return 0;
}
#endif
