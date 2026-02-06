| Systemy Operacyjne - _PROJEKT_ | Kamil Gargula | GR_01, II rok, st. stacjonarne | Semestr zimowy 2025/26 | [GitHub repo](https://github.com/75gk2/SystemyOperacyjne_projekt_SOR) |
| :----------------------------- | :------------ | :----------------------------- | :--------------------- | :-------------------------------------------------------------------- |

# Raport – Projekt „SOR”

## 1. Założenia projektowe (z tematu)

- Symulacja SOR realizowana wieloprocesowo :
- Komunikacja między procesami oparta o kolejki komunikatów i pamięć dzieloną.
- Synchronizacja liczników kolejek realizowana semaforami System V.
- Rejestracja posiada 2 okienka, z dynamicznym sterowaniem:
  - $K_1 = N/2$ → otwarcie 2. okienka.
  - $K_2 = N/3$ → zamknięcie 2. okienka.
- Priorytety w triażu: czerwony, żółty, zielony.
- VIP w rejestracji obsługiwany bez kolejki.
- Sygnały dyrektora
  - `SIGUSR1` → przerwanie pracy lekarza i czasowa przerwa.
  - `SIGUSR2` → ewakuacja wszystkich procesów.

## 2. Architektura

Architektura to generyczny lib z własnymi testami i implementacją w app.
Pliki app/childProcesses nadpisują klasy bazowe aby **osiągnąć w pełni zabezpieczoną kontrolę nad procesami pochodnymi i strukturami IPC**. W ten sposób destruktor klasy bazowaej Process zintegrowany z ProcessManagerem bezpośrednio steruje projektem.
Aby osiągnąć pełne `RAII` klasy pochodne które tworzą de facto z processManagerem niezależne procesy, są _właścicielami_ tworzonych struktur, i dbają o ich destrukcję, a także o bezpieczne zamknięcie procesu, jeśli reaper tego nie zrobił.

Taka konstrukcja pozwala bezpiecznie importować np. ścieżki do plików wykonywalnych są importowane dynamicznie poprzez globalny interfejs między konfiguracjami cmakelists, tak aby nie było konflików.

### Uruchomienie projektu:

```
./run.sh
```

Plik pozwala na uruchomienie projektu z konsoli. Wtedy się uruchomi defaultSimulation.

W pliku symulacji dodałem podstwową bazę:

#### Procesy potomne (implementacje pochodne `Process`) – argumenty i rola

Poniższe procesy są uruchamiane przez `ProcessManager` przez `fork()`+`execv()`.
Argumenty to dokładnie `extraArgs` przekazywane w konstruktorach klas z [app/childProcesses](app/childProcesses).

| Proces (binarka)   | Klasa          | Argumenty (argv)                                                                                                                                                                                                                                                                   |
| ------------------ | -------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `registrationProc` | `Registration` | `argv[1]=N` (pojemność poczekalni / progi okienek), `argv[2]=delayMs` (własny czas oczekiwania)                                                                                                                                                                                    |
| `triageProc`       | `Triage`       | (brak)                                                                                                                                                                                                                                                                             |
| `doctorProc`       | `Doctor`       | `argv[1]=specialist` (`Triage::Specialist`) - numer typu lekarza specjalisty, `argv[2]=delayMs` (własny czas oczekiwania)                                                                                                                                                          |
| `patientProc`      | `Patient`      | `argv[1]=index` (int, opcjonalny do logów), `argv[2]=isVIP` (T/F) - czy jest vipem, `argv[3]=allowDiesNow` (T/F) - dla 1 sztucznie zwiększa kolor czerwony i blokuj wypisanie już na triażu, `argv[4]=isThisParentWithChildren` (T/F) - spwanuje pacjenta z dzieckiem jeśli prawda |
| `directorProc`     | `Director`     | `argv[1]=cmd` gdzie: `1` → `SIGUSR1` do wszystkich lekarzy; `2` → `SIGUSR2` broadcast do wszystkich procesów; `3..9` → `SIGUSR1` do lekarza który się pojawił jak n-ty, gdzie n to wartość argumentu -2                                                                            |

Na potrzeby symulacji można uruchomić dyrektora po prostu z innej konsoli:

```
./build/bin/directorProc 1
./build/bin/directorProc 2
```

Można też uruchomić interaktywne menu z walidacją:

```
./run.sh 1
```

W przypadku dodania flagi 1 uruchamia się menu, jedak interfejs tego jest "kulawy" gdyż domyślnie włączyłem logi na konsolę. Mimo to zadbałem o wymaganą walidację do plików.
→ Aby komfortowo korzystać z tego sposobu można przełączyć sink spdloggera na journald (dobrze się sprawdził w przypadku eksportu logów z dockerza) lub do pliku.

### 2.1 Struktura projektu:

```
├── CMakeLists.txt
├── Dockerfile
├── README.md
├── app
│   ├── CMakeLists.txt
│   ├── childProcesses
│   │       Implementacje klasy Process, dla każdego typu procesu
│   ├── director
│   ├── doctor
│   ├── main
│   │   └── src
│   │       ├── main.cpp // plik wejściowy
│   │       └── scenarios
│   │               //implementacje scenraiuszy
│   ├── patient
│   ├── registration
│   ├── triage
├── docs
│   ├── README.md
│   ├── gfxs
│   │   ├── image-1.png
│   │   └── image.png
│   ├── temat.md
│   └── testy.md
├── lib
│   ├── CMakeLists.txt
│   ├── include
│   │       // implementacja plików
│   └── src
│           //implementacje struktur z include
├── run.sh // plik uruchamiający
├── test
│     //Testy
```

### Wyróżniające elementy:

- zastosowanie frameworków zewnętrznych takich jak `Catch2` i `spdlogger`
- Pełna obsługa RAII poprzez obiektową implementację wrapperów do klas ipc poprzez interfejsy bazowe klas `GenericIPC`, i `Process`
- Wsparcie wymuszenia zamknięcia procesu poprzez obiektową konwencję `Process`, co się przekłąda na automatyczną destrukcję, po wywołaniu destruktora ProcessManagera
- commity na github prowadziłem w systemie `conventional commits`, a w mieszanych przypadkach, używałem branchów feature/ i merge/rebase do dev + MR na master

## 4 Synchronizacja

### Flow programu

Plik: [app/patient/src/main.cpp](app/patient/src/main.cpp)

0. **Inicjalizacja danych pacjenta**

- Pacjent losuje/ustawia dane (`BasicData`) i ewentualnie tworzy wątek dziecka (jeśli `isThisParentWithChildren`).
- Instalowany jest handler `SIGUSR2`

1. **Rejestracja (VIP vs standard)**

- Standard:
  - inkrementuje licznik kolejki do rejestracji semaforem `SEM_TYPE::REGISTRATION_QUEUE`
  - wysyła się na kolejkę sterującą rejestracją
  - czeka na wiadomość z ID okienka z broadcastu rejestracji (`Q_REGISTRATION_ID='R'`, `mtype=1`).
  - dekrementuje semafor kolejki do rejestracji (bez czekania, to jest asynchroniczny licznik dla rejestracji, który nie spada poniżej 0)
  - wysyła dane do okienka (`QID_WINDOW_1_IN='r'`, `mtype=2`).
- VIP:
  - pomija semafor kolejki "Wpycha się"
  - wysyła dane na stos okienek rejestracji z wyższym priorytetem (`QID_WINDOW_1_IN='r'`, `mtype=1`).
- Odpowiedź z rejestracji:
  - pacjent odbiera odpowiedź z `QID_WINDOW_1_OUT='s'` z `mtype = socialId`.

1. **Wejście do poczekalni (limit miejsc)**

- Poczekalnia jest ograniczona semaforem `SEM_TYPE::WAITING_ROOM_QUEUE` jako licznikiem „wolnych miejsc”.
- Pacjent zajmuje miejsca przez `pullDown(WAITING_ROOM_QUEUE, 1)`.
- Rodzic z dzieckiem zajmuje 2 miejsca (`pullDown(..., 2)`).

3. **Triaż**

- Pacjent wysyła dane do triażu (`QID_TRIAGE_IN='T'`, `mtype=1`).
- Pacjent odbiera wynik z triażu (`QID_TRIAGE_OUT='U'`, `mtype = socialId`). Wynik to: kolor (priorytet) + specjalista + flaga „czy jest odesłany do domu”
- Jeśli pacjent jest odrzucony po triażu zwalnia zajęte miejsca w poczekalni (semafor) i kończy.

4. **Kolejka do lekarza (priorytet + specjalizacja)**

- Pacjent wysyła żądanie do kolejki lekarzy (`QID_DOCTOR_IN='D'`) z rozczytaniem kanału poprzez funkcję `priorityToType(specialist, color)`, payload to:
  - specjalista (bazowy offset),
  - priorytet (RED/YELLOW/GREEN).

1. **Wejście do gabinetu + diagnostyka (2-fazowa komunikacja)**

- Lekarz „woła” pacjenta wiadomością w kolejce gabinetu (`QID_DOCTORS_ROOM='E'`, `mtype = socialId`).
- Pacjent wysyła dane diagnostyczne do tej samej kolejki, ale innym typem: `mtype = QTYPE_DIAGNOSE_OFFSET + socialId`.
  - Jeśli pacjent ma dziecko: sygnalizuje wątek dziecka że ma się poddać badaniu `pthread_cond_signal`, a wątek dziecka przekazuje dane diagnostyczne na kolejkę pokojów lekarzy.
  - Jeśli nie: pacjent wysyła diagnozę sam.

6. **Werdykt lekarza i wyjście z poczekalni**

- Pacjent odbiera wynik leczenia z `QID_DOCTORS_VERDICT='F'` z `mtype = socialId`.
- Zwalnia miejsce(a) w poczekalni (`pullUp(WAITING_ROOM_QUEUE, 1/2)`) i kończy.

#### Okienka rejestracji

Plik: [app/registration/src/main.cpp](app/registration/src/main.cpp)

- Każde okienko działa jako osobny wątek i gdy jest wolne, wysyła swój jakby token dostępności ja kolejkę oczekiwania pacjentów.

- Potem okienko czeka na podejście pacjenta(wspólna kolejka dla obu okienek) `QID_WINDOW_1_IN='r'` z priotytetami:
  - Standard: `mtype=2` (`Registration::QTYPE_WINDOW_IN`).
  - VIP: `mtype=1` (`Registration::QTYPE_WINDOW_IN_VIP`).

- Priorytet VIP vs standard (ważne):
  - Odbiera wartość ujemną z kolejki, czyli jeśli są VIP (`mtype=1`) i standard (`mtype=2`), to okienko zawsze wybierze VIP w pierwszej kolejności.
  - Okienko aktywuje pacjętów czekających na `QID_WINDOW_1_IN='r'`

- W procesie asynchronicznie odbywa się dynamiczne sterowanie liczbą okienek:
  - Kontroler „czeka na zmianę” blokując się na `QID_REGISTRATION_CTRL`, a później podejmuje decyzję na podstwie liczby osób w kolejce: (wspomniany semafor `SEM_TYPE::REGISTRATION_QUEUE` jako _asynchroniczny licznik_)

### Lekarz

Plik: [app/doctor/src/main.cpp](app/doctor/src/main.cpp)

- Pacjent wysyła `Doctor::Q_DOCTOR_IN_STRUCT{ basic, color, specialist }` na `D` z `mtype` wyliczonym jako:
  - `mtype = specialist*10 + priorytetKoloru`, - To tworzy „podkolejki” per-specjalista + per-kolor w jednej kolejce.

- Wezwanie pacjenta do gabinetu (call-in):
  - Lekarz wysyła na kolejkę wyjściową pustą wiadomość `Doctor::Q_DOCTOR_CALLS_IN{}` z `mtype = socialId`.
  - wsskazany pacjent ją odbiera.

- Wybór pacjenta przez lekarza:
  - Lekarz próbuje odebrać pacjenta w kolejności priorytetu:
    - najpierw `RED` (non-blocking),
    - potem `YELLOW` (non-blocking),
    - potem `GREEN` (non-blocking),

- Druga faza wizyty: diagnoza (life data):
  - Pacjent odsyła na tą samą kolejkę, ale już z innym type, boy uniknąć błędów nasłuchiwania:
  - Lekarz odbiera tą diagnozę

- Trzeci etap:
  - Lekarz wysyła werdykt
  - Pacjent go odbiera

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

W ostatnim już kroku sprawdziłem różnice funkcją *compare files*.
_Test wykazał w zależności od próby 10, 13 a także 19 przeniesionych linijek_
Taki rezultat jest niejako oczekiwany ze względu na margines błędu jednoczesnych operacji, a takżę asynchroniczności sinka spdloggera.

Potwierdza to rozproszenie wyników od oczekiwanych, czyli zamiana na wyłąćznie jeden proces jak poniżejL
![alt text](image-1.png)
Dodatkowo oba pliki miały 1000 linijek, co potwierdza hermetyczność sygnału

_Uwaga:_ aby odtworzyć test należy odkomentować linijkę w pliku
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
