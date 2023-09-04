#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define BUT_PIO PIOD
#define BUT_PIO_ID ID_PIOD
#define BUT_PIO_IDX 28
#define BUT_IDX_MASK (1 << BUT_PIO_IDX)

#define BUT3_PIO PIOA
#define BUT3_PIO_ID ID_PIOA
#define BUT3_PIO_IDX 19
#define BUT3_IDX_MASK (1u << BUT3_PIO_IDX)

#define BUT2_PIO PIOC
#define BUT2_PIO_ID ID_PIOC
#define BUT2_PIO_IDX 31
#define BUT2_IDX_MASK (1u << BUT2_PIO_IDX)

#define LED_PIO PIOA
#define LED_PIO_ID  ID_PIOA
#define LED_PIO_IDX 0
#define LED_IDX_MASK  (1 << LED_PIO_IDX)

void io_init(void);
void pisca_led(int n, int t);

volatile char but_flag;
volatile char but_flag2;
volatile char but_flag3;
volatile char pino_cima;
volatile char ja_clicado;
volatile char subiu;
volatile char desceu;
int aumento = 0;
int tempo = 0;

void but_callback3(void){
	but_flag3 = 1;
}

void but_callback2(void){
	but_flag2 = 1;
	aumento -= 2 ;
}

void but_callback(void)

{
	if (!but_flag)
	{
		if (pio_get(BUT_PIO, PIO_INPUT, BUT_IDX_MASK)) {
			// PINO == 1 --> Borda de subida
			pino_cima = 1;
			if (tempo < 2000){
				aumento ++;
			}
			else{
				aumento --;
			}
			but_flag = 1;
		}
		else {
			//PINO == 0 --> Borda de descida
			tempo = 0;
			pino_cima = 0;
			
		}
	}

}

/************************************************************************/
/* funções                                                              */
/************************************************************************/

// pisca led N vez no periodo T
void pisca_led(int n, int t){
	for (int i=0;i<n;i++){
		pio_clear(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
		pio_set(LED_PIO, LED_IDX_MASK);
		delay_ms(t);
	}
}



// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{

	// Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	

	// Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);

	pmc_enable_periph_clk(BUT3_PIO_ID);


	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);
	
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_IDX_MASK, 60);
	
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_IDX_MASK, 60);
	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);
	
	pio_handler_set(BUT3_PIO,
	BUT3_PIO_ID,
	BUT3_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but_callback2);
	
	pio_handler_set(BUT2_PIO,
	BUT2_PIO_ID,
	BUT2_IDX_MASK,
	PIO_IT_RISE_EDGE,
	but_callback3);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	
	
	pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 3); // Prioridade 4
	
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 3); // Prioridade 4
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	gfx_mono_ssd1306_init();

	// Inicializa clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	char str[128]; // (1)
	char str2[128];
	char str3[128];

	// configura botao com interrupcao
	io_init();

	// super loop
	// aplicacoes embarcadas no devem sair do while(1).
	while(1)
	{

		if (!pino_cima)
		{
			tempo += 1;
			delay_ms(1);
		}
		if (but_flag) {  // (2)
			int cont = aumento;
			cont = cont *100;
			cont = cont + 500;
			sprintf(str, "Freq: %d", cont);
			gfx_mono_draw_string(str, 0, 0, &sysfont);
			pisca_led(10, (cont));
			but_flag = 0;
		}
		if (but_flag2)
		{
			sprintf(str2, "Diminui");
			gfx_mono_draw_string(str2, 0, 0, &sysfont);
			delay_ms(10);
			but_flag2 = 0;
			
		}
		if (but_flag3)
		{
			sprintf(str3, "Parando");
			gfx_mono_draw_string(str3, 0, 0, &sysfont);
			delay_ms(5000);
			but_flag3 = 0;
		}
	}
}






