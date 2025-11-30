| Systemy Operacyjne - _PROJEKT_ | Kamil Gargula | GR_01, II rok, st. stacjonarne | Semestr zimowy 2025/26 |
| :----------------------------- | :------------ | :----------------------------- | :--------------------- |

# Propozycje na testy - werja robocza, TBD

## Test 1: Limit miejsc w poczekalni (N)

Cel weryfikuje ilość mniejsc w poczekalni.

- Czy pacjent trafi do poczekalni gdy jest pusta?
- Czy pacjent będzie czekał w kolejce do poczekali, gdy mu nie starczy miejsca.
- Czy pacejnt trafi do poczekalni, gdy stoi na korytarzu, ale właśnie się zwolniło miejsce w środku.

## Test 2: Zarządzanie okienkami rejestracji (K1=N/2, K2=N/3)

Manipilacja poziomami, aby zarządzać okienkami:

- Faza 1: kolejka > K1 → oczekiwane otwarcie 2. okienka.
- Faza 2: kolejka < K2 → oczekiwane zamknięcie 2. okienka.

Sprawdza czy okienka działają jak nalezy

## Test 3: Priorytety triażu i rejestracji

a) Zmierzyć czy z poczekalni zostają wywoływani pacjenci po triażu zgodnie z priorytetm.
Oczekiwane rezultaty: Pacjenci czerwoni zawsze obsługiwani pierwsi, potem żółci, następnie zieloni.
Suma wywołań wynosi 100%
b) Zmierzyć czy VIP rejestruje się priorytetowo

## Test 4: Obsługa sygnałów Dyrektora (sygnał 1 i 2)

a) S=1 przetestować sytuacje gdy lekarz jest zajęty i wraca na oddział,
b) S=2 wszyscy mają opusicć szpital
