#include <util/delay.h>
#include <avr/io.h>
#include <usart.h>
#include <display.h>
#include <button.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include <potentio.h>
#include <timer.h>
#include <dj.h>
#include <leds.h>

#define pulsePin PC5
uint32_t elapsedMs = 0;
uint16_t totalBeats = 0;
uint64_t totalMs = 0;
uint8_t average = 0;
uint8_t playing = 0;
SONG *song;
uint8_t minuteBeats = 0;
uint8_t noteIndex = 0;
uint8_t stop = 0;
uint8_t snooze = 0;
uint8_t alarm = 0;
// states
uint8_t awaitHigh = 1;
uint8_t awaitZero = 0;
uint16_t snoozeSeconds = 0;
ISR(TIMER2_COMPA_vect)
{

  counter++;

  elapsedMs += 4;

  if (counter % VEELVOUD == 0)
  {
    average = (int)(((double)totalBeats / (elapsedMs / 1000)) * 60);
    printf("%d.", (int)(((double)totalBeats / (elapsedMs / 1000)) * 60));

    if (elapsedMs / 1000 == 120)
    {
      totalBeats -= minuteBeats;
      elapsedMs -= 60000;
    }
    tick();
    if (snooze == 1)
    {
      snoozeSeconds++;
      if (snoozeSeconds == 10)
      {
        snoozeSeconds = 0;
        playingSong = 1;
        snooze = 0;
      }
    }
  }
  // verhoog counter met 1
  // als counter + 1 deelbaar is door VEELVOUD tel één seconde.
}
ISR(PCINT1_vect)
{
  if (whoPushed() != -1)
  {
    _delay_us(300);
    if (whoPushed() == 0)
    {
      printf("stop");
      playingSong = 0;
      stop = 1;
      printf(":");
    }
    if (whoPushed() == 2)
    {
      printf("snooze");
      playingSong = 0;
      snooze = 1;
    }
  }
}
ISR(USART_RX_vect) // De interrupt routine voor het ontvangen van data
{
  uint8_t byte = UDR0; // seriële data zit in UDR0 register
  char byteValue = (char)byte;
  if (byteValue == '1')
  {
    alarm = 1;
    lightUpAllLeds();
  }
}

void calcHeartrate()
{
  uint16_t rawADC = readADC(PC5);
  if (awaitHigh == 1)
  {
    if (rawADC == 1023)
    {
      awaitHigh = 0;
    }
  }
  else if (rawADC == 0)
  {
    totalBeats += 1;
    minuteBeats += 1;
    awaitHigh = 1;
  }
}

int main()
{
  initUSART();
  initTimer();
  startTimer();
  initADC();
  enableAllLeds();
  lightDownAllLeds();
  initDisplay();
  sei();
  UCSR0B |= (1 << RXCIE0);

  NOTE *notes[] = {
      createNote(E4, 100),
      createNote(G4, 100),
      createNote(A4, 200),
      createNote(F4, 100),
      createNote(D4, 100),
      createNote(G4, 200),
      createNote(E4, 100),
      createNote(G4, 100),
      createNote(A4, 200),
      createNote(G4, 100),
      createNote(C5, 100),
      createNote(G4, 200),
      createNote(E4, 100),
      createNote(G4, 100),
      createNote(A4, 200),
      createNote(F4, 100),
      createNote(D4, 100),
      createNote(G4, 200),
      createNote(E4, 100),
      createNote(G4, 100),
      createNote(A4, 200),
      createNote(G4, 100),
      createNote(C5, 100),
      createNote(G4, 200),
      createNote(E4, 100),
      createNote(G4, 100),
      createNote(A4, 200),
      createNote(F4, 100),
      createNote(D4, 100),
      createNote(G4, 200)};

  song = createSong("Despacito", notes, 28);
  enableButtons();
  enableAllButtonInterrupts();
  while (1)
  {

    calcHeartrate();

    for (int i = 0; i < 1000; i++)
    {
      writeNumber(average);
    }
    if (stop == 1)
    {
      break;
    }
    if (alarm == 1)
    {
      if (elapsedMs / 1000 > 15)
      {
        if (average > 80 && snooze == 0)
        {
          enableDj();
          playSong(song);
          if (snooze == 0)
          {
            printf(":");
          }
        }
      }
      lightUpAllLeds();
    }
  }

  return 0;
}
