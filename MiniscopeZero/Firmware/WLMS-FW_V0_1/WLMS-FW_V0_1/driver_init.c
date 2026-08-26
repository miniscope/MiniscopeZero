/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_init.h"
#include <peripheral_clk_config.h>
#include <utils.h>
#include <hal_init.h>

#include <hpl_adc_base.h>
#include <hpl_adc_base.h>

/*! The buffer size for USART */
#define USART_0_BUFFER_SIZE 16

struct usart_async_descriptor USART_0;
struct spi_m_sync_descriptor  SPI_0;
struct timer_descriptor       TIMER_0;

static uint8_t USART_0_buffer[USART_0_BUFFER_SIZE];

struct adc_sync_descriptor ADC_0;

struct adc_sync_descriptor ADC_1;

struct camera_async_descriptor CAMERA_0;

struct pwm_descriptor PWM_0;

void ADC_0_PORT_init(void)
{

	// Disable digital pin circuitry
	gpio_set_pin_direction(BATT_VOLT, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(BATT_VOLT, PINMUX_PA02B_ADC0_AIN0);
}

void ADC_0_CLOCK_init(void)
{
	hri_mclk_set_APBDMASK_ADC0_bit(MCLK);
	hri_gclk_write_PCHCTRL_reg(GCLK, ADC0_GCLK_ID, CONF_GCLK_ADC0_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
}

void ADC_0_init(void)
{
	ADC_0_CLOCK_init();
	ADC_0_PORT_init();
	adc_sync_init(&ADC_0, ADC0, (void *)NULL);
}

void ADC_1_PORT_init(void)
{

	// Disable digital pin circuitry
	gpio_set_pin_direction(WPT_VOLT, GPIO_DIRECTION_OFF);

	gpio_set_pin_function(WPT_VOLT, PINMUX_PB08B_ADC1_AIN0);
}

void ADC_1_CLOCK_init(void)
{
	hri_mclk_set_APBDMASK_ADC1_bit(MCLK);
	hri_gclk_write_PCHCTRL_reg(GCLK, ADC1_GCLK_ID, CONF_GCLK_ADC1_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
}

void ADC_1_init(void)
{
	ADC_1_CLOCK_init();
	ADC_1_PORT_init();
	adc_sync_init(&ADC_1, ADC1, (void *)NULL);
}

void EXTERNAL_IRQ_0_init(void)
{
	hri_gclk_write_PCHCTRL_reg(GCLK, EIC_GCLK_ID, CONF_GCLK_EIC_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
	hri_mclk_set_APBAMASK_EIC_bit(MCLK);

	// Set pin direction to input
	gpio_set_pin_direction(nCHRG, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(nCHRG,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(nCHRG, PINMUX_PB23A_EIC_EXTINT7);

	// Set pin direction to input
	gpio_set_pin_direction(PUSH_BUT_MCU, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(PUSH_BUT_MCU,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(PUSH_BUT_MCU, PINMUX_PA25A_EIC_EXTINT9);

	// Set pin direction to input
	gpio_set_pin_direction(FrameValid, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(FrameValid,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(FrameValid, PINMUX_PB14A_EIC_EXTINT14);

	ext_irq_init();
}

void CAMERA_0_PORT_init(void)
{

	gpio_set_pin_function(PCC_CLK, PINMUX_PA14K_PCC_CLK);

	gpio_set_pin_function(PCC_D0, PINMUX_PA16K_PCC_DATA0);

	gpio_set_pin_function(PCC_D1, PINMUX_PA17K_PCC_DATA1);

	gpio_set_pin_function(PCC_D2, PINMUX_PA18K_PCC_DATA2);

	gpio_set_pin_function(PCC_D3, PINMUX_PA19K_PCC_DATA3);

	gpio_set_pin_function(PCC_D4, PINMUX_PA20K_PCC_DATA4);

	gpio_set_pin_function(PCC_D5, PINMUX_PA21K_PCC_DATA5);

	gpio_set_pin_function(PCC_D6, PINMUX_PA22K_PCC_DATA6);

	gpio_set_pin_function(PCC_D7, PINMUX_PA23K_PCC_DATA7);

	gpio_set_pin_function(PCC_FV, PINMUX_PA12K_PCC_DEN1);

	gpio_set_pin_function(PCC_HV, PINMUX_PA13K_PCC_DEN2);
}

void CAMERA_0_CLOCK_init(void)
{
	hri_mclk_set_APBDMASK_PCC_bit(MCLK);
}

void CAMERA_0_init(void)
{
	CAMERA_0_CLOCK_init();
	camera_async_init(&CAMERA_0, PCC);
	CAMERA_0_PORT_init();
}

/**
 * \brief USART Clock initialization function
 *
 * Enables register interface and peripheral clock
 */
void USART_0_CLOCK_init()
{

	hri_gclk_write_PCHCTRL_reg(GCLK, SERCOM1_GCLK_ID_CORE, CONF_GCLK_SERCOM1_CORE_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
	hri_gclk_write_PCHCTRL_reg(GCLK, SERCOM1_GCLK_ID_SLOW, CONF_GCLK_SERCOM1_SLOW_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));

	hri_mclk_set_APBAMASK_SERCOM1_bit(MCLK);
}

/**
 * \brief USART pinmux initialization function
 *
 * Set each required pin to USART functionality
 */
void USART_0_PORT_init()
{

	gpio_set_pin_function(PA00, PINMUX_PA00D_SERCOM1_PAD0);

	gpio_set_pin_function(IR_REC, PINMUX_PB22C_SERCOM1_PAD2);
}

/**
 * \brief USART initialization function
 *
 * Enables USART peripheral, clocks and initializes USART driver
 */
void USART_0_init(void)
{
	USART_0_CLOCK_init();
	usart_async_init(&USART_0, SERCOM1, USART_0_buffer, USART_0_BUFFER_SIZE, (void *)NULL);
	USART_0_PORT_init();
}

void SPI_0_PORT_init(void)
{

	gpio_set_pin_level(TX,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(TX, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(TX, PINMUX_PB16C_SERCOM5_PAD0);

	gpio_set_pin_level(RX,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(RX, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(RX, PINMUX_PB17C_SERCOM5_PAD1);

	// Set pin direction to input
	gpio_set_pin_direction(I2C_BB_SDA, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(I2C_BB_SDA,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(I2C_BB_SDA, PINMUX_PB00D_SERCOM5_PAD2);
}

void SPI_0_CLOCK_init(void)
{
	hri_gclk_write_PCHCTRL_reg(GCLK, SERCOM5_GCLK_ID_CORE, CONF_GCLK_SERCOM5_CORE_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
	hri_gclk_write_PCHCTRL_reg(GCLK, SERCOM5_GCLK_ID_SLOW, CONF_GCLK_SERCOM5_SLOW_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));

	hri_mclk_set_APBDMASK_SERCOM5_bit(MCLK);
}

void SPI_0_init(void)
{
	SPI_0_CLOCK_init();
	spi_m_sync_init(&SPI_0, SERCOM5);
	SPI_0_PORT_init();
}

void delay_driver_init(void)
{
	delay_init(SysTick);
}

void PWM_0_PORT_init(void)
{

	gpio_set_pin_function(LED_PWM, PINMUX_PB30E_TC0_WO0);
}

void PWM_0_CLOCK_init(void)
{

	hri_mclk_set_APBAMASK_TC0_bit(MCLK);
	hri_gclk_write_PCHCTRL_reg(GCLK, TC0_GCLK_ID, CONF_GCLK_TC0_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));
}

void PWM_0_init(void)
{
	PWM_0_CLOCK_init();
	PWM_0_PORT_init();
	pwm_init(&PWM_0, TC0, _tc_get_pwm());
}

/**
 * \brief Timer initialization function
 *
 * Enables Timer peripheral, clocks and initializes Timer driver
 */
static void TIMER_0_init(void)
{
	hri_mclk_set_APBAMASK_TC1_bit(MCLK);
	hri_gclk_write_PCHCTRL_reg(GCLK, TC1_GCLK_ID, CONF_GCLK_TC1_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos));

	timer_init(&TIMER_0, TC1, _tc_get_timer());
}

void system_init(void)
{
	init_mcu();

	// GPIO on PA04

	gpio_set_pin_level(SPI_BB_MOSI,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_BB_MOSI, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SPI_BB_MOSI, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA05

	gpio_set_pin_level(SPI_BB_SCK,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_BB_SCK, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SPI_BB_SCK, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA06

	// Set pin direction to input
	gpio_set_pin_direction(SPI_BB_MISO, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(SPI_BB_MISO,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(SPI_BB_MISO, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA07

	// Set pin direction to input
	gpio_set_pin_direction(MONITOR0, GPIO_DIRECTION_IN);

	gpio_set_pin_pull_mode(MONITOR0,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(MONITOR0, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA15

	gpio_set_pin_level(TRIGGER0,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(TRIGGER0, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(TRIGGER0, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PA24

	gpio_set_pin_level(EN_3V3,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(EN_3V3, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(EN_3V3, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB01

	gpio_set_pin_level(ENT_LED,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(ENT_LED, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(ENT_LED, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB02

	gpio_set_pin_level(I2C_BB_SCL,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(I2C_BB_SCL, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(I2C_BB_SCL, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB09

	gpio_set_pin_level(LED_STATUS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(LED_STATUS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(LED_STATUS, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB12

	gpio_set_pin_level(SPI_BB_NSS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   true);

	// Set pin direction to output
	gpio_set_pin_direction(SPI_BB_NSS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(SPI_BB_NSS, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB13

	gpio_set_pin_level(RESET_CMOS,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	// Set pin direction to output
	gpio_set_pin_direction(RESET_CMOS, GPIO_DIRECTION_OUT);

	gpio_set_pin_function(RESET_CMOS, GPIO_PIN_FUNCTION_OFF);

	// GPIO on PB15

	gpio_set_pin_direction(GCLK1_OUT,
	                       // <y> Pin direction
	                       // <id> pad_direction
	                       // <GPIO_DIRECTION_OFF"> Off
	                       // <GPIO_DIRECTION_IN"> In
	                       // <GPIO_DIRECTION_OUT"> Out
	                       GPIO_DIRECTION_OUT);

	gpio_set_pin_level(GCLK1_OUT,
	                   // <y> Initial level
	                   // <id> pad_initial_level
	                   // <false"> Low
	                   // <true"> High
	                   false);

	gpio_set_pin_pull_mode(GCLK1_OUT,
	                       // <y> Pull configuration
	                       // <id> pad_pull_config
	                       // <GPIO_PULL_OFF"> Off
	                       // <GPIO_PULL_UP"> Pull-up
	                       // <GPIO_PULL_DOWN"> Pull-down
	                       GPIO_PULL_OFF);

	gpio_set_pin_function(GCLK1_OUT,
	                      // <y> Pin function
	                      // <id> pad_function
	                      // <i> Auto : use driver pinmux if signal is imported by driver, else turn off function
	                      // <GPIO_PIN_FUNCTION_OFF"> Auto
	                      // <GPIO_PIN_FUNCTION_OFF"> Off
	                      // <GPIO_PIN_FUNCTION_A"> A
	                      // <GPIO_PIN_FUNCTION_B"> B
	                      // <GPIO_PIN_FUNCTION_C"> C
	                      // <GPIO_PIN_FUNCTION_D"> D
	                      // <GPIO_PIN_FUNCTION_E"> E
	                      // <GPIO_PIN_FUNCTION_F"> F
	                      // <GPIO_PIN_FUNCTION_G"> G
	                      // <GPIO_PIN_FUNCTION_H"> H
	                      // <GPIO_PIN_FUNCTION_I"> I
	                      // <GPIO_PIN_FUNCTION_J"> J
	                      // <GPIO_PIN_FUNCTION_K"> K
	                      // <GPIO_PIN_FUNCTION_L"> L
	                      // <GPIO_PIN_FUNCTION_M"> M
	                      // <GPIO_PIN_FUNCTION_N"> N
	                      GPIO_PIN_FUNCTION_M);

	ADC_0_init();

	ADC_1_init();

	EXTERNAL_IRQ_0_init();

	CAMERA_0_init();

	USART_0_init();

	SPI_0_init();

	delay_driver_init();

	PWM_0_init();

	TIMER_0_init();
}
