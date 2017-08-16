# INSTALLATION DE TRAPPETTE  

<small>
__NOTE 1 :__ Les exemples sont donnés pour le Raspberry Pi avec clé SDR et la sonde M10.  
__NOTE 2 :__ Ne pas connecter votre clé SDR avant d'avoir installé rtl-sdr  ! 
</small>

## 1.	INSTALLATION DES OUTILS NÉCESSAIRES  

Avant toute chose, pour être sûr de partir sur de bonnes bases, mettez à jour votre système :  

	sudo apt-get update
	sudo apt-get upgrade  
	
Dans votre console, exécutez la commande suivante :  

	sudo apt-get install cmake git libusb-1.0-0-dev

Elle permet d'installer les outils nécessaires :   

* git ( pour récupérer les sources de rtlsdr )
* cmake ( pour compiler rtlsdr )
* libusb-1.0-0-dev ( lien entre les programmes de rtlsdr et le hardware )  


## 2.	RÉCUPÉRER RTL-SDR  

(source : https://osmocom.org/projects/sdr/wiki/rtl-sdr )

	git clone git://git.osmocom.org/rtl-sdr.git
	cd rtl-sdr
	mkdir build
	cd build/
	cmake ../ -DINSTALL_UDEV_RULES=ON
	make
	sudo make install
	sudo ldconfig
    
## 3.	EMPÊCHER LE KERNEL DE JOUER AVEC LA CLÉ RTL2832U À LA PLACE DE RTLSDR  

( source: https://opendesignengine.net/news/53 )  

	sudo bash                        (le mot de passe par defaut est "raspberry")  
	echo "blacklist dvb_usb_rtl28xxu" >> /etc/modprobe.d/blacklist.conf  
	exit

Eteignez votre Raspberry Pi  
Connectez votre clé RTL2832U dans un des port USB  
Rebranchez votre Raspberry Pi et relancer un terminal.  

## 4.	VÉRIFIER QUE LA CLÉ RTL2832U FONCTIONNE  
	rtl_test

ça fonctionne? Bravo! On a fait le plus dur…  
Ctrl+C pour arrêter rtl_test  

ça ne fonctionne pas?  
lisez calmement les sources cités ci-dessus pour chercher le soucis.

## 5.	MESURER LA DÉVIATION PPM DE VOTRE CLÉ RTL2832U  

Lancez **rtl_test** avec l’option -p, et laissez-le tourner **30 min à 1h** :  

	rtl_test -p
	[...]
	real sample rate: 2048140 current PPM: 69 cumulative PPM: 79
	real sample rate: 2048147 current PPM: 72 cumulative PPM: 79

	
Ctrl+C pour arrêter rtl_test  
La déviation est par exemple ici pour moi de 79 ppm ( à mémoriser )  

## 6.	VÉRIFIER QU’ON REÇOIT BIEN QUELQUE CHOSE AVEC LA CLÉ  
	rtl_fm -p 79 -f 402M -M fm -s 48k -E dc - | hexdump -Cv
	 ou 
	rtl_fm -p 79 -f 402M -M fm -s 48k -E dc - | aplay -r 48k -f S16_LE

Ctrl+C pour arrêter rtl_fm 

## 7.	RÉCUPÉRER ET COMPILER LE PROGRAMME “TRAPPETTE”  

Si c'est la première fois que vous récupérez les sources :  

	git clone https://github.com/Quadricopter/trappette.git
	cd trappette
	make

Pour mettre à jour une installation existante :  

	cd trappette
	git pull
	make clean
	make

## 8.	FICHIER DE CONFIGURATION  

Pour créer le fichier de configuration nous allons commencer par copier l'exemple fourni : 

	cp trappette.cfg.sample trappette.cfg

Avec votre éditeur favori, examinez alors les paramètres et modifiez ceux qui doivent l'être (notamment vos coordonnées et les diverses options).  


	# Your QRA coordinates, Eiffel Tower follow:
	qra.latitude = 48.858256
	qra.longitude = 2.294544
	qra.altitude  = 334
	
	# Output
	time.offset = 3600
	earth.ellipsoid = 49
	#header.repeat = 22
	
	# GPS output
	gps.out.port = /dev/ttyUSB0
	gps.out.baud = 4800

	# Rotor
	rotor.port = /dev/ttyACM0
	rotor.baud = 9600
	rotor.azimuth.init = 246 # Eiffel Tower -> Trappes
	rotor.elevation.init = 0

	rotor.azimuth.min = 0
	rotor.azimuth.max = 360
	rotor.elevation.min = 0
	rotor.elevation.max = 90
 


## 9. UTILISATION

Trappette lit le flux binaire sur stdin au format **48kHz/S16LE**.  
Ce flux de données peut être créé par arecord, gqrx (socket UDP), rtl_fm, sox, etc ...  
La commande va donc varier en fonction de la source des données.
Vous devez donc exécuter le premier logiciel et renvoyer sa sortie vers trappette.

### Carte son
Pour utiliser la carte son, un signal BF venant d'un récepteur appliqué à l'entrée ligne de l'ordinateur par exemple, vous pouvez utiliser arecord :

	arecord -f S16_LE -c 1 -r 48k -t raw | ./trappette

### Socket UDP gqrx
Vous pouvez activer l'UDP gqrx (port par défaut : 7355 ), et transmettre le flux avec **nc**  
(http://gqrx.dk/doc/streaming-audio-over-udp)  
```
nc -l -u 7355 | ./trappette
```  

### Rtl_fm  

Avec rtl_fm vous spécifiez la correction en ppm (-p), la fréquence (-f) en Hertz, la modulation (-f) ici FM, la vitesse d'échantillonage (-s), et (-E).
Le résultat est transmis, "pipé", vers trappette.  

Exemple pour **402MHz** et une correction de **75ppm** :  

```
rtl_fm -p 75 -f 402M -M fm -s 48k -E dc - | ./trappette
```  

Pour déterminer la correction nécessaire à votre clé sdr, lancez **rtl_test** avec le paramètre -p, et laissez-le tourner au moins 30 min, jusqu'à ce que la valeur se stabilise.   

```
rtl_test -p
```  

Notez le résultat "cumulative PPM: **XX**".  Et utilisez le pour le paramètre -p.
  
  
### Fichier .wav  

```
sox votre_fichier.wav -b 16 -e signed-integer -c 1 -r 48k -t raw - | ./trappette
```
  
## 10. OPTIONS

```
Usage: ./trappette [-k kmlfile] [-n nemafile] [-gpsout] [-hex] [-v]  
                   [-a seconds] [-t seconds] [-q lat:lon:alt]  
		   
 -k: indique le nom d'un fichier de sortie au format .kml 
 -n: indique le nom d'un fichier de sortie au format gps .nema
 -gpsout: autorise la sortie au format NEMA sur le port série
 -hex: dump hexadecimal
 -v: "verbose" - affiche davantage de commentaires
 -a: "abandon"  - temps en secondes au bout duquel le programme s'arrête s'il n'a rien reçu du tout
 -t: "time out" - temps en secondes au bout duquel le programme s'arrête après la dernière position reçue
 -q: pour indiquer la position de la station en ligne de commande. Surpasse les valeurs mentionnées dans le fichier de configuration.
 	entrer -q latitude:longitude:altitude. les coordonnées en degrés décimaux. l'altitude en mètres est facultative.
```
