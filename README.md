
# Symulator windy – sprawozdanie

## Autor
Dominik Dylak 203518
Adam Zapiórkowski 203746

## Opis projektu
Symulator windy w budynku 5-piętrowym. Winda obsługuje pasażerów z przyciskami, ograniczeniami wagowymi i wizualizacją GDI+.

## Technologie
- C++
- GDI+ (do grafiki)
- Windows API

## Pliki projektu
- `projekt_4.cpp` – główny plik z logiką windy i rysowaniem GUI
- `README.md` – dokumentacja projektu (ten plik)

## Sposób uruchomienia
1. Skonfiguruj projekt w Visual Studio
2. Upewnij się, że linkujesz `Gdiplus.lib`
3. Uruchom `projekt_4.cpp`

## Kluczowe funkcje
- `Elevator::move()` – zarządza ruchem windy
- `DrawElevator()` – rysuje windę i pasażerów
- `CreateFloorButtons()` – przyciski GUI

## Przykładowy scenariusz działania
1. Kliknij "Do 3" na piętrze 0 → pojawia się pasażer
2. Winda przyjeżdża, zabiera go, wysiada na 3
3. Po 5s bezczynności wraca na 0

## Podsumowanie
Projekt demonstruje działanie prostego systemu windy z interaktywną obsługą i ograniczeniami.
