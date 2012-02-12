#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

static volatile unsigned long milliCount = 0;
static volatile byte interrupted = 0;
void initTime();

void  setup(void) {
    milliCount = 0;
    wdt_disable();

    initTime();
    Serial.begin(115200);
}

void loop() {
    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sei();
    sleep_enable();
    while (!interrupted) {
      sleep_cpu();
    }
    sleep_disable();
    interrupted = 0;
    Serial.println(milliCount);
    delay(10);
}

void initTime() {
	cli();

	// stop timer2 for now
	TCCR2B  = 0;

	// Enable asynchronous mode
	TIMSK2 = 0;
	ASSR |= _BV(AS2);

	TCNT2 = 0;
	OCR2A = 7;

	// set CTC mode, clear PWM modes
	TCCR2A = _BV(WGM21) | _BV(WGM20);

	// x1024 prescaler, start clock
	TCCR2B = _BV(CS20) | _BV(CS21) | _BV(CS22) | _BV(WGM22);

        // Wait for registers to be set (2 clock2 intervals, ~488 main clocks)
        while (ASSR & (_BV(TCN2UB) | _BV(OCR2AUB) | _BV(TCR2AUB) | _BV(TCR2BUB))) ;

	// enable interrupt on overflow
	TIFR2 |= _BV(TOV2);
	TIMSK2 |= _BV(TOIE2);

	// set up variables
	milliCount = 0;
        
        // re-enable interrupts
	sei();
}


ISR(TIMER2_OVF_vect) {
	milliCount += 250;
        interrupted = 1;
}
