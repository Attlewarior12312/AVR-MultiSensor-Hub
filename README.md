# AVR Multi-Functional System: Thermometer, Stopwatch & Solar Tracker

## Opis Projektu
Projekt napisany w języku C na mikrokontrolery z rodziny AVR (taktowanie 16 MHz, kompatybilne m.in. z ATmega328P / Arduino Uno). Program integruje w sobie trzy główne funkcjonalności: pomiar temperatury otoczenia za pomocą termistora NTC, precyzyjny stoper z fizycznym przyciskiem sterującym oraz system śledzenia źródła światła (Solar Tracker) wykorzystujący fotorezystory i serwomechanizm.

Dane są prezentowane w czasie rzeczywistym na wyświetlaczu znakowym LCD (HD44780) oraz przesyłane do komputera za pośrednictwem sprzętowego interfejsu UART.

## Funkcjonalności
* **Cyfrowy Termometr:** Konwersja wartości ADC z termistora NTC na stopnie Celsjusza. Przeliczanie rezystancji na temperaturę z wykorzystaniem parametru Beta oraz zaimplementowanym (opcjonalnym) algorytmem Steinharta-Harta.
* **Solar Tracker:** Porównywanie natężenia światła z dwóch fotorezystorów (LDR) i automatyczna regulacja kąta serwomechanizmu poprzez sygnał sprzętowy Fast PWM (Timer1) w celu "podążania" za światłem.
* **Stoper z obsługą przycisku:** Oparty na przerwaniach sprzętowych Timer0 zliczający czas (ms, s, min, h). Zawiera zintegrowany mechanizm programowego eliminowania drgań styków (*debouncing*) oraz inteligentnego rozróżniania kliknięć (Start/Pauza) i przytrzymania (Reset timer'a po 1000ms).
* **Wyświetlacz LCD:** Lekka biblioteka do obsługi wyświetlaczy zgodnych ze sterownikiem HD44780 w trybie 4-bitowym z możliwością precyzyjnego pozycjonowania tekstu i wyświetlania zmiennych (funkcje z rodziny `printf`).
* **Komunikacja UART:** Asynchroniczne wysyłanie sformatowanych danych diagnostycznych (odczyty przetwornika ADC) z prędkością 9600 baud.

## Wymagania Sprzętowe i Pinout
Poniżej znajduje się domyślne mapowanie wyprowadzeń mikrokontrolera zdefiniowane w kodzie (należy dostosować do konkretnego schematu płytki):

**Wyświetlacz LCD (HD44780):**
* `RS` -> PD7
* `EN` -> PB0
* `D4` -> PB2
* `D5` -> PB3
* `D6` -> PB4
* `D7` -> PB5
* `Podświetlenie (opcja)` -> PC6

**Peryferia Analogowe / Wejścia (ADC):**
* `Termistor NTC` -> PC5 (ADC5)
* `Fotorezystor LDR - Lewy` -> PC3 (ADC3)
* `Fotorezystor LDR - Prawy` -> PC4 (ADC4)
> **Uwaga:** Układ wykorzystuje wewnętrzne źródło odniesienia napięcia ADC ustawione na 1.1V.

**Elementy Wykonawcze i Sterujące:**
* `Serwomechanizm (PWM)` -> PB1 (OC1A)
* `Przycisk Stopera` -> PC0 (domyślnie aktywny stan niski - zwarcie do masy). W kodzie włączony jest wewnętrzny rezystor Pull-Up.
* `UART TX` -> Sprzętowy pin TX mikrokontrolera (zazwyczaj PD1).

## Struktura Kodu
* `main.c` - Główna pętla programu, konfiguracja rejestrów (Timer0, Timer1, Timer2), obsługa przerwań (`ISR`), logika działania stopera, sterowanie serwem i przeliczanie temperatury.
* `lcd_displ.h` / `lcd_displ.c` - Biblioteka napisana w C służąca do bezproblemowej komunikacji z wyświetlaczem LCD za pomocą zaledwie 6 pinów.

## Środowisko Programistyczne i Kompilacja
* **Kompilator:** `avr-gcc`
* **Zależności:** Standardowe biblioteki środowiska AVR (`<avr/io.h>`, `<util/delay.h>`, `<avr/interrupt.h>`, `<math.h>`)
* **Taktowanie (F_CPU):** Kod jest zoptymalizowany dla zegara `16000000UL` (16 MHz). W przypadku zmiany kwarcu, należy zaktualizować definicję w pliku `main.c` i ew. dostosować preskalery timerów.
<img width="1024" height="600" alt="obraz" src="https://github.com/user-attachments/assets/2f11c65a-ac33-4655-bc43-2ca2f25e6653" />
