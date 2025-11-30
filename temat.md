| Systemy Operacyjne - _PROJEKT_ | Kamil Gargula | GR_01, II rok, st. stacjonarne | Semestr zimowy 2025/26 |
| :----------------------------- | :------------ | :----------------------------- | :--------------------- |

# Temat 8 – SOR.

SOR działa przez całą dobę, zapewniając natychmiastową pomoc osobom w stanach nagłego zagrożenia zdrowia i życia. Działanie SOR-u opiera się na triażu, czyli ocenie stanu pacjentów, który określa priorytet udzielania pomocy (nie decyduje kolejność zgłoszenia). W poczekalni jest N miejsc.

## Zasady działania SOR:

- SOR jest czynny całą dobę;
- W poczekalni SOR w danej chwili może się znajdować co najwyżej $N$ pacjentów (pozostali, jeżeli są czekają przed wejściem);
- Dzieci w wieku poniżej 18 lat na SOR przychodzą pod opieką osoby dorosłej;
- Osoby uprawnione VIP (np. honorowy dawca krwi,) do rejestracji podchodzą bez kolejki;
- Każdy pacjent przed wizytą u lekarza musi się udać do rejestracji;
- W przychodni są 2 okienka rejestracji, zawsze działa min. 1 stanowisko;
- Operacje na okienkach. Jeśli w kolejce do rejestracji jest:
  - więcej niż $K_1=N/2$ pacjentów $→$ otwórz 2. okienko, jeśli zamknięte
  - mniej niż $K_2=N/3$ pacjentów $→$ zamknij 2. okienko, jeśli otwarte

## Przebieg wizyty na SOR:

### A. Rejestracja:

- Pacjent podaje swoje dane i opisuje objawy.

_**Uwaga**: Rejestracja następuje przed wejściem do poczekalni, i osoby w kolejce nie zapełniają poczekalni. Po odbyciu rejestracji trafiają bezpośrednio do poczekalni lub do kolejki przed wejście do poczekalni (np. na krzesełka na korytarzu) i następnie do poczekalnii._

### B. Ocena stanu zdrowia (Triaż):

- Lekarz POZ weryfikuje stan pacjenta i przypisuje mu kolor zgodny z pilnością
  udzielenia pomocy (na tej podstawie określa się, kto otrzyma pomoc w pierwszej
  kolejności):
  - <span style="color: red">czerwony</span> – wskazuje na bezpośrednie zagrożenie zdrowia bądź życia, wymaga natychmiastowej pomocy - ok. **10%** pacjentów;
  - <span style="color: yellow">żółty</span> – oznacza pilny przypadek, a pacjent powinien zostać jak najszybciej przyjęty - ok. **35%** pacjentów;
  - <span style="color: lightgreen">zielony</span> – świadczy o stabilnym przypadku, w którym nie występuje poważny uszczerbek na zdrowiu czy stan zagrożenia życia, dlatego pacjent może zostać przyjęty po osobach z kodem czerwonym oraz żółtym - ok. **50%** pacjentów;
- Ok. **5%** pacjentów jest **odsyłanych do domu** bezpośrednio z triażu;
- Lekarz POZ po przypisaniu koloru, kieruje danego pacjenta do lekarza specjalisty:
  - kardiologa,
  - neurologa,
  - okulisty,
  - laryngologa,
  - chirurga,
  - pediatry;

_**Uwaga I**: Do triażu kolejno wywoływane są osoby po wejściu na poczekalnię. Osoba w triażu ma zajęte "krzesełko" w poczekalni._

_**Uwaga II:** Rozważam mechanizm pierwszeństwa do triażu zgłaszających pilną potrzebę przy rejestracji, albo nawet pomijając tą procedurę jeśli osobę przywiezie karetką. Taka osoba by trafiła następnie:_

- _do poczekalni nawet jak nie ma miejsca - kolor czerwony,_
- _na sam początek kolejki do poczekalni - kolor żółty,_
- _na koniec kolejki do triażu - kolor zielony._

### C. Wstępna diagnostyka i leczenie:

- Lekarz specjalista wykonuje niezbędne badania (wywiad, badanie fizykalne, EKG, pomiar ciśnienia, itp.), aby ustabilizować funkcje życiowe pacjenta.

_**Uwaga**: pacejnt idąc do gabinetu specjalisty opuszcza poczekalnię_

### D. Decyzja o dalszym postępowaniu:

- Po wstępnej diagnozie i stabilizacji stanu pacjent może zostać przez lekarza specjalistę:
  - **Wypisany do domu** – ok. **85%** pacjentów.
  - Skierowany na **dalsze leczenie** do odpowiedniego oddziału szpitalnego – ok. **14.5%** pacjentów.
  - Skierowany do innej, **specjalistycznej placówki** (np. z uwagi na brak specjalisty lub brak miejsca na oddziale szpitalnym) – ok. **0,5%** pacjentów.

## Polecenia Dyrektora

Na polecenie Dyrektora `sygnał 1` dany lekarz specjalista bada bieżącego pacjenta i przerywa pracę na SOR-rze i udaje się na oddział. Wraca na SOR po określonym losowo czasie.
Na polecenie Dyrektora `sygnał 2` wszyscy pacjenci i lekarze natychmiast opuszczają budynek.

## Zadanie

Napisz procedury:

- Dyrektor,
- Rejestracja,
- Lekarz
- Pacjent

symulujące działanie SOR. Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

# Uwagi:

Aby ustandaryzować kierowanie ruchem zakładam że:

- Dziecko z dorosłym zajmuje 2 miejsca. Jest to logicznie powiązane z fizyczną ilością krzeseł w poczekalni.
- Tak jak to wynika z poprzednich uwag, triaż jest wykonywany osobom dopiero na etapie poczekalnii, i osoba wraca do niej, nie zmieniając statusu swojego "krzesełka".
