# SolEx Motorisation (English version below)

Le Sol'Ex de Christian Buil est un "Explorateur solaire" qui permet d'imager le soleil dans une longueur d'onde souhaitée, allant du proche UV au proche IR.
La construction de ce spectroscope est détaillée sur son site internet: http://www.astrosurf.com/solex/sol-ex-objet.html
La partie traitement informatique du signal enregistré est confiée soit à i-Spec (programme écrit par Christian Buil), soit par Inti, programme développé par Valérie Desnoux. Ces deux programmes sont référencés sur le site Sol'Ex, et des mises à jour régulières sont publiées.
Un forum d'échange est aussi disponible à l'adresse : https://groups.io/g/Solex-project La communauté y est très active.

Nous vous présentons ici un appendice développé conjointement par Jean Brunet et Stéphane Ferier, qui permet de motoriser le Sol'Ex initial.
Le but ici est d'apporter une facilité à voyager parmi les raies spectrales qui composent le spectre solaire, au moyen d'un software développé pour Android/IOS, basé sur une page web gérée par le software. Des raies spectrales d'intérêt majeur y sont définies, et accessibles d'un simple clic. Le software permet outre les raies pré-programmées de sélectionner une longueur d'onde souhaitée et de demander au moteur d'orienter le réseau sur cette longueur d'onde. A l'inverse, il est aussi possible de visualiser une longueur d'onde (par retour de caméra d'acquisition), et de l'identifier sur l'affichage de la page web.

Le programme ainsi que les fichiers .stl de construction du support de motorisation sont "open-source", et toute amélioration est la bienvenue.
A cet effet, le fichier .rar contient tout le matériel nécessaire, y compris un fichier excel qui permet de trouver les paramètres de droite standard d'étalonnage.
En effet, le matériel fourni est optimisé pour un réseau de diffraction de 2400tr/mm; optiques 80/125mm constituant le Sol'Ex, ainsi que pour des roues dentées de motorisation d'un rapport de démultiplication 16 et 60 (GT-2) soit 3.75x. Si l'on souhaite utiliser un autre rapport, la droite d'étalonnage doit être adaptée en conséquence.

Outre le fichier .rar, vous trouverez aussi un fichier .pdf qui décrit en détails comment réaliser cette motorisation, comment monter le système sur le Sol'Ex, comment réaliser le montage électronique de pilotage, comment uploader le software dans l'ESP32 utilisé, ainsi que coment utiliser le software. Ce fichier .pdf est disponible en Français, et en Anglais.

# SolEx Motorization (English Version)

Christian Buil's Sol'Ex  is a "Solar Explorer" that allows you to image the sun in a desired wavelength, ranging from near UV to near IR.
The construction of this spectroscope is detailed on his website : http://www.astrosurf.com/solex/sol-ex-objet.html
The computer processing part of the recorded signal is entrusted either to i-Spec (program by  Christian Buil), or by Inti, program developed by Valérie Desnoux. These two programs are referenced on the Sol'Ex website, and regular updates are published.
An exchange forum is also available at: https://groups.io/g/Solex-project The community is very active.

We present here an appendix developed jointly by Jean Brunet and Stéphane Ferier, which allows to motorize the initial Sol'Ex .
The goal here is to provide an ease of travel among the spectral lines that make up the solar spectrum, by means of a software developed for Android/IOS, based on a web page managed by the software. Spectral lines of major interest are defined, and accessible within a simple click. In addition to the pre-programmed lines, the software allows you to  select a desired wavelength and ask the engine to orient the grating on this wavelength. Conversely, it is also possible to visualize a wavelength (by return of acquisition camera), and to identify it on the display of the web page.

The program as well as the .STL to build the motorization are "open-source", and any improvements are welcome.
For this purpose, the . RAR contains all the necessary material, including an Excel file  that allows you to find the standard calibration right settings.
Indeed, the equipment supplied is optimized for a diffraction grating of 2400lines/mm;  80/125mm optics constituting the Sol'Ex, as well as for gear wheels of motorization of a ratio of 16 and 60 (GT-2) or 3.75x. If another gear is to be used, the calibration line must be adjusted accordingly.

In addition to the . rar, you will also find a .pdf which describes in detail  how to make this engine, how to mount the system on the Sol'Ex, how to make the electronic control assembly, how to upload the software in the ESP32 used, as well as how to  use the software. This . pdf is available in French and English.
