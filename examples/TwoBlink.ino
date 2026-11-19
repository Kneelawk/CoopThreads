#include <CoopThreads.h>

CoopThreads<8192> threads(8);

void myThread(size_t i) {
  while (true) {
    threads.delay(300);
    digitalWrite(12, HIGH);
    threads.delay(300);
    digitalWrite(12, LOW);
  }
}

void setup() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // schedule the thread
  threads.spawn<size_t>(myThread, 10);
}

void loop() {
  threads.delay(500);
  digitalWrite(13, HIGH);
  threads.delay(500);
  digitalWrite(13, LOW);
}
