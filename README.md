# Information zum Projekt

> Zuständige Professor: Prof. Dr. Mächtel

> Hochschule: HTWG

> Scheinart: Projekt

> Teampartner: Dennis Griesser & Torben Woltjen

> Semester: SS2015

> Fortführung des Projektes: Ja

> Status: Pausiert seit Juli 2015

# Realzeitsysteme Projekt - Potzroboter - Dokumentation

![IMG_20150623_203904](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/IMG_20150623_203904.jpg)

## Projektbeschreibung

Während unseres Projektes soll ein realzeitfähiger "Putzroboter" realisiert werden, welcher dazu in der Lage sein soll, sich in einem Raum autonom fortzubewegen. Dabei wird davon ausgegangen, dass der Fußboden eine ebene Fläche darstellt: Es sollten keine kleineren Absätze oder ähnliche Hindernisse vorhanden sein. Größere Hindernisse wie Möbel, Tischbeine usw. sind allerdings erlaubt. Letztendlich soll der Roboter dazu in der Lage sein, sich mehr oder weniger zufällig im Raum zu bewegen, ohne dabei Möbel oder andere Gegenstände zu berühren.

Die Abstandmessung zu den vorhandenen Hindernissen erfolgt durch drei Ultraschallsensoren, welche an der Vorderseite des Roboters angebracht werden. Die Sensoren werden dazu periodisch abgefragt und die Rückgabewerte werden anschließen verarbeitet und interpretiert, das heißt in eine absolute Entfernung zu den vor dem Roboter vorhandenen Gegenständen umgerechnet. Diese Informationen werden dazu verwendet, die Motoren des Roboters anzusteuern und eine autonome und kollisionsfreie Fahrt des Roboters im Raum zu ermöglichen.

Im Gegensatz zu den kommerziell erwerbbaren Putzrobotern werden wir mehr Wert auf die Navigation als auf die Reinigungsqualität legen: Der Roboter soll einfach ein Swiffer-Staubtuch unter sich mitziehen.

## Anforderung

Die drei Ulraschallsensoren sollen in einem noch zu definierenden Zeitintervall angesteuert werden. Derzeit wissen wir aber noch nicht, wie diese sich verhalten, also ob die Sensoren gleichzeitig verwendet werden können, oder man sie nacheinander abfragen muss, um eine gegenseitige Beeinflussung zu verhindern. Die Ultraschallsensoren können den Abstand zwischen sich selber und einem Gegenstand ermitteln, indem sie einen kurzen Ultraschallimpuls aussenden und das Echo wieder empfangen. Anhand der dazwischenliegenden Zeit und der Schallgeschwindigkeit, lässt sich anschließen der Abstand zwischen Ultraschallsensor und Gegenstand berechnen.

Nach Vorliegen der Sensordaten, soll der Roboter entscheiden, was diese für die Ansteuerung der Motoren bedeuten. Wenn der Roboter auf ein Hindernis zusteuert (oder sich ein Hindernis dem Roboter nähert), soll er sich um 25° drehen und erneut eine Abstandsmessung durchführen. Dieser Vorgang wird so lange wiederholt, bis der Roboter genügend Abstand zu allen Gegenständen vor ihm hat und wieder geradeaus weiterfahren kann.

Die Steuerung des Roboters erfolgt mehr oder weniger zufällig nach dem oben beschriebenen Prozess. Die Anforderung liegt darin, dass der Roboter nicht mit einem Objekt kollidiert.

Eine Schwierigkeit wird darin liegen, die drei Sensoren so auszuwerten, dass alle Realzeitbedingungen erfüllt werden und der Roboter nebenher noch weiterfahren kann. Da der von uns verwendete Raspberry Pi eine Singlecore-CPU verwendet, können wir diese zwei Aufgaben (Auswerten der Sensoren sowie Fahren) nicht auf zwei unterschiedliche CPUs verteilen.

## Realzeitbedingung

Insgesamt muss der Roboter so entworfen werden, dass die Auslastung kleiner oder gleich der Anzahl der Rechnerkerne ist. Dies bedeutet, dass die einzelnen Threads so programmiert und synchronisiert werden müssen, dass die Rechnerauslastung in Bezug auf das Einkernsystem unter 100% bleibt.

Die Werte der Sensoren werden alle 100 ms gemessen, dadurch ergibt sich für das Abfragen der drei Sensoren eine maximale Reaktionszeit von 300ms. Anschließend kann der Motor angesteuert werden, welcher ebenfalls eine maximale zulässige Reaktionszeit von 300ms aufweist. Dies bedeutet die Reaktion muss dann jeweils innerhalb der Zeitspanne von 300ms erfolgen.

Des Weiteren müssen alle Deadlines in der vorgegebene Zeit eingehalten werden. Die Blockierzeiten sind zu berücksichtigen, da mehrere Threads verwendet werden und auf einen gemeinsamen Bereich zugreifen, der deshalb geschützt werden muss.

## Realzeitnachweis ohne Last

*Hinweis 1: Bessere Darstellung der Formel im Buch "Moderne Realzeitsysteme kompakt" Im folgenden werden wir einen Realzeitnachweis auf Anwendungsebene durchführen, dabei gehen wir von einer idealen Hardware und einem idealen Betriebssysteme aus. Für weitere Informationen zu Schwierigkeiten, siehe Punkt "Probleme und Lösungen"
Zeiten*

| Task | tP,min in ms | tD,min in ms | tD,max in ms | tE,min in ms | tE,max in ms | Blockierzeit in ms |
| ---- | ------------ | ------------ | ------------ | ------------ | ------------ | ------------------ |
| Ultraschallsensor 1 | 250 | 0 | 250 | 0,5 | 18 | 3 |
| Ultraschallsensor 2 | 250 | 0 | 250 | 0,5 | 19 | 3 |
| Ultraschallsensor 3 | 250 | 0 | 250 | 0,5 | 18 | 3 |
| Motor (Mainthread) | 40 | 0 | 40 | 0,6 | 2,1 | 0,9 |

*Hinweis: Die Blockierzeit wird gemessen, indem für jede IO-Funktion die Blockierzeit berechnet wird.*

#### Berechnung der Blockierzeit:

> Ultraschallsensoren tE,max = Executiontime mit Blockierzeit - Blockierzeit

> Motor tE,max = Executiontime mit Blockierzeit - Blockierzeit

### Scheduling

Verwendeter Scheduler: prioritätsgesteuertes Scheduling

| Task | Priorität |
| ---- | --------- |
| Ultraschallsensor 1 | 99 |
| Ultraschallsensor 2 | 99 |
| Ultraschallsensor 3 | 99 |
| Motor | 99 |

### Unterbrechbarkeit und Abhängigkeit

| Task | Unterbrechbarkeit | Abhängigkeit |
| ---- | ----------------- | ------------ |
| Ultraschallsensor 1 | - | - |
| Ultraschallsensor 2 | - | - |
| Ultraschallsensor 3 | - | - |
| Motor (Mainthread) | - | Ultraschallsensor 1 - 3 |

### Ressourcenbenutzung

| Task | Ressource | Dauer |
| ---- | --------- | ----- |
| Ultraschallsensor 1 | Linker Ultraschallsensor | 207ms |
| Ultraschallsensor 2 | Front Ultraschallsensor | 207ms |
| Ultraschallsensor 3 | Rechter Ultraschallsensor | 207ms |
| Motor (Mainthread) | Motor | 40ms

### Grundlage

Der Putzroboter ist ein Realzeitsystem, da er beide Realzeitbedingungen erfüllt:

#### Erste Bedingung

| Task | Pges = Summe(tE,max / tP,min) | größer/gleich |  c  | Bedingung: Pges <= c |
| ---- | ----------------------------- | ------------- | --- | -------------------- |
| Ultraschallsensor 1 | 0,084 | <= | - | - |
| Ultraschallsensor 2 | 0,088 | <= | - | - |
| Ultraschallsensor 3 | 0,084 | <= | - | - |
| Motor (Mainthread) | 0,075 | <= | - | - |
| Gesamt | 0,331 | <= | 1 | Erfüllt |

#### Zweite Bedingung

| Task | tD,min | größer/gleich | tR,min | größer/gleich | tR,max | größer/gleich | tD,max 	Bedingung |
| ---- | ------ | ------------- | ------ | ------------- | ------ | ------------- | ----------------- |
| Ultraschallsensor 1 | 0 | <= | 20 | <= | 207 | <= | 250 | Erfüllt |
| Ultraschallsensor 2 | 0 | <= | 20 | <= | 207 | <= | 250 | Erfüllt |
| Ultraschallsensor 3 | 0 | <= | 20 | <= | 207 | <= | 250 | Erfüllt |
| Motor (Mainthread) | 0 | <= | 3,2 | <= | 31 | <= | 40 | Erfüllt |

*Nachweis ohne Berücksichtigung der Ressourcen*

### Hinreichender Schedulingtest

#### Formel 1:

> u = Summe(tE,max / ( min( tD,max ; tP,min ))) <= n(21/n - 1); Summe -> Oben n und Unten j=1

| Task | Summe(tE,max / ( min( tD,max ; tP,min ))) | größer/gleich | n(21/n - 1) |
| ---- | ----------------------------------------- | ------------- | ----------- |
| Ultraschallsensor 1 | 8,4% | <= | - |
| Ultraschallsensor 2 | 8,8% | <= | - |
| Ultraschallsensor 3 | 8,4% | <= | - |
| Motor (Mainthread) | 7,5% | <= | - |
| Gesamt | 33,1% | <= | 75,6% |

*Auslastungsgrenze bei 4 Threads liefert eine Aussage -> Kein notwendiger Schedulertest nötig. *
*Nachweis unter Berücksichtigung der Ressourcen*

### Blockierzeit

#### Datenfluss / Ressourcenverwendung

![Datenfluss](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/Datenfluss%20Resy.png)

| Task | Ultraschallsensor Links | Ultraschallsensor Front | Ultraschallsensor Rechts |
| ---- | ----------------------- | ----------------------- | ------------------------ |
| Ultraschallsensor 1 (U1) | 1 | 0 | 0 |
| Ultraschallsensor 2 (U2) | 0 | 3 | 0 |
| Ultraschallsensor 3 (U3) | 0 | 0 | 5 |
| Motor (Mainthread) (M) | 0 | 0 | 0 |
| Pi(R) | U1 | U2 | U3 |

| Task | Distanzvariable 1 | Distanzvariable 2 | Distanzvariable 3 | Motor |
| ---- | ----------------- | ----------------- | ----------------- | ----- |
| Ultraschallsensor 1 (U1) | 2 | 0 | 0 | 0 |
| Ultraschallsensor 2 (U2) | 0 | 4 | 0 | 0 |
| Ultraschallsensor 3 (U3) | 0 | 0 | 6 | 0 |
| Motor (Mainthread) (M) | 7 | 8 | 9 | 10 |
| Pi(R) | U1 | U2 | U3 | M |

#### Blockierzeit bei Unterbrechnungssperren

*Hinweis 2: In unserem Realzeitsystem gibt es keine Blockierzeiten, da wir uns entgegen der ursprünglichen Planung gegen einen mutex-geschützten, geteilten Speicherbereich entschieden haben. Weil der Motorthread nur lesend auf die Werte zugreift, kann es nicht zu einer Racecondition kommen.*

*Hinweis 3: Im Projekt werden die Protokolle PIP und PCP nicht verwendet, weshalb keine Berechnung erfolgen muss. Der Grund für dies ist, dass alle Threads die gleichen Prioritäten haben. Außerdem verwendeten wir keine blockierten Bereiche, weshalb die Implementierung von PIP und PCP unnötig wäre.*

### Schedulingtest

| Task | tP,min in ms | tE,max in ms | tD,max in ms |
| ---- | ------------ | ------------ | ------------ |
| Ultraschallsensor 1|(U1) | 250 | 21 | 250 |
| Ultraschallsensor 2 (U2) | 250 | 22 | 250 |
| Ultraschallsensor 3 (U3) | 250 | 21 | 250 |
| Motor (Mainthread) (M) | 40 | 3,2 | 40 |

#### Blockierzeiten

*siehe Hinweis 2*

## Realzeitnachweis mit Last

*Hinweis 1: Bessere Darstellung der Formel im Buch "Moderne Realzeitsysteme kompakt" Im folgenden werden wir einen Realzeitnachweis auf Anwendungsebene durchführen, dabei gehen wir von einer idealen Hardware und einem idealen Betriebssysteme aus. Für weitere Informationen zu Schwierigkeiten, siehe Punkt "Probleme und Lösungen"
Zeiten*

### Eigener Skript

| Task | tP,min in ms | tD,min in ms | tD,max in ms | tE,min in ms | tE,max in ms |
| ---- | ------------ | ------------ | ------------ | ------------ | ------------ |
| Ultraschallsensor 1 | 250 | 0 | 250 | 1 | 21 |
| Ultraschallsensor 2 | 250 | 0 | 250 | 0,7 | 22 |
| Ultraschallsensor 3 | 250 | 0 | 250 | 0,9 | 22 |
| Motor (Mainthread) | 40 | 0 | 40 | 0,6 | 3,2 |

### Stresstest mit 2000 IOs

| Task | tP,min in ms | tD,min in ms | tD,max in ms | tE,min in ms | tE,max in ms | Blockierzeit in ms |
| ---- | ------------ | ------------ | ------------ | ------------ | ------------ | ------------------ |
| Ultraschallsensor 1 | 250 | 0 | 250 | 1 | 21 | 3 |
| Ultraschallsensor 2 | 250 | 0 | 250 | 1 | 22 | 3 |
| Ultraschallsensor 3 | 250 | 0 | 250 | 1 | 22 | 3 |
| Motor (Mainthread) | 40 | 0 | 40 | 10 | 2 | 32 |

*Hinweis: Da beim Motorthread ständig auf das Sysfilesystem zugegriffen wird, ergeben sich lange Blockierzeiten, wenn das Programm Stress mit vielen IO-Threads ausgeführt wird. Bei den Ultraschallsensoren werden auf WiringPi-Funktion zugegriffen, welche direkt in die Register der GPIO-IC geschrieben werden, deshalb ändert sich hier an den Zeiten nichts. Beim Motorthread erhöht sich die Blockierzeit auf das ca. 32-fache. Die Deadline von 40ms wird in diesem Fall trotzdem eingehalten.*

#### Berechnung der Blockierzeit:

> Ultraschallsensoren tE,max = Executiontime mit Blockierzeit - Blockierzeit

> Motor tE,max = Executiontime mit Blockierzeit - Blockierzeit

### Scheduling

Verwendeter Scheduler: prioritätsgesteuertes Scheduling

| Task | Priorität |
| ---- | --------- |
| Ultraschallsensor 1 | 99 |
| Ultraschallsensor 2 | 99 |
| Ultraschallsensor 3 | 99 |
| Motor | 99 |

### Unterbrechbarkeit und Abhängigkeit

| Task | Unterbrechbarkeit | Abhängigkeit |
| ---- | ----------------- | ------------ |
| Ultraschallsensor 1 | - | - |
| Ultraschallsensor 2 | - | - |
| Ultraschallsensor 3 | - | - |
| Motor (Mainthread) | - | Ultraschallsensor 1 - 3 |

### Ressourcenbenutzung

| Task | Ressource | Dauer |
| ---- | --------- | ----- |
| Ultraschallsensor 1 | Linker Ultraschallsensor | 207ms |
| Ultraschallsensor 2 | Front Ultraschallsensor | 207ms |
| Ultraschallsensor 3 | Rechter Ultraschallsensor | 207ms |
| Motor (Mainthread) | Motor | 40ms |

### Grundlage

Der Putzroboter ist ein Realzeitsystem, da er beide Realzeitbedingungen erfüllt:

#### Erste Bedingung

| Task | Pges = Summe(tE,max / tP,min) | größer/gleich |  c  | Bedingung: Pges <= c |
| ---- | ----------------------------- | ------------- | --- | -------------------- |
| Ultraschallsensor 1 | 0,084 | <= | - | - |
| Ultraschallsensor 2 | 0,088 | <= | - | - |
| Ultraschallsensor 3 | 0,088 | <= | - | - |
| Motor (Mainthread) | 0,08 | <= | - | - |
| Gesamt | 0,335 | <= | 1 | Erfüllt |

#### Zweite Bedingung

| Task | tD,min | größer/gleich | tR,min | größer/gleich | tR,max | größer/gleich | tD,max | Bedingung |
| ---- | ------ | ------------- | ------ | ------------- | ------ | ------------- | ------ | --------- |
| Ultraschallsensor 1 | 0 | <= | 20 | <= | 207 | <= | 250 | Erfüllt |
| Ultraschallsensor 2 | 0 | <= | 20 | <= | 207 | <= | 250 | Erfüllt |
| Ultraschallsensor 3 | 0 | <= | 20 | <= | 207 | <= | 250 | Erfüllt |
| Motor (Mainthread) | 0 | <= | 3,2 | <= | 33 | <= | 40 | Erfüllt |

*Nachweis ohne Berücksichtigung der Ressourcen*

### Hinreichender Schedulingtest

#### Formel 1:

> u = Summe(tE,max / ( min( tD,max ; tP,min ))) <= n(21/n - 1); Summe -> Oben n und Unten j=1

| Task | Summe(tE,max / ( min( tD,max ; tP,min ))) | größer/gleich | n(21/n - 1) |
| ---- | ----------------------------------------- | ------------- | ----------- |
| Ultraschallsensor 1 | 8,4% | <= | - |
| Ultraschallsensor 2 | 8,8% | <= | - |
| Ultraschallsensor 3 | 8,4% | <= | - |
| Motor (Mainthread) | 8% | <= | - |
| Gesamt | 33,5% | <= | 75,6% |

*Auslastungsgrenze bei 4 Threads liefert eine Aussage -> Kein notwendiger Schedulertest nötig. 
 Nachweis unter Berücksichtigung der Ressourcen*

### Blockierzeit

#### Datenfluss / Ressourcenverwendung

![Datenfluss](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/Datenfluss%20Resy.png)

| Task | Ultraschallsensor Links | Ultraschallsensor Front | Ultraschallsensor Rechts |
| ---- | ----------------------- | ----------------------- | ------------------------ |
| Ultraschallsensor 1 (U1) | 1 | 0 | 0 |
| Ultraschallsensor 2 (U2) | 0 | 3 | 0 |
| Ultraschallsensor 3 (U3) | 0 | 0 | 5 |
| Motor (Mainthread) (M) | 0 | 0 | 0 |
| Pi(R) | U1 | U2 | U3 |

| Task | Distanzvariable 1 | Distanzvariable 2 | Distanzvariable 3 | Motor |
| ---- | ----------------- | ----------------- | ----------------- | ----- |
| Ultraschallsensor 1 (U1) | 2 | 0 | 0 | 0 |
| Ultraschallsensor 2 (U2) | 0 | 4 | 0 | 0 |
| Ultraschallsensor 3 (U3) | 0 | 0 | 6 | 0 |
| Motor (Mainthread) (M) | 7 | 8 | 9 | 10 |
| Pi(R) | U1 | U2 | U3 | M |

### Blockierzeit bei Unterbrechnungssperren

*Hinweis 2: In unserem Realzeitsystem gibt es keine Blockierzeiten, da wir uns entgegen der ursprünglichen Planung gegen einen mutex-geschützten, geteilten Speicherbereich entschieden haben. Weil der Motorthread nur lesend auf die Werte zugreift, kann es nicht zu einer Racecondition kommen.*

*Hinweis 3: Im Projekt werden die Protokolle PIP und PCP nicht verwendet, weshalb keine Berechnung erfolgen muss. Der Grund für dies ist, dass alle Threads die gleichen Prioritäten haben. Außerdem verwendeten wir keine blockierten Bereiche, weshalb die Implementierung von PIP und PCP unnötig wäre.*

### Schedulingtest

| Task | tP,min in ms | tE,max in ms | tD,max in ms |
| ---- | ------------ | ------------ | ------------ |
| Ultraschallsensor 1 (U1) | 250 | 21 | 250 |
| Ultraschallsensor 2 (U2) | 250 | 22 | 250 |
| Ultraschallsensor 3 (U3) | 250 | 22 | 250 |
| Motor (Mainthread) (M) | 40 | 3,2 | 40 |

### Blockierzeiten

*siehe Hinweis 2*

## Design

### Verteilung der Aufgaben auf verschiedene Threads

Für die einzelnen Ultraschallsensoren wird jeweils ein Thread erzeugt. Jeder Thread hat die Aufgabe, die Messungen des ihm zugeordneten Sensors durchzuführen und die Daten in eine absolute Entfernung umzurechnen. Das Ergebnis wird in einer Variable abgespeichert.

- Minimale zulässige Reaktionszeit (tDmin): 0ms
- Maximale zulässige Reaktionszeit (tDmax): 250ms
- Maximale Auftrittshäufigkeit (tPmin): 250ms

Beide Motoren werden von einem Thread angesteuert. Dieser Thread wertet die von den Ultraschallsensoren gelieferten Abstandswerte aus, und entscheidet anhand dieser Daten, mit welcher Geschwindigkeit die Motoren angesteuert werden. Zum Beispiel kann der Roboter bei einem sich nähernden Hindernis die Geschwindigkeit reduzieren und sich um 25° drehen, wenn er direkt vor einem Hindernis steht. Auch hier ist das Einhalten der Realzeitbedingungen notwendig, da das Wissen eines nahenden Zusammenstoßes (also die rechtzeitig vorhandenen Abstandswerte der Ultraschallsensoren) nichts bringt, wenn die Roboter seine Motoren trotzdem nicht rechtzeitig stoppen kann. Wichtig ist außerdem, dass die Geschwindigkeitsanpassung beider Motoren direkt nacheinander erfolgt (zum Beispiel könnte diese Sektion durch einen Mutex geschützt sein), um zu verhindern, dass die beiden Motoren mit einem zu großen zeitlichen Abstand angesteuert werden, was eine ungewollte Drehung zur Folge hätte (angenommen, Motor 1 fährt los und Motor 2 reagiert erst 1 Sekunde später).
 
- Minimale zulässige Reaktionszeit (tDmin): 0ms
- Maximale zulässige Reaktionszeit (tDmax): 40ms
- Maximale Auftrittshäufigkeit (tPmin): 40ms

### Theorie / Berechnungen

Wir sind bei der Berechnung der maximal zulässigen Reaktionsgeschwindigkeit der einzelnen Funktionen von folgenden Annahmen ausgegangen:

> Der Roboter schafft beim geradeausfahren eine Geschwindigkeit von 0,4m/s (= 4m pro 10s). Tatsächlicher gemessener Wert = 0,42m/s

> Der Bremsweg beträgt bei voller Geschwindigkeit zwischen 6 und 7cm

> Das Auswerten der Ultraschallsensoren benötigt pro Sensor 100ms, bei den vorhandenen drei Sensoren wären das insgesamt maximal 300ms.

> Die Ansteuerung der Motoren inklusive der Entscheidungsfindung über die zu fahrende Geschwindigkeit sollte nicht länger als 300ms dauern.

Aus den oben aufgelisteten Einschränkungen lassen sich nun weitere Schlüsse ziehen:

> 0,42m/s * (0,2s Sensorauswertung * 3 Sensoren + 0,3s Motoransteuerung) + 0,07m Bremsweg = 0,448m

- Zeit der Motoransteuerung wird angenommen

Das heißt, das unser Roboter spätestens bei einem Sicherheitsabstand von 0,49m zu einem Hindernis eine Bremsung einleiten muss. Eventuell wäre es sinnvoll, schon vorher ein wenig abzubremsen, um den "Sicherheitsabstand" zu Hindernis verkürzen zu können und damit die gereinigte Fläche zu vergrößern.

Zu beachten ist, dass der Raspberry keine Hardware-PWM besitzt, sondern nur Software-PWM. Somit sind je nach PWM-Bibliothek noch weitere Threads in den Realzeitnachweis mit ein zu berechnen.

### Datenfluss

![Datenfluss](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/Datenfluss%20Resy.png)

### Schaltplan

![Schaltplan](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/Schaltplan.png)

### Belegungsplan - Raspberry Pi

| Pin | Bezeichnung | Belegung | Define |
| --- | ----------- | -------- | ------ |
| 1 | 3V3 | NC | n/a |
| 2 | 5V | L293D -> Pin 2, Ultraschallsensor 1/2/3 -> Pin VCC | n/a |
| 3 | GPIO2 | NC | n/a |
| 4 | 5V | NC | n/a |
| 5 | GPIO3 | NC | n/a |
| 6 | Ground | L293D - Pin 7/8/9/10, Ultraschallsensor 1/2/3 -> Pin GND | n/a |
| 7 | GPIO4 | Ultraschallsensor 1 -> Pin Echo (über Spannungsteiler R1 und R2) | GPIO_US_LEFT_ECHO |
| 8 | GPIO14 | NC | n/a |
| 9 | Ground | NC | n/a |
| 10 | GPIO15 | NC | n/a |
| 11 | GPIO17 | Ultraschallsensor 1 -> Pin Trig | GPIO_US_LEFT_TRIG |
| 12 | GPIO18 | L293D -> Pin 1 (Motor 1 <- EN) | GPIO_MOTOR_LEFT_ENABLE |
| 13 | GPIO27 | Ultraschallsensor 2 -> Pin Echo (über Spannungsteiler R3 und R4) | GPIO_US_FRONT_ECHO |
| 14 | Ground | NC | n/a |
| 15 | GPIO22 | Ultraschallsensor 2 -> Pin Trig | GPIO_US_FRONT_TRIG |
| 16 | GPIO23 | L293D -> Pin 3 (Motor 1 <- Input 1) | GPIO_MOTOR_LEFT_FORWARD |
| 17 | 3V3 | NC | n/a |
| 18 | GPIO24 | L293D -> Pin 7 (Motor 1 <- Input 2) | GPIO_MOTOR_LEFT_BACKWARD |
| 19 | GPIO10 | Ultraschallsensor 3 -> Pin Echo (über Spannungsteiler R5 und R6) | GPIO_US_RIGHT_ECHO |
| 20 | Ground | NC | n/a |
| 21 | GPIO09 | Ultraschallsensor 3 -> Pin Trig | GPIO_US_RIGHT_TRIG |
| 22 | GPIO25 | L293D -> Pin 16 (Motor 2 <- EN) | GPIO_MOTOR_RIGHT_ENABLE |
| 23 | GPIO11 | NC | n/a |
| 24 | GPIO8 | L293D -> Pin 4 (Motor 2 <- Input 1) | GPIO_MOTOR_RIGHT_FORWARD |
| 25 | Ground | NC | n/a |
| 26 | GPIO7 | L293D -> Pin 14 (Motor 2 <- Input 2) | GPIO_MOTOR_RIGHT_BACKWARD |

### Belegungsplan - Restliche elektronische Bauteile

| Pin | Bauteil |  ZU  | Pin | Bauteil |
| --- | ------- | ---- | --- | ------- |
| 15 | L293D | ZU | VDD | Batterie |
| 5 | L293D | ZU | VDD | Motor 1 |
| 11 | L293D | ZU | VSS | Motor 1 |
| 6 | L293D | ZU | VDD | Motor 2 |
| 12 | L293D | ZU | VSS | Motor 2 |
| VSS | Ultraschallsensor 1/2/3 | ZU | Pin 1 | Widerstand R1/R3/R5 = 660 Ohm |
| ECHO | Ultraschallsensor 1/2/3 | ZU | Pin 1 | Widerstand R2/R4/R6 = 330 Ohm |

### Verteilung der Prioritäten:

| Thread | Priorität |
| ------ | --------- |
| Ultraschallsensor 1 | 99 |
| Ultraschallsensor 2 | 99 |
| Ultraschallsensor 3 | 99 |
| Motor | 99 |

### Anwendungsszenarien

Haushaltshilfe: Der Roboter soll die Bodenreinigung im Haushalt übernehmen.

#### Verwendete Hardware

- HC-SR04 Modul (Ultraschall-Abstandssensor)
- Widerstände (330Ω, 470Ω, 820 Ohm)
- L293D (Motoransteuerungs-IC)
- 2WD Robot Motor Chassis

#### Gehäuseplanung

Erste Planung des Gehäuses für den 3D-Drucker

![Gehaeuseplanung](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/Gehaeuseplanung.jpg)

## Milestones

### Gesetzte Milestones

- Zusammenbau des Roboters
- Verdrahtung der elektrischen Bauteile
- Ansteuern und kalibrieren der Ultraschallsensoren
- Auswertung der Daten von den Ultraschallsensoren
- Implementierung der Geschwindigkeitsregelung nach den oben genannten Realzeitbedingungen
- Auswertung der gemessenen Zeiten für die Belegung des Realzeitbetriebes
- Formeller Realzeitnachweis

### Erreichte Milestones

- Zusammenbau des Roboters
- Verdrahtung der elektrischen Bauteile
- Ansteuern und kalibrieren der Ultraschallsensoren
- Auswertung der Daten von den Ultraschallsensoren
- Entwicklung des Gehäuses
- Implementierung der Geschwindigkeitsregelung nach den oben genannten Realzeitbedingungen
- Zusammenbau des Roboters
- Auswertung der gemessenen Zeiten für die Belegung des Realzeitbetriebes
- Formeller Realzeitnachweis

## Probleme und Lösungen

### Bereich Programmierung

Zahlenüberlauf eines long integer in der Abstandsberechnung mithilfe der Daten der Ultraschallsensoren. Das Problem trat nur innerhalb einer Zwischenrechnung in einer Zeile auf und ist uns daher zuerst nicht aufgefallen.

> Lösung: Verwendung eines unsigned long long Datentyps

### Bereich Mechanik 

Durch das freirotierbare Steuerrad am hinteren Ende des Roboters, wird die Fahrtrichtung so abgelenkt, dass der Roboter nicht geradeaus fährt sondern einen Bogen nach links oder nach rechts fährt.

> Lösung: Hinterrad fixieren

### Bereich Hardware/Programmierung

Die Ultraschallsensoren geben Störwerte aus, wenn der Abstand zu einem Objekt außerhalb des Messbereichs ist, sprich wenn ein Objekt entweder kürzer als 3cm entfernt ist oder weiter als 3,5m entfernt ist. Da die fehlerhaften Werte innerhalb eines plausiblen Wertebereichs liegen, "erkennt" der Roboter in solchen Fällen Hindernisse wo keine sind und dreht sich dann...

Diese Störungen haben wir versucht mit einem Oszilloskop zu debuggen:

![IMG_20150623_203904](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/IMG_20150623_203904.jpg)

Im folgenden Bild ist das Verhalten des Ultraschallsensors bei vernünftigen Werten zu erkennen. Auch schön zu sehen ist unsere Periode von 250ms. Der Channel 1 (oben) zeigt das Echo-Signal während der Channel 2 (unten) den Trigger zeigt (letzterer wird bei dieser Zoomstufe aufgrund der geringen Auflösung des Oszilloskop-Bildschirms nur manchmal dargestellt; es existiert aber kurz vor jeder Echo-HIGH-Flanke):

![IMG_20150623_224747](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/IMG_20150623_224747.jpg)

In folgendem Bild sieht man hingegen das Fehlverhalten des Ultraschallsensors bei out-of-range. Es sind immer wieder sehr kurze Flanken zu sehen.

![IMG_20150623_204752](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/IMG_20150623_204752.jpg)

Wenn der Sensor konstant "out-of-range" detektiert, treten diese Flanken genau jede zweite Messung auf. Nun könnte man auf die Idee kommen, einfach jede zweite Messung zu verwerfen - das war auch unser erster Ansatz:

![IMG_20150623_223602](https://github.com/Dav1dS1mon/de.htwg.resy.robot/blob/master/Ressourcen%20Dokumentation/IMG_20150623_223602.jpg)

Leider treten diese falschen Messwerte in der Praxis (z.B. bei Stuhlbeinen, welche auf Entfernung nur unzuverlässig detektiert werden, weil sie zu schmal sind) nur sehr unregelmäßig auf - teils gibt es auch mehrere falsche Messwerte in Folge. -> Schlussfolgerung: Durch weitere Testvorgänge kamen wir zum Entschluss, dass ein Ultraschallsensor defekt ist und deshalb diese Problematik auftritt

> Lösung: Keine

## Quellen

[Raspberry Pi - Ultraschallsensor](http://www.tutorials-raspberrypi.de/gpio/entfernung-messen-mit-ultraschallsensor-hc-sr04/) 

- Aufruf 16.04.2015, 17:10

[Dokumentation, Realzeitnachweis und Codebeispiele](http://www.amazon.de/Moderne-Realzeitsysteme-kompakt-Einf%C3%BChrung-Embedded/dp/3898648303) 

- Verwendetes Buch
