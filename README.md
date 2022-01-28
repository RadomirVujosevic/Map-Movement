
# Uputstvo za pokretanje iz terminala
1. git clone https://github.com/RadomirVujosevic/Map-Movement.git
2. cd Map-Movement/
3. cmake .
4. make
5. ./project_base

# Opis

Projekat predstavlja kretanje figure na karti. Pritiskom na strelicu gore ce se figura pomeriti.

Model igraca ima direkciono svetlo i svetlo iz tacke koja se krece u krug oko njega.
modelShader koristi spekularno svetlo po Blinn-Phong modelu

Mapa ima isto direkciono svetlo i spotlight koji predstavlja vidokrug igraca. 

Za strelice je koristen discard blending. 
One predstavljaju kuda igrac moze da ide. 
Nije implementirana stvarna logika za pomeranje strelica, vec stoji na fiksnoj mestu u odnosu na igraca


Implementiran Cubemap iz grupe A

# Credits

Viking model prezuet sa sajta free3d.com
https://free3d.com/3d-model/monitor-viking-v1--698176.html

Tekstura napravljena u Blender-u, a spekularna mapa u GIMP-u

Marker modele sam pravio u Blender-u