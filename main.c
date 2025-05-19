#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include <stdio.h>

#define LED_VERDE     11
#define LED_VERMELHO  13
#define BUZZER        14
#define BOTAO         15

volatile bool botao_pressionado = false;
volatile bool em_travessia = false;
typedef enum { VERMELHO, VERDE, AMARELO } estado_t;
estado_t estado_atual = VERMELHO;

void botao_callback(uint gpio, uint32_t events) {
    if (gpio == BOTAO) {
        botao_pressionado = true;
        printf("Botão de Pedestres acionado\n");
    }
}

void config_gpio() {
    gpio_init(LED_VERDE);
    gpio_init(LED_VERMELHO);
    gpio_init(BUZZER);
    gpio_init(BOTAO);

    gpio_set_dir(LED_VERDE, GPIO_OUT);
    gpio_set_dir(LED_VERMELHO, GPIO_OUT);
    gpio_set_dir(BUZZER, GPIO_OUT);
    gpio_set_dir(BOTAO, GPIO_IN);
    gpio_pull_up(BOTAO);

    gpio_set_irq_enabled_with_callback(BOTAO, GPIO_IRQ_EDGE_FALL, true, &botao_callback);
}

void desliga_leds() {
    gpio_put(LED_VERDE, 0);
    gpio_put(LED_VERMELHO, 0);
}

int64_t alarme_callback(alarm_id_t id, void *user_data) {
    desliga_leds();
    switch (estado_atual) {
        case VERMELHO:
            gpio_put(LED_VERMELHO, 1);
            printf("Sinal: VERMELHO\n");
            if (em_travessia) {
                for (int i = 5; i > 0; i--) {
                    printf("Travessia: %d\n", i);
                    gpio_put(BUZZER, 1);
                    sleep_ms(100);
                    gpio_put(BUZZER, 0);
                    sleep_ms(900);
                }
                em_travessia = false;
            }
            estado_atual = VERDE;
            return 10 * 1000 * 1000;
        case VERDE:
            gpio_put(LED_VERDE, 1);
            printf("Sinal: VERDE\n");
            estado_atual = AMARELO;
            return 10 * 1000 * 1000;
        case AMARELO:
            gpio_put(LED_VERDE, 1);
            gpio_put(LED_VERMELHO, 1);
            printf("Sinal: AMARELO\n");
            estado_atual = VERMELHO;
            return 3 * 1000 * 1000;
    }
    return 0;
}

int main() {
    stdio_init_all();
    config_gpio();
    add_alarm_in_ms(0, alarme_callback, NULL, true);
    while (true) {
        if (botao_pressionado && estado_atual != VERMELHO) {
            botao_pressionado = false;
            estado_atual = AMARELO;
            em_travessia = true;
        }
        tight_loop_contents();
    }
}
