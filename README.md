| Systemy Operacyjne - _PROJEKT_ | Kamil Gargula | GR_01, II rok, st. stacjonarne | Semestr zimowy 2025/26 | [GitHub repo](https://github.com/75gk2/SystemyOperacyjne_projekt_SOR) |
| :----------------------------- | :------------ | :----------------------------- | :--------------------- | :-------------------------------------------------------------------- |

# Raport – Projekt „SOR”

## 1. Założenia projektowe (z tematu)
- Symulacja SOR realizowana wieloprocesowo z użyciem `fork()` i `exec()`.
- Komunikacja między procesami oparta o kolejki komunikatów i pamięć dzieloną.
- Synchronizacja liczników kolejek realizowana semaforami System V.
- Rejestracja posiada 2 okienka, z dynamicznym sterowaniem:
  - $K_1 = N/2$ → otwarcie 2. okienka.
  - $K_2 = N/3$ → zamknięcie 2. okienka.
- Priorytety w triażu: czerwony, żółty, zielony.
- VIP w rejestracji obsługiwany bez kolejki.
- Sygnały dyrektora:git push
  - `SIGUSR1` → przerwanie pracy lekarza i czasowa przerwa.
  - `SIGUSR2` → ewakuacja wszystkich procesów.

## 2. Architektura i przepływ
### 2.1 Procesy (odpowiedzialności)
- `main`  uruchamia symulacje i tworzy procesy przez `ProcessManager`.
  Pliki: [app/main/src/main.cpp](app/main/src/main.cpp), [app/main/src/scenarios/simulateTriage.cpp](app/main/src/scenarios/simulateTriage.cpp)


## 5. Testy (4)

### Test: Statystyki triażu i kolejność ich obsłużenia przez lekarzy. 
([Plik testu](app/main/src/scenarios/simulateNoDoctors.cpp))

Test polega na spawnie 10.000 pacjentów i poczekalni o równym rozmiarze. 
Następnie czekam na wszystkich aż przejdą przez rejestrację, i dopiero wtedy następuje uruchomienie lekarzy.
Test sprawdza czy zapisy do lekarzy działają i żadan nie zostanie pominięty nawet jeśli lekarza nie ma w gabinecie.

```
grep -c 'Assigning Patient process' sor.log
10000

grep -c 'triage result color=DISMISSED' sor.log
483

grep -c 'triage result color=RED' sor.log
1039

grep -c 'triage result color=GREEN' sor.log
5021

grep -c 'triage result color=YELLOW' sor.log
3457

grep -c 'Patient: doctor outcome=home' sor.log
8095

grep -c 'Patient: doctor outcome=hospital' sor.log
1383

grep -c 'Patient: doctor outcome=redirect' sor.log
39

grep -c 'Patient: doctor outcome=unknown' sor.log
0
```

`10000 - 483 = 9517` → ilość pacjentów bez tych zdyskfalifikowanych do dalszej diagnozy

`1039 + 5021 + 3457 = 9517 `→ ilość pacjentów zakwalifikowanych do dalszej diagnozy

`8095 + 1383 + 39 + 0 = 9517` → Ilość pacjentów po wizycie u specjalisty

### 1) Wynik triażu (10 000 pacjentów)

| Kategoria triażu |     Liczba | % z 10 000 | cel  |
| ---------------- | ---------: | ---------: | ---- |
| DISMISSED        |        483 |      4,83% | 5%   |
| RED              |      1 039 |     10,39% | 10%  |
| YELLOW           |      3 457 |     34,57% | 35%  |
| GREEN            |      5 021 |     50,21% | 50%  |
| **Suma**         | **10 000** |       100% | 100% |

### 2) Wynik wizyty u specjalisty (dla 9 517 pacjentów)

| Outcome  |    Liczba |   % z 9 517 | cel   |
| -------- | --------: | ----------: | ----- |
| home     |     8 095 |      85,06% | 85%   |
| hospital |     1 383 |      14,53% | 14,5% |
| redirect |        39 |       0,41% | 0,5%  |
| unknown  |         0 |       0,00% | 0%    |
| **Suma** | **9 517** | **100,00%** | 100%  |


### Test z 50k pacjentów działający bez sleepów - same busy waity.
[Plik testu](app/main/src/scenarios/simulateFullLargeFlowNoDelay.cpp)
Gotowy schemat znajduje się w liście symulacji.

Założenia: 50.000 pacjentów. Pojemność poczekalni również 50.000
Sprawia to że wszyscy przechodzą przez poczekalnie, i w niej czekają zapisania do lekarzy. Wąskim gardłem jest poczekalna którą ustwaiłem na 1% liczby pacjentów (500)

Symulacja przebiegła i zamknęła się normalnie.
Pod koniec sprawdziłem i nie znalazłem żadnych niedokończonych procesów:
![alt text](image.png)

Wynik następujący:
```
grep -c 'Assigning Patient process' sor.log
50000
grep -c 'Patient: starting' sor.log
50000
grep -c 'triage result color=DISMISSED' sor.log
2574
grep -c 'Patient: doctor outcome=home' sor.log
40402
grep -c 'Patient: doctor outcome=hospital' sor.log
6818
grep -c 'Patient: doctor outcome=redirect' sor.log
206
grep -c 'Patient: doctor outcome=unknown' sor.log
0
```



Efekt jest taki:
`2574 + 40402 + 6818 + 206 = 50000`
To znaczy że wszyscy pacjenci pomyslnie zostali obsłużeni na bardzo dużej grupie i bez sleepów.


| Wynik (doctor outcome)    |    Liczba | % bez wypisanych na triażu |
| ------------------------- | --------: | -------------------------: |
| home                      |     40402 |                     85.16% |
| hospital                  |      6818 |                     14.38% |
| redirect                  |       206 |                      0.43% |
| **Suma (bez wypisanych)** | **47426** |                       100% |
| dismissed (wypisani)      |      2574 |           5.15% (z 50.000) |
| **suma (z wypisanymi)**   | **50000** |                            |


### Test mechaniki okienek rejestracji
[Plik testu](app/main/src/scenarios/simulateWindowsReception.cpp)


Założenia: 
Operacje na okienkach. Jeśli w kolejce do rejestracji jest:
  - więcej niż $K_1=N/2$ pacjentów $→$ otwórz 2. okienko, jeśli zamknięte
  - mniej niż $K_2=N/3$ pacjentów $→$ zamknij 2. okienko, jeśli otwarte

Tak więc jedno okienko jest otwierane gdy liczba kolejki przekroczy próg. Poniżej demonstruję działanie dla 5 osobnych fali po 1000 pacjentów w kolejce, gdy N = 1000.

Czyli dla N = 1000 :
    jeśli pacjentów > 500 (tj. ≥ 501 ) → otwórz 2. okienko, jeśli zamknięte,
    jeśli pacjentów < 333 (tj. ≤ 332 ) → zamknij 2. okienko, jeśli otwarte.



```
cat registration.log | grep RegistrationWindow -C 3
[13:36:24.293] [tid 401454] [info] RegistrationWindow, initialized window, windowId=1
[13:36:24.293] [tid 401444] [info] RegistrationManagment: count=0
[13:36:24.294] [tid 401444] [info] RegistrationManagment: count=0
[13:36:24.294] [tid 401444] [info] RegistrationManagment: count=0
--
[13:36:24.348] [tid 401444] [info] RegistrationManagment: count=499
[13:36:24.348] [tid 401444] [info] RegistrationManagment: count=500
[13:36:24.348] [tid 401444] [info] RegistrationManagment: count=501
[13:36:24.348] [tid 401966] [info] RegistrationWindow, initialized window, windowId=2
[13:36:24.348] [tid 401444] [info] RegistrationManagment: count=502
[13:36:24.348] [tid 401444] [info] RegistrationManagment: count=502
[13:36:24.348] [tid 401444] [info] RegistrationManagment: count=501
--
[13:36:26.019] [tid 401444] [info] RegistrationManagment: count=334
[13:36:26.021] [tid 401444] [info] RegistrationManagment: count=333
[13:36:26.024] [tid 401444] [info] RegistrationManagment: count=332
[13:36:26.026] [tid 401966] [info] RegistrationWindow: received shutdown in window 2
[13:36:26.026] [tid 401966] [info] RegistrationWindow: Shutting down window 2
[13:36:26.026] [tid 401444] [info] RegistrationManagment: count=332
[13:36:26.029] [tid 401444] [info] RegistrationManagment: count=331
[13:36:26.034] [tid 401444] [info] RegistrationManagment: count=330
--
[13:36:30.473] [tid 401444] [info] RegistrationManagment: count=500
[13:36:30.474] [tid 401444] [info] RegistrationManagment: count=501
[13:36:30.474] [tid 401444] [info] RegistrationManagment: count=501
[13:36:30.474] [tid 402971] [info] RegistrationWindow, initialized window, windowId=2
[13:36:30.474] [tid 401444] [info] RegistrationManagment: count=500
[13:36:30.474] [tid 401444] [info] RegistrationManagment: count=501
[13:36:30.474] [tid 401444] [info] RegistrationManagment: count=502
--
[13:36:32.142] [tid 401444] [info] RegistrationManagment: count=334
[13:36:32.147] [tid 401444] [info] RegistrationManagment: count=333
[13:36:32.147] [tid 401444] [info] RegistrationManagment: count=332
[13:36:32.152] [tid 402971] [info] RegistrationWindow: received shutdown in window 2
[13:36:32.152] [tid 402971] [info] RegistrationWindow: Shutting down window 2
[13:36:32.152] [tid 401444] [info] RegistrationManagment: count=332
[13:36:32.152] [tid 401444] [info] RegistrationManagment: count=331
[13:36:32.157] [tid 401444] [info] RegistrationManagment: count=330
--
[13:36:36.625] [tid 401444] [info] RegistrationManagment: count=499
[13:36:36.625] [tid 401444] [info] RegistrationManagment: count=500
[13:36:36.625] [tid 401444] [info] RegistrationManagment: count=501
[13:36:36.625] [tid 403971] [info] RegistrationWindow, initialized window, windowId=2
[13:36:36.625] [tid 401444] [info] RegistrationManagment: count=502
[13:36:36.625] [tid 401444] [info] RegistrationManagment: count=502
[13:36:36.625] [tid 401444] [info] RegistrationManagment: count=501
--
[13:36:38.289] [tid 401444] [info] RegistrationManagment: count=333
[13:36:38.289] [tid 401444] [info] RegistrationManagment: count=333
[13:36:38.294] [tid 401444] [info] RegistrationManagment: count=332
[13:36:38.299] [tid 401454] [info] RegistrationWindow: received shutdown in window 1
[13:36:38.299] [tid 401454] [info] RegistrationWindow: Shutting down window 1
[13:36:38.299] [tid 401444] [info] RegistrationManagment: count=331
[13:36:38.299] [tid 401444] [info] RegistrationManagment: count=331
[13:36:38.299] [tid 401444] [info] RegistrationManagment: count=330
--
[13:36:42.829] [tid 401444] [info] RegistrationManagment: count=499
[13:36:42.830] [tid 401444] [info] RegistrationManagment: count=500
[13:36:42.830] [tid 401444] [info] RegistrationManagment: count=501
[13:36:42.830] [tid 404979] [info] RegistrationWindow, initialized window, windowId=1
[13:36:42.830] [tid 401444] [info] RegistrationManagment: count=501
[13:36:42.830] [tid 401444] [info] RegistrationManagment: count=502
[13:36:42.830] [tid 401444] [info] RegistrationManagment: count=501
--
[13:36:44.474] [tid 401444] [info] RegistrationManagment: count=334
[13:36:44.476] [tid 401444] [info] RegistrationManagment: count=333
[13:36:44.479] [tid 401444] [info] RegistrationManagment: count=332
[13:36:44.481] [tid 404979] [info] RegistrationWindow: received shutdown in window 1
[13:36:44.481] [tid 404979] [info] RegistrationWindow: Shutting down window 1
[13:36:44.481] [tid 401444] [info] RegistrationManagment: count=332
[13:36:44.484] [tid 401444] [info] RegistrationManagment: count=331
[13:36:44.489] [tid 401444] [info] RegistrationManagment: count=330
--
[13:36:49.103] [tid 401444] [info] RegistrationManagment: count=500
[13:36:49.104] [tid 401444] [info] RegistrationManagment: count=501
[13:36:49.104] [tid 401444] [info] RegistrationManagment: count=501
[13:36:49.104] [tid 405989] [info] RegistrationWindow, initialized window, windowId=1
[13:36:49.104] [tid 401444] [info] RegistrationManagment: count=500
[13:36:49.104] [tid 401444] [info] RegistrationManagment: count=501
[13:36:49.104] [tid 401444] [info] RegistrationManagment: count=502
--
[13:36:50.724] [tid 401444] [info] RegistrationManagment: count=334
[13:36:50.729] [tid 401444] [info] RegistrationManagment: count=333
[13:36:50.729] [tid 401444] [info] RegistrationManagment: count=332
[13:36:50.734] [tid 403971] [info] RegistrationWindow: received shutdown in window 2
[13:36:50.734] [tid 403971] [info] RegistrationWindow: Shutting down window 2
[13:36:50.734] [tid 401444] [info] RegistrationManagment: count=332
[13:36:50.735] [tid 401444] [info] RegistrationManagment: count=331
[13:36:50.740] [tid 401444] [info] RegistrationManagment: count=330
```

### Test: wszyscy pacjeńci do jednego doktora z czerwonym priorytetem (1000)

Test polegał na uruchomieniu 1000 pacjentów dla jednego lekarza z priorytetem wysokim i porównanie logów wyjściowych - powinny się pokrywać.

Został uruchomiony z dużą pojemnością poczekalni i bez pacjentów VIP. Tym sposobem unikamy asynchronicznego przydzielania kolejności.

Następnie po zebraniu logów do pliku wyeksportowałem potrzebne logi:
```
grep  'Patient: starting' sor.log > starting_seqence
grep  'Patient: doctor outcome' sor.log > ending_seqence
```

Następnie użyłem narzędzia _Replace with regex_ edytora VSCode, aby usunąć zbędny tekst i zostawić tylko pid procesów:
`^.*?\[tid\s+(\d+)\].*$` → `$1`


W ostatnim już kroku sprawdziłem różnice funkcją _compare files_. 
*Test wykazał w zależności od próby 10, 13 a także 19 przeniesionych linijek*
Taki rezultat jest niejako oczekiwany ze względu na margines błędu jednoczesnych operacji, a takżę asynchroniczności sinka spdloggera.

Potwierdza to rozproszenie wyników od oczekiwanych, czyli zamiana na wyłąćznie jeden proces jak poniżejL
![alt text](image-1.png)
Dodatkowo oba pliki miały 1000 linijek, co potwierdza hermetyczność sygnału


*Uwaga:* aby odtworzyć test należy odkomentować linijkę w pliku
[app/triage/src/main.cpp](app/triage/src/main.cpp#L68)
`triageOut.send(Triage::Q_TRIAGE_OUT_STRUCT{Patient::RED,Triage::PEDIATRICIAN, false}, patient.socialId); continue;`


---

## 6. Linki do kodu – wymagane konstrukcje systemowe

### a) Tworzenie i obsługa plików
- `open()` – [lib/src/ProcessManager.cpp#L134](lib/src/ProcessManager.cpp#L134)
- `close()` – [lib/src/ProcessManager.cpp#L147](lib/src/ProcessManager.cpp#L147)
- `read()` – [test/PathResolvingTest.cpp#L35](test/PathResolvingTest.cpp#L35)
- `write()` – [lib/src/ProcessManager.cpp#L139](lib/src/ProcessManager.cpp#L139)

### b) Tworzenie procesów
- `fork()` – [lib/src/ProcessManager.cpp#L65](lib/src/ProcessManager.cpp#L65)
- `exec()` (np. `execv`) – [lib/src/ProcessManager.cpp#L80](lib/src/ProcessManager.cpp#L80)
- `_exit()` – [lib/src/ProcessManager.cpp#L84](lib/src/ProcessManager.cpp#L84)
- `wait()` (np. `waitpid`) – [lib/src/Process.cpp#L37](lib/src/Process.cpp#L37)

### c) Tworzenie i obsługa wątków
- `pthread_create()` – [app/registration/src/main.cpp#L165](app/registration/src/main.cpp#L165)
- `pthread_join()` – [app/registration/src/main.cpp#L210](app/registration/src/main.cpp#L210)
- `pthread_mutex_lock()` – [app/registration/src/main.cpp#L77](app/registration/src/main.cpp#L77)
- `pthread_mutex_unlock()` – [app/registration/src/main.cpp#L85](app/registration/src/main.cpp#L85)
- `pthread_cond_wait()` – [app/registration/src/main.cpp#L206](app/registration/src/main.cpp#L206)
- `pthread_cond_signal()` – [app/registration/src/main.cpp#L84](app/registration/src/main.cpp#L84)

### d) Obsługa sygnałów
- `kill()` – [app/director/src/main.cpp#L16](app/director/src/main.cpp#L16)
- `sigaction()` – [app/doctor/src/main.cpp#L74](app/doctor/src/main.cpp#L74)

### e) Synchronizacja procesów (wątków) – semafory System V
- `ftok()` – [lib/src/SemaphoreArray.cpp#L15](lib/src/SemaphoreArray.cpp#L15)
- `semget()` – [lib/src/SemaphoreArray.cpp#L22](lib/src/SemaphoreArray.cpp#L22)
- `semctl()` – [lib/src/SemaphoreArray.cpp#L37](lib/src/SemaphoreArray.cpp#L37)
- `semop()` – [lib/src/SemaphoreArray.cpp#L50](lib/src/SemaphoreArray.cpp#L50)

### g) Segmenty pamięci dzielonej (System V)
- `ftok()` – [lib/src/SharedMemory.cpp#L14](lib/src/SharedMemory.cpp#L14)
- `shmget()` – [lib/src/SharedMemory.cpp#L21](lib/src/SharedMemory.cpp#L21)
- `shmat()` – [lib/src/SharedMemory.cpp#L29](lib/src/SharedMemory.cpp#L29)
- `shmdt()` – [lib/src/SharedMemory.cpp#L42](lib/src/SharedMemory.cpp#L42)
- `shmctl()` – [lib/src/SharedMemory.cpp#L49](lib/src/SharedMemory.cpp#L49)

### h) Kolejki komunikatów (System V)
- `ftok()` – [lib/src/MessageQueue.cpp#L16](lib/src/MessageQueue.cpp#L16)
- `msgget()` – [lib/src/MessageQueue.cpp#L23](lib/src/MessageQueue.cpp#L23)
- `msgsnd()` – [lib/include/MessageQueue.tpp#L18](lib/include/MessageQueue.tpp#L18)
- `msgrcv()` – [lib/include/MessageQueue.tpp#L42](lib/include/MessageQueue.tpp#L42)
- `msgctl()` – [lib/src/MessageQueue.cpp#L38](lib/src/MessageQueue.cpp#L38)
