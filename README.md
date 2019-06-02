## NS3 Residential Scenario

### draw.py
Rysuje pozycje AP i stacji na podstawie pliku csv wygenerowanego podczas symulacji.

Skrypt posiada następujące parametry:

- --nFloors - liczba pięter budynku
- --xnFlats - liczba mieszkań w wymiarze x
- --ynFlats - liczba mieszkań w wymiarze y
- --nSta - liczba stacji przypadająca na jedno mieszkanie
- --side - przyjmuje trzy parametry:
  - [False | True] - czy ma być wyświetlony widok z boku budynku
  - [x | y] - czy widok ma być na oś x czy y
  - int - pion z mieszkaniami który ma być wyświetlony
- --showFloor - które piętro ma być wyświetlone (dla side=False (domyślnie))

##### Przykład:<br>
./draw.py --nFloors 2 --xnFlats 2 --ynFlats 2 --nSta 3 --side True x 1<br>
./draw.py --nFloors 2 --xnFlats 2 --ynFlats 2 --nSta 3 --showFloor 1<br>

Parametry mogą być także konfigurowane za pomocą pliku konfiguracyjnego config.conf. Parametry zawarte w pliku konfiguracyjnym można nadpisywać za pomocą parametrów w linii poleceń.

##### Przykład:<br>
plik config.conf:<br>
[CONFIG]<br>
nFloors=2<br>
xnFlats=2<br>
ynFlats=2<br>
nSta=3<br>
side=False,x,1<br>
showFloor=0<br>

wywołanie:<br>
./draw --nSta 5

### resid_scenario.cc
Saves AP and STA positions to nodes.csv file.
