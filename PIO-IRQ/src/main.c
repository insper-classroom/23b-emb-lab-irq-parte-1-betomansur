/************************************************************************
* 5 semestre - Eng. da Computao - Insper
* Rafael Corsi - rafael.corsi@insper.edu.br
*
* Material:
*  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
*
* Objetivo:
*  - Demonstrar interrupção do PIO
*
* Periféricos:
*  - PIO
*  - PMC
*
* Log:
*  - 10/2018: Criação
************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define BUT_PIO PIOD
#define BUT_PIO_ID ID_PIOD
#define BUT_PIO_IDX 28
#define BUT_IDX_MASK (1 << BUT_PIO_IDX)

#define LED_PIO PIOA
#define LED_PIO_ID  ID_PIOA
#define LED_PIO_IDX 0
#define LED_IDX_MASK  (1 << LED_PIO_IDX)



/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);
void gfx_mono_draw_string(const char *str, const gfx_coord_t x, const gfx_coord_t y, const struct font *font);


/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

/*
* Exemplo de callback para o botao, sempre que acontecer
* ira piscar o led por 5 vezes
*
* !! Isso é um exemplo ruim, nao deve ser feito na pratica, !!
* !! pois nao se deve usar delays dentro de interrupcoes    !!
*/

volatile char but_flag;
volatile char pino_cima;
volatile char ja_clicado;
volatile char subiu;
volatile char desceu;
int aumento;
volatile int tempo;

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

	// Configura PIO para lidar com o pino do botão como entrada
	// com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);
	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
	BUT_PIO_ID,
	BUT_IDX_MASK,
	PIO_IT_EDGE,
	but_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	// Inicializa clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	char str[128]; // (1)

	// configura botao com interrupcao
	io_init();

	// super loop
	// aplicacoes embarcadas no devem sair do while(1).
	while(1)
	{
		

		sprintf(str, "%d", aumento);
		// gfx_mono_draw_string(str, 0, 0, &sysfont);
		
		if (!pino_cima)
		{
			tempo += 1;
			delay_ms(1);
		}
		if (but_flag) {  // (2)
			pisca_led(10, (500 + (aumento*100)));
			but_flag = 0;
			//delay_ms(10);
		}
	}
}
